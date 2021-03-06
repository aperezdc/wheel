/*
 * wtask.c
 * Copyright (C) 2014 aperez <aperez@hikari>
 *
 * Distributed under terms of the MIT license.
 */

/*
 * Coroutine ("Task") support inspired by Plan9's libtask.
 *
 * IMPLEMENTATION NOTES
 *
 * - Macros from queue.h are used directly instead of w_list_t or other
 *   container provided by libwheel in order to avoid having dependencies
 *   on other parts of libwheel.
 *
 * - libtask includes assembler routines for popular platforms, whereas this
 *   implementation uses {get,set}context(), which should be available in any
 *   reasonable Unix-like system in which libwheel can be used.
 */

/**
 * Tasks
 * =====
 *
 * The tasks system provides “green threads” (also known as *lightweight
 * cooperative threads*, or
 * `coroutines <https://en.wikipedia.org/wiki/Coroutine>`__).
 *
 * Additionally, the following utilities are provided to be used along with
 * the tasks system:
 *
 * - Functions to suspend a coroutine and wait for I/O to be completed:
 *   :func:`w_task_yield_io_read()`, :func:`w_task_yield_io_write()`.
 *
 * - The :func:`w_io_task_open()` and :func:`w_io_task_init()` functions
 *   can be used to create a :type:`w_io_task_t` wrapper to ease using
 *   asynchronous I/O with tasks.
 *
 * - Socket listeners, which greatly simplify writing network servers using
 *   a task to handle each client connection: :func:`w_task_listener_new()`
 *   can be used to create  a :type:`w_task_listener_t`, which then can be
 *   run in a task to start accepting connections using
 *   :func:`w_task_prepare()`.
 */

/**
 * Types
 * -----
 */

/*~t w_task_t
 *
 * Type of a task.
 *
 * Tasks are created by :func:`w_task_prepare()`, and the resources used by a
 * task (including the stack space used by the task and the :type:`w_task_t`
 * value itself) will be automatically freed when the tasks is exited. Never
 * deallocate a task manually, or use it after it has been exited.
 */

/*~t w_task_func_t
 *
 * Type of task functions.
 */

/*~t w_task_listener_t
 *
 * Type of a task-based socket listener.
 */

/*~t w_task_listener_func_t
 *
 * Type of handler functions for connections served using a task-based socket
 * listener.
 */

/*~t w_io_task_t
 *
 * Stream wrapper for asynchronous input/output.
 *
 * This is a subclass of :type:w_io_t; all the
 * :ref:`stream functions <wio-functions>` can be used on objects of this
 * type. Reading and writing data uses :func:`w_task_yield_io_read()` and
 * :func:`w_task_yield_io_write()`, so instead of blocking until completion
 * the current task will be suspended automatically.
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"
#include "queue.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <stdint.h>
#include <ucontext.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#ifndef MAP_ANONYMOUS
# ifdef MAP_ANON
#  define MAP_ANONYMOUS MAP_ANON
# else
#  error Neither MAP_ANON or MAP_ANONYMOUS is defined
# endif /* MAP_ANON */
#endif /* !MAP_ANONYMOUS */

#ifndef MAP_STACK
# define 0
# warning MAP_STACK is undefined, but this should not be a problem
#endif /* !MAP_STACK */

#ifndef MAP_UNINITIALIZED
# define MAP_UNINITIALIZED 0
#endif /* !MAP_UNINITIALIZED */


TAILQ_HEAD (task_list, w_task);

static ucontext_t s_scheduler_uctx;
static struct task_list s_runqueue;
static w_task_t *s_current_task     = NULL;
static unsigned  s_num_system_tasks = 0;
static unsigned  s_num_tasks        = 0;


#define CHECK_SCHEDULER( )                                          \
    do {                                                            \
        if (!s_current_task)                                        \
            W_FATAL ("Called without a running task scheduler.\n"); \
    } while (0)


enum task_state
{
    TASK_READY,
    TASK_RUN,
    TASK_YIELD,
    TASK_WAITIO,
    TASK_EXIT,
};


struct w_task
{
    char           *name;
    enum task_state state;
    bool            is_system;
    w_task_func_t   task_func;
    void           *task_data;
    ucontext_t      context;

    TAILQ_ENTRY (w_task) tailq;
};


static void
task_start_trampoline (uint32_t hi, uint32_t lo)
{
    /* Reconstruct the task pointer (explanation below) */
    w_task_t *t = (w_task_t*)
        (((uintptr_t) hi & 0xFFFFFFFF) << 32 | (lo & 0xFFFFFFFF));
    (t->task_func) (t->task_data);
    w_task_exit ();
}


static size_t
round_to_pagesize (size_t stack_size)
{
    static size_t page_size = 0;
    if (page_size == 0)
        page_size = sysconf (_SC_PAGESIZE);

    size_t remainder = stack_size % page_size;
    if (remainder)
        stack_size += page_size - remainder;
    w_assert (stack_size % page_size == 0);
    return stack_size;
}


static w_task_t*
allocate_task_and_stack (size_t stack_size)
{
    size_t alloc_size = round_to_pagesize (stack_size + sizeof (w_task_t));
    void *addr = mmap (NULL,
                       alloc_size,
                       PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_STACK | MAP_PRIVATE | MAP_UNINITIALIZED,
                       -1,
                       0);
    if (addr == MAP_FAILED)
        W_FATAL ("mmap() failed: $E\n");

    memset (addr, 0x00, sizeof (w_task_t));
    w_task_t* t = (w_task_t*) addr;
    t->state    = TASK_READY;

    /* Zero-init the signal mask in the task context. */
    sigset_t zero;
    sigemptyset (&zero);
    sigprocmask (SIG_BLOCK, &zero, &t->context.uc_sigmask);

    /* Initialize with the current context. */
    if (getcontext (&t->context) < 0)
        W_FATAL ("getcontext() failed: $E\n");

    t->context.uc_stack.ss_size = alloc_size - sizeof (w_task_t);
    t->context.uc_stack.ss_sp   = (void*) (t + 1); /* Point _after_ the task struct */

    /*
     * Most Unix systems only pass 32-bit integer values correctly through
     * makecontext(). We pass the high/low parts of a (potential) 64-bit
     * pointer separately, which gets reconstructed at the start trampoline.
     */
    makecontext (&t->context, (void (*)()) task_start_trampoline, 2,
                 (uint32_t) ((((uintptr_t) t) >> 32) & 0xFFFFFFFF),
                 (uint32_t) (((uintptr_t) t) & 0xFFFFFFFF));
    return t;
}


static void
free_task_and_stack (w_task_t *t)
{
    w_assert (t);
    w_free (t->name);
    if (munmap ((void*) t, t->context.uc_stack.ss_size + sizeof (w_task_t)) < 0)
        W_WARN ("munmap() failed: $E (trying to continue)\n");
}


static inline void
switch_context (ucontext_t *from, ucontext_t *to)
{
    w_assert (from);
    w_assert (to);

    if (swapcontext (from, to) < 0)
        W_FATAL ("swapcontext() failed: $E\n");
}


/*~f w_task_t* w_task_prepare (w_task_func_t function, void *data, size_t stack_size)
 *
 * Creates a task with a given `stack_size` and prepares it for running a
 * `function`, passing a `data` pointer to the function.  The task will be
 * in paused state upon creation.
 *
 * To get tasks running, the scheduler must be running, see
 * :func:`w_task_run_scheduler()`.
 *
 * The `stack_size` is  always rounded up to the size of a memory page. It
 * is possible to pass zero to get the smallest possible stack size (usually
 * 4 kB).
 */
w_task_t*
w_task_prepare (w_task_func_t func, void *data, size_t stack_size)
{
    w_assert (func);

    if (s_num_tasks == 0)
        TAILQ_INIT (&s_runqueue);

    w_task_t *t  = allocate_task_and_stack (stack_size);
    t->task_func = func;
    t->task_data = data;
    s_num_tasks++;

    /* A task is ready to be scheduled after creation. */
    TAILQ_INSERT_TAIL (&s_runqueue, t, tailq);
    return t;
}


/*~f w_task_t* w_task_current ()
 *
 * Obtains the task currently running.
 *
 * .. warning:: This function **must** be called from inside a task, once the
 *    task scheduler has been started. Otherwise, calling this function is an
 *    error and the execution of the program will be aborted.
 */
w_task_t*
w_task_current (void)
{
    CHECK_SCHEDULER ();
    return s_current_task;
}


/*~f void w_task_set_is_system (w_task_t *task, bool is_system)
 *
 * Set whether a `task` is a system task. System tasks are those which
 * are always running.
 *
 * System tasks do not prevent :func:`w_task_run_scheduler()` from
 * returning.
 */
void
w_task_set_is_system (w_task_t *task, bool is_system)
{
    w_assert (task);

    if (task->is_system && !is_system) {
        task->is_system = false;
        s_num_system_tasks--;
        s_num_tasks++;
    } else if (!task->is_system && is_system) {
        task->is_system = true;
        s_num_system_tasks++;
        s_num_tasks--;
    }
}


/*~f bool w_task_get_is_system (w_task_t *task)
 *
 * Checks whether a task is a system task.
 *
 * See also :func:`w_task_set_is_system()`.
 */
bool
w_task_get_is_system (w_task_t *task)
{
    w_assert (task);
    return task->is_system;
}


/*~f void w_task_set_name (w_task_t *task, const char *name)
 *
 * Sets the `name` of a `task`. The name of the tast is copied as-is, and
 * it is not interpreted in any way. The ability of naming tasks is mainly
 * provided as an aid for debugging client code.
 *
 * It is possible to pass ``NULL`` as the `name`, which will clear any
 * custom name previously set.
 */
void
w_task_set_name (w_task_t *task, const char *name)
{
    w_assert (task);
    w_free (task->name);
    task->name = name ? w_str_dup (name) : NULL;
}


/*~f const char* w_task_get_name (w_task_t *task)
 *
 * Obtains the name of a `task`.
 *
 * If a name has not been set using :func:`w_task_set_name()`, an
 * autogenerated one of the form ``Task<ID>`` will be returned.
 */
const char*
w_task_get_name (w_task_t *task)
{
    w_assert (task);
    if (!task->name) {
        w_buf_t b = W_BUF;
        (void) w_buf_format (&b, "Task<$p>", task);
        task->name = w_buf_str (&b);
    }
    return task->name;
}


/*~f void w_task_run_scheduler ()
 *
 * Runs the task scheduler.
 *
 * The scheduler will choose tasks in a round-robin fashion, and let each
 * task run until it gives up the CPU explicitly using :func:`w_task_yield()`
 * or implicitly when waiting for input/output on a stream be means of
 * :func:`w_task_yield_io_read()` and :func:`w_task_yield_io_write()`.
 *
 * The scheduler will keep scheduling tasks until all non-system tasks
 * have been exited.
 *
 * This function **must** be called in the main function of a program.
 * Typically:
 *
 * .. code-block:: c
 *
 *      extern void process_argument (void*);
 *
 *      int main (int argc, char **argv) {
 *          while (argc--)
 *              w_task_prepare (process_argument, *argv++, 0);
 *          w_task_run_scheduler ();
 *          return 0;
 *      }
 */
void
w_task_run_scheduler (void)
{
    if (s_num_tasks == 0)
        W_FATAL ("No tasks. Missing w_task_prepare() calls?\n");

    while (s_num_tasks > 0) {
        w_assert (!TAILQ_EMPTY (&s_runqueue));

        s_current_task = TAILQ_FIRST (&s_runqueue);
        TAILQ_REMOVE (&s_runqueue, s_current_task, tailq);
        s_current_task->state = TASK_RUN;

        /* Switch to the newly-scheduled task. */
        switch_context (&s_scheduler_uctx, &s_current_task->context);

        /*
         * The task may have called w_task_exit() or w_task_yield(). In
         * the former case, we need to free its associated resources.
         */
        if (s_current_task->state == TASK_EXIT) {
            if (!s_current_task->is_system)
                s_num_tasks--;
            free_task_and_stack (s_current_task);
        }
    }
}


static inline void
yield_to_scheduler (enum task_state next_state)
{
    /* Put the current task back in the run queue. */
    s_current_task->state = next_state;

    if (next_state != TASK_EXIT)
        TAILQ_INSERT_TAIL (&s_runqueue, s_current_task, tailq);

    /* Switch to the scheduler. */
    switch_context (&s_current_task->context, &s_scheduler_uctx);
}


/*~f void w_task_yield ()
 *
 * Make the current task give up the CPU, giving control back to the
 * task scheduler, which will give other other tasks the chance to run.
 */
void
w_task_yield (void)
{
    CHECK_SCHEDULER ();
    yield_to_scheduler (TASK_YIELD);
}


/*~f void w_task_exit ()
 *
 * Exits the current task. This can be used to exit from a task at any
 * point, without needing to return from the task function.
 */
void
w_task_exit (void)
{
    CHECK_SCHEDULER ();
    yield_to_scheduler (TASK_EXIT);
}


/*~f w_io_result_t w_task_yield_io_read (w_io_t *stream, void *buffer, size_t count)
 *
 * Reads `count` bytes into the memory block at `buffer` from an input
 * `stream`, suspending the current task as needed.
 *
 * If the `stream` has been set as non-blocking and reading from it results
 * in an ``EAGAIN`` or ``EWOULDBLOCK`` error, the current task will give up
 * the CPU and wait until the data is available for reading as many times as
 * needed, until `count` bytes are read, the end-of-file marker is reached,
 * or an error is found.
 */
w_io_result_t
w_task_yield_io_read (w_io_t *io, void *buf, size_t len)
{
    CHECK_SCHEDULER ();
    w_assert (io);

    w_io_result_t ret = W_IO_RESULT (len);
    while (len) {
        w_assert (buf);
        w_io_result_t r = w_io_read (io, buf, len);

        if (w_io_failed (r)) {
            int err = w_io_result_error (r);
            if (err == EAGAIN || err == EWOULDBLOCK) {
                yield_to_scheduler (TASK_WAITIO);
            } else {
                return r;
            }
        }

        if (w_io_eof (r)) {
            return r;
        }

        len -= w_io_result_bytes (r);
        buf += w_io_result_bytes (r);
    }
    return ret;
}


/*~f w_io_result_t w_task_yield_io_write (w_io_t *stream, const void *buffer, size_t count)
 *
 * Writes `count` bytes from the memory block at `buffer` to an output
 * `stream`, suspending the current task as needed.
 *
 * If the `stream` has been set as non-blocking and writing to it results in
 * an ``EAGAIN`` or ``EWOULDBLOCK`` error, the current task will give up the
 * CPU and wait until the stream accepts writing data as many times as needed,
 * until `count` bytes are written, or an error is found.
 */
w_io_result_t
w_task_yield_io_write (w_io_t *io, const void *buf, size_t len)
{
    CHECK_SCHEDULER ();
    w_assert (io);

    w_io_result_t ret = W_IO_RESULT (len);
    while (len) {
        w_assert (buf);
        w_io_result_t r = w_io_write (io, buf, len);

        if (w_io_failed (r)) {
            int err = w_io_result_error (r);
            if (err == EAGAIN || err == EWOULDBLOCK) {
                yield_to_scheduler (TASK_WAITIO);
            } else {
                return r;
            }
        }

        if (w_io_eof (r)) {
            return r;
        }

        len -= w_io_result_bytes (r);
        buf += w_io_result_bytes (r);
    }
    return ret;
}


static int
w_io_task_getfd (w_io_t *iobase)
{
    w_io_task_t *io = (w_io_task_t*) iobase;
    return io->wrapped ? w_io_get_fd (io->wrapped) : -1;
}


static w_io_result_t
w_io_task_flush (w_io_t *iobase)
{
    w_io_task_t *io = (w_io_task_t*) iobase;
    return io->wrapped
        ? w_io_flush (io->wrapped)
        : W_IO_RESULT_ERROR (errno = EBADF);
}


static w_io_result_t
w_io_task_close (w_io_t *iobase)
{
    w_io_task_t *io = (w_io_task_t*) iobase;

    if (!io->wrapped)
        return W_IO_RESULT_ERROR (errno = EBADF);

    w_obj_unref (io->wrapped);
    io->wrapped = NULL;
    return W_IO_RESULT_SUCCESS;
}


static w_io_result_t
w_io_task_read (w_io_t *iobase, void *buf, size_t len)
{
    w_io_task_t *io = (w_io_task_t*) iobase;
    return io->wrapped
        ? w_task_yield_io_read (io->wrapped, buf, len)
        : W_IO_RESULT_ERROR (errno = EBADF);
}


static w_io_result_t
w_io_task_write (w_io_t *iobase, const void *buf, size_t len)
{
    w_io_task_t *io = (w_io_task_t*) iobase;
    return io->wrapped
        ? w_task_yield_io_write (io->wrapped, buf, len)
        : W_IO_RESULT_ERROR (errno = EBADF);
}


/*~f bool w_io_task_init (w_io_task_t *wrapper, w_io_t *stream)
 *
 * Initializes a stream `wrapper` object (possibly allocated in the stack)
 * which wraps a `stream`. The `wrapper` behaves like the wrapped `stream`,
 * suspending the current task when needed to ensure that I/O is performed
 * asynchronously.
 *
 * The return value indicates whether the `stream` can be wrapped. Most of
 * the streams for which :func:`w_io_get_fd()` returns a valid file descriptor
 * can be wrapped.
 */
bool
w_io_task_init (w_io_task_t *io, w_io_t *wrapped)
{
    w_assert (io);
    w_assert (wrapped);

    int fd = w_io_get_fd (wrapped);
    if (fd < 0)
        return false;

    int flags = fcntl (fd, F_GETFL);
    if (flags < 0 || fcntl (fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return false;

    w_io_init (&io->parent);

    io->parent.write = w_io_task_write;
    io->parent.read  = w_io_task_read;
    io->parent.flush = w_io_task_flush;
    io->parent.getfd = w_io_task_getfd;
    io->parent.close = w_io_task_close;
    io->wrapped = w_obj_ref (wrapped);
    return true;
}


/*~f w_io_t* w_io_task_open (w_io_t *stream)
 *
 * Wraps a `stream` and returns an object that behaves like the wrapped
 * `stream`, suspending the current task when needed to ensure that I/O
 * is performed asynchronously.
 *
 * Returns ``NULL`` when the `stream` cannot be wrapped. Most of the streams
 * for which :func:`w_io_get_fd()` returns a valid file descriptor can be
 * wrapped.
 */
w_io_t*
w_io_task_open (w_io_t *wrapped)
{
    w_io_task_t *io = w_obj_new (w_io_task_t);
    if (w_io_task_init (io, wrapped))
        return (w_io_t*) io;
    w_obj_destroy (io);
    return NULL;
}

/*~f void w_task_system ()
 *
 * Mark the current task as a system task.
 *
 * See also :func:`w_task_set_system()`.
 */

/*~f const char* w_task_name ()
 *
 * Obtain the name of the current task.
 *
 * See also :func:`w_task_get_name()`.
 */


struct listener_conn_info
{
    w_task_listener_t *listener;
    w_io_t            *socket;
};


static void
listener_conn_trampoline (void *obj)
{
    struct listener_conn_info *conn_info = obj;
    w_assert (conn_info);

    /*
     * Save values to the stack, to avoid leaking the heap-allocated
     * struct when w_task_exit() gets called. The connection handler
     * callback function is supplied by the user, who does not know
     * about what the trampoline is doing under the hood.
     */
    w_task_listener_t *listener = conn_info->listener;
    w_io_t *socket = conn_info->socket;
    w_free (conn_info);

    (*listener->handle_connection) (listener, socket);
}


/*~f void w_task_listener_run (void *listener)
 *
 * Implements the logic for listener tasks. This function is a
 * :type:`w_task_func_t`, and **must** be used along with
 * func:`w_task_prepare()`, in the following way:
 *
 * .. code-block:: c
 *
 *    w_task_listener_t *listener = w_task_listener_new ( … );
 *    w_task_prepare (w_task_listener_run, listener, 0);
 *
 * This The task scheduler will take care of running the listener.
 */
void
w_task_listener_run (void *arg)
{
    CHECK_SCHEDULER ();
    w_assert (arg);

    w_task_listener_t *listener = w_obj_ref (arg);
    listener->running = true;

    while (listener->running) {
        struct sockaddr sa;
        socklen_t slen = sizeof (sa);

        int new_fd = accept (listener->fd, (void*) &sa, &slen);
        if (new_fd >= 0) {
            int n = 1;
            setsockopt (new_fd, IPPROTO_TCP, TCP_NODELAY, (char*) &n, sizeof (int));
            w_io_t *client_io = w_io_unix_open_fd (new_fd);

            struct listener_conn_info *conn_info = w_new (struct listener_conn_info);
            conn_info->socket   = w_io_task_open (client_io);
            conn_info->listener = listener;
            w_obj_unref (client_io);

            w_task_t *task = w_task_prepare (listener_conn_trampoline,
                                             conn_info,
                                             16384);

            unsigned task_name_len = strlen (w_task_name ());
#if defined(W_CONF_IPv6) && INET6_ADDRSTRLEN > INET_ADDRSTRLEN
            char name[task_name_len + 2 + INET6_ADDRSTRLEN];
#else
            char name[task_name_len + 2 + INET_ADDRSTRLEN];
#endif /* W_CONF_IPv6 && INET6_ADDRSTRLEN > INET_ADDRSTRLEN */
            strncpy (name, w_task_name (), task_name_len);
            name[task_name_len] = '(';
            if (inet_ntop (sa.sa_family, (void*) &sa,
                           name + task_name_len + 1,
                           sizeof (name) - task_name_len - 1)) {
                strcat (name, ")"); /* XXX: Inefficient. */
            } else {
                strncat (name + task_name_len + 1, "?)", 2);
            }
            w_task_set_name (task, name);
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            yield_to_scheduler (TASK_WAITIO);
            /*
             * XXX: Currently, if all tasks are in waiting state
             *      (including TASK_WAITIO), the scheduler will keep
             *      scheduling tasks instead of sleeping, and the CPU
             *      will be all the time busy. Sleeping 500µs here is
             *      good enough, but it is still a kludge.
             */
            usleep (500);
        } else {
            w_printerr ("$s: Error accepting connection: $E\n", w_task_name ());
        }
    }

    w_obj_unref (listener);
}


/*~f void w_task_listener_stop (w_task_listener *listener)
 *
 * Stops a task listener. This function can be used to make a listener
 * stop accepting connections, and shut down its associated socket.
 */
void
w_task_listener_stop (w_task_listener_t *listener)
{
    CHECK_SCHEDULER ();
    w_assert (listener);
    listener->running = false;
}


/*
 * Input: proto[:host]:port
 * proto: tcp | tcp4 | tcp6
 *  host: * | ip-address | hostname
 *  port: number
 */
static bool
make_listener_socket (w_task_listener_t *listener,
                      const char        *spec)
{
    char *lcolon = strchr (spec, ':');
    char *rcolon = strchr (spec, ':');
    unsigned proto_len = (unsigned) (lcolon - spec);
    unsigned addr_len = (unsigned) (rcolon - lcolon);

    if (!w_str_uint (rcolon + 1, &listener->socket_port))
        return false;

#ifdef W_CONF_IPv6
    bool ipv4_in_ipv6 = false;
    struct sockaddr_in6 sa;
#else
    struct sockaddr_in sa;
#endif /* W_CONF_IPv6 */

    memset (&sa, 0x00, sizeof (sa));
    sa.sin_port = htons (listener->socket_port);

    switch (proto_len) {
        case 3:
            if (!strncmp ("tcp", spec, 3)) {
#ifdef W_CONF_IPv6
                sa.sin_family = AF_INET6;
                ipv4_in_ipv6  = true;
#else
                sa.sin_family = AF_INET;
#endif /* W_CONF_IPv6 */
            } else {
                return false;
            }
            break;

        case 4:
            if (!strncmp ("tcp4", spec, 4)) {
                sa.sin_family = AF_INET;
#ifdef W_CONF_IPv6
            } else if (!strncmp ("tcp6", spec, 4)) {
                sa.sin_family = AF_INET6;
#endif /* W_CONF_IPv6 */
            } else {
                return false;
            }
            break;

        default:
            return false;
    }

#ifdef W_CONF_IPv6
    sa.sin6_addr.s_addr = in6addr_any;
#else
    sa.sin_addr.s_addr = INADDR_ANY;
#endif /* W_CONF_IPv6 */

    if (addr_len) {
        char address[addr_len + 1];
        strncpy (address, lcolon + 1, addr_len);
#ifdef W_CONF_IPv6
#else
#endif /* W_CONF_IPv6 */
    }

    int fd = socket (sa.sin_family, SOCK_STREAM, 0);
    if (fd < 0) return false;

    socklen_t slen;
    int n;
    if (getsockopt (fd, SOL_SOCKET, SO_TYPE, (void*) &n, &slen) >= 0) {
        n = 1;
        setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char*) &n, sizeof (int));
    }

    if (bind (fd, (struct sockaddr*) &sa, sizeof (sa)) == -1) {
        close (fd);
        return false;
    }

    if (listen (fd, 16) == -1) {
        close (fd);
        return false;
    }

    n = fcntl (fd, F_GETFL);
    if (n < 0 || fcntl (fd, F_SETFL, n | O_NONBLOCK) == -1) {
        close (fd);
        return false;
    }

    listener->fd = fd;
    return true;
}


static bool
w_task_listener_init (w_task_listener_t     *listener,
                      const char            *bind_spec,
                      w_task_listener_func_t handler,
                      void                  *userdata)
{
    w_assert (listener);
    w_assert (bind_spec);
    w_assert (handler);

    listener->handle_connection = handler;
    listener->userdata = userdata;

    if (!make_listener_socket (listener, bind_spec))
        return false;

    listener->bind_spec = w_str_dup (bind_spec);
    listener->socket_name = NULL; /* TODO */
    return true;
}


static void
w_task_listener_destroy (void *obj)
{
    w_task_listener_t *listener = obj;
    w_free (listener->bind_spec);
    w_free (listener->socket_name);
}


/*~f w_task_listener_t* w_task_listener_new(const char *bind_spec, w_task_listener_func_t handler, void* userdata)
 *
 * Creates a new task-based socket listener.
 *
 * A listener will create a socket bound to the protocol, address, and port
 * specified by `bind_spec`; using the `handler` function to serve each
 * connection. Once the listener is started, it will use a task for accepting
 * connections, plus one new task to handle each client connection.
 *
 * The `bind_spec` is a string in the ``protocol:port``, or
 * ``protocol:host:port`` formats, and it determines the address to which the
 * listening socket is bound. The value for ``port`` must be numeric, ``host``
 * can be an IP address, or a host name. If a ``host`` value is not supplied,
 * the listener will use the wildcard address to listen on all interfaces. The
 * recognized values for ``protocol`` are:
 *
 * - ``tcp4``: IPv4 TCP socket.
 * - ``tcp6``: IPv6 TCP socket. Only available if the library is build with
 *   ``W_CONF_IPv6``.
 * - ``tcp``: If IPv6 support is built in the library, use a dual-stack
 *   socket, which works both for IPv4 and IPv6; otherwise fall-back to
 *   using IPv4 only.
 *
 * Once a listener has been created, a task for it needs to be created using
 * :func:`w_task_prepare()` using the :func:`w_task_listener_run()` function
 * as task implementation.
 *
 * The following example creates a task-based TCP server which will just
 * accept connections, and write a single line of content to clients:
 *
 * .. code-block:: c
 *
 *    static void
 *    handle_connection (w_task_listener_t *listener, w_io_t *socket) {
 *      w_unused (listener);
 *      w_printerr ("$s: Connection accepted!\n", w_task_name ());
 *      W_IO_NORESULT (w_io_format ("Hello from task $s\n", w_task_name ()));
 *      W_IO_NORESULT (w_io_close (socket));
 *    }
 *
 *    int main () {
 *      w_task_listener_t *listener =
 *              w_task_listener_new ("tcp:9000", handle_connection, NULL);
 *      w_task_prepare (w_task_listener_run, listener, 0);
 *      w_task_run_scheduler ();
 *      w_obj_unref (listener);
 *      return 0;
 *    }
 */
w_task_listener_t*
w_task_listener_new (const char            *bind_spec,
                     w_task_listener_func_t handler,
                     void                  *userdata)
{
    w_assert (bind_spec);
    w_assert (handler);

    w_task_listener_t *listener = w_obj_new (w_task_listener_t);
    if (w_task_listener_init (listener, bind_spec, handler, userdata))
        return w_obj_dtor (listener, w_task_listener_destroy);

    w_obj_unref (listener);
    return NULL;
}

/*~f unsigned w_task_listener_port (const w_task_listener_t *listener)
 *
 * Returns the port to which a listener is bound.
 */

/*~f const char* w_task_listener_host (const w_task_listener_t *listener)
 *
 * Returns the address (or hostname, if available) to which the listener is
 * bound. Do not free the returned string.
 */
