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

#include "wheel.h"
#include "queue.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
#include <ucontext.h>
#include <unistd.h>
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


w_task_t*
w_task_current (void)
{
    CHECK_SCHEDULER ();
    return s_current_task;
}


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


bool
w_task_get_is_system (w_task_t *task)
{
    w_assert (task);
    return task->is_system;
}


void
w_task_set_name (w_task_t *task, const char *name)
{
    w_assert (task);
    w_free (task->name);
    task->name = name ? w_str_dup (name) : NULL;
}


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


void
w_task_yield (void)
{
    CHECK_SCHEDULER ();
    yield_to_scheduler (TASK_YIELD);
}


void
w_task_exit (void)
{
    CHECK_SCHEDULER ();
    yield_to_scheduler (TASK_EXIT);
}


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


}
