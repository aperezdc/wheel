/*
 * wio-socket.c
 * Copyright (C) 2010-2015 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * .. _wio-socket:
 *
 * Input/Output on Sockets
 * =======================
 *
 * Provides support for using the :ref:`stream functions <wio-functions>` to
 * read and write to and from sockets.
 *
 * The following kinds of sockets are supported:
 *
 * - TCP sockets.
 * - Unix sockets.
 *
 *
 * Usage
 * -----
 *
 * Client
 * ~~~~~~
 *
 * To connect a socket to a remote server (and use it as a client), the usual
 * procedure is as follows:
 *
 * #. Create the socket with :func:`w_io_socket_open()`.
 *
 * #. Connect to the remote endpoint using :func:`w_io_socket_connect()`.
 *
 * #. Exchange data using :func:`w_io_write()` and :func:`w_io_read()`, or any
 *    other of the :ref:`stream functions <wio-functions>`.
 *
 * #. *(Optional)* Once no more data needs to be written, a half-close can be
 *    performed using :func:`w_io_socket_send_eof()`. It will still be
 *    possible to read data from the stream.
 *
 * #. Close the socket, using :func:`w_io_close()` or calling
 *    :func:`w_obj_unref()` on it and letting it be closed and destroyed when
 *    no more references to the socket are held.
 *
 * The following code will make an HTTP request and read the response back
 * into a :type:`w_buf_t`:
 *
 * .. code-block:: c
 *
 *      w_buf_t
 *      http_get (const char *ip_address, unsigned port, const char *resource)
 *      {
 *          w_lobj w_io_t *stream = w_io_socket_open (W_IO_SOCKET_TCP4,
 *                                                    ip_address,
 *                                                    port);
 *          if (!stream || !w_io_socket_connect ((w_io_socket_t*) stream))
 *              w_die ("Cannot create socket and connect to $s\n", ip_address);
 *
 *          W_IO_NORESULT (w_io_format (stream, "GET $s HTTP/1.0\r\n", resource));
 *          w_io_socket_send_eof ((w_io_socket_t*) stream);
 *
 *          w_buf_t response = W_BUF;
 *          char buf[512];
 *
 *          for (;;) {
 *              w_io_result_t r = w_io_read (stream, buf, w_lengthof (buf));
 *              if (w_io_failed (r))
 *                  w_die ("Error reading from $s: $R\n", ip_address, r);
 *              if (w_io_result_bytes (r) > 0)
 *                  w_buf_append_mem (&response, buf, w_io_result_bytes (r));
 *              else if (w_io_eof (r))
 *                  break;
 *          }
 *
 *          return response;
 *      }
 *
 *
 * Server
 * ~~~~~~
 *
 * Using a socket to server requests is a bit more convoluted, but still
 * much easier than using the sockets API directly. The overall procedure
 * is:
 *
 * #. Define a request handler function which conforms to the following
 *    signature:
 *
 *    .. code-block:: c
 *
 *          bool (*request_handler) (w_io_socket_t *socket)
 *
 * #. Create the socket with :func:`w_io_socket_open()`.
 *
 * #. Start serving requests with :func:`w_io_socket_serve()`. The function
 *    will bind to the address specified when creating the socket and start
 *    serving requests. For each request, the request handler function will
 *    be called with a socket that can be used to handle the request.
 *
 * The following code implements a simple TCP echo server:
 *
 * .. code-block:: c
 *
 *      static bool
 *      handle_echo_request (w_io_socket_t *socket)
 *      {
 *          char buf[512];
 *          for (;;) {
 *              w_io_result_t r = w_io_read ((w_io_t*) socket, buf, w_lengthof (buf));
 *              if (w_io_failed (r))
 *                  w_die ("Error reading from client: $R\n", r);
 *              if (w_io_eof (r))
 *                  break;
 *
 *              r = w_io_write ((w_io_t*) socket, buf, w_io_result_bytes (r));
 *              if (w_io_failed (r))
 *                  w_die ("Error writing to client: $R\n", r);
 *          }
 *          w_io_socket_send_eof (socket);
 *
 *          return true;  // Keep accepting more requests.
 *      }
 *
 *      int main (void)
 *      {
 *          w_lobj w_io_t *socket = w_io_socket_open (W_IO_SOCKET_TCP4,
 *                                                    "0.0.0.0",  // Address.
 *                                                    4242);      // Port.
 *          if (!socket)
 *              w_die ("Cannot create socket: $E\n");
 *
 *          if (!w_io_socket_serve ((w_io_socket_t*) socket,
 *                                  W_IO_SOCKET_FORK,  // Handle each request in a child process.
 *                                  handle_echo_request))
 *              w_dir ("Cannot accept connections: $E\n");
 *
 *          return 0;
 *      }
 *
 *
 * Types
 * -----
 */

/*~t w_io_socket_t
 *
 * Performs input/output on sockets.
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#ifdef W_CONF_PTHREAD
#include <pthread.h>
#endif /* W_CONF_PTHREAD */

#ifndef W_IO_SOCKET_BACKLOG
#define W_IO_SOCKET_BACKLOG 1024
#endif /* !W_IO_SOCKET_BACKLOG */

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + \
                      strlen ((ptr)->sun_path))
#endif /* SUN_LEN */


static void
w_io_socket_cleanup (void *obj)
{
    w_io_socket_t *io = (w_io_socket_t*) obj;

    if (io->bound && io->kind == W_IO_SOCKET_UNIX) {
        struct sockaddr_un *un = (struct sockaddr_un*) io->sa;
        unlink (un->sun_path);
    }
    /* Unfortunately, errors can't be reported here. */
    W_IO_NORESULT (w_io_close ((w_io_t*) obj));
}


static bool
w_io_socket_init_unix (w_io_socket_t *io, const char *path)
{
    struct sockaddr_un *un;
    int fd;

    w_assert (io);
    w_assert (path);
    w_assert (*path);

    /* Create socket fd. */
    if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        return false;
    }

    un = (struct sockaddr_un*) io->sa;

    w_io_unix_init_fd ((w_io_unix_t*) io, fd);

    strcpy (un->sun_path, path);
    un->sun_family = AF_UNIX;
    io->kind = W_IO_SOCKET_UNIX;
	io->slen = SUN_LEN (un);

    w_assert (memcmp (un, io->sa, io->slen) == 0);

    w_obj_dtor (io, w_io_socket_cleanup);
    return true;
}


static bool
w_io_socket_init_tcp4 (w_io_socket_t *io, const char *host, int port)
{
    struct sockaddr_in *in;
    int fd;

    w_assert (io);
    w_assert (host);
    w_assert (port > 0); /* XXX Maybe allow <=0 for random port number? */
    w_assert (port <= 0xFFFF);

    in = (struct sockaddr_in*) io->sa;

    if (host) {
        if (!inet_aton (host, &in->sin_addr)) {
            errno = EINVAL;
            return false;
        }
    }
    else {
        in->sin_addr.s_addr = INADDR_ANY;
    }

    in->sin_port = htons (port);
    in->sin_family = AF_INET;

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        return false;
    }

    w_io_unix_init_fd ((w_io_unix_t*) io, fd);

    io->slen = sizeof (struct sockaddr_in);
    io->kind = W_IO_SOCKET_TCP4;

    w_assert (memcmp (in, io->sa, io->slen) == 0);

    w_obj_dtor (io, w_io_socket_cleanup);
    return true;
}


static bool
w_io_socket_initv (w_io_socket_t *io, w_io_socket_kind_t kind, va_list args)
{
    const char *host;
    int port;

    w_assert (io);

    switch (kind) {
        case W_IO_SOCKET_UNIX:
            host = va_arg (args, const char*);
            return w_io_socket_init_unix (io, host);
        case W_IO_SOCKET_TCP4:
            host = va_arg (args, const char*);
            port = va_arg (args, int);
            return w_io_socket_init_tcp4 (io, host, port);
        default:
            return false;
    }

    return true;
}


/*~f bool w_io_socket_init (w_io_socket_t *stream, w_io_socket_kind_t kind, ...)
 *
 * Initializes a socket `stream` (possibly allocated in the stack).
 *
 * For a description of the additional function arguments, please check the
 * documentation for :func:`w_io_socket_open()`.
 */
bool
w_io_socket_init (w_io_socket_t *io, w_io_socket_kind_t kind, ...)
{
    bool ret;
    va_list args;
    va_start (args, kind);
    ret = w_io_socket_initv (io, kind, args);
    va_end (args);
    return ret;
}


/*~f w_io_t* w_io_socket_open (w_io_socket_kind_t kind, ...)
 *
 * Create a new socket of a given `kind`.
 *
 * Sockets are created in a state in which they can be used both for
 * client and server sockets:
 *
 * - :func:`w_io_socket_connect()` will put the socket in client mode
 *   and connect it to the specified address.
 *
 * - :func:`w_io_socket_serve()` will put the socket in server mode, and
 *   start listening for connections at the specified address.
 *
 * The parameters that need to be passed to this function vary depending on
 * the `kind` of the socket being created.
 *
 * **Unix sockets:**
 *      Pass ``W_IO_SOCKET_UNIX`` as `kind`, and the path in the file system
 *      where the socket is to be created (or connected to).
 *
 * **TCP sockets:**
 *      Pass ``W_IO_SOCKET_TCP4`` as `kind`, plus the IP address (as a string)
 *      and the port to use (or to connect to).
 */
w_io_t*
w_io_socket_open (w_io_socket_kind_t kind, ...)
{
    w_io_socket_t *io = w_obj_new (w_io_socket_t);
    va_list args;
    bool ret;

    /* Start with an invalid socket fd */
    io->parent.fd = -1;

    va_start (args, kind);
    ret = w_io_socket_initv (io, kind, args);
    va_end (args);

    if (!ret) {
        w_obj_unref (io);
        return NULL;
    }

    return (w_io_t*) io;
}


#ifdef W_CONF_PTHREAD
struct w_io_socket_thread
{
    pthread_t      thread;
    bool        (*hnd) (w_io_socket_t*);
    w_io_socket_t *io;
};


static void*
w_io_socket_serve_thread_run (void *udata)
{
    struct w_io_socket_thread *st = (struct w_io_socket_thread*) udata;

    /*
     * Call the handler.
     *
     * TODO Take into account the return value that signals the finalization
     * of the accept-serve loop.
     */
    (*st->hnd) (st->io);

    /*
     * Cleanup after the handler. Make sure we remove the reference the
     * thread holds to the I/O object.
     *
     * XXX It is a bit tricky that the deallocation of this is in a thread
     * different than the thread that created the object. This may be
     * revisited to make deallocations happen in the origin thread if
     * problems arise. Let's keep things simple for the moment, sacrifice
     * a goat and pray...
     */
    w_obj_unref (st->io);

    /* XXX Is it safe to free the pthread_t before it finished running? */
    w_free (st);

    return NULL;
}


static bool
w_io_socket_serve_thread (w_io_socket_t *io,
                          bool (*handler) (w_io_socket_t*))
{
    struct w_io_socket_thread *st = w_new0 (struct w_io_socket_thread);

    /*
     * The thread keeps a reference to the I/O object. The reference counter
     * is incremented here before the thread is spawned to avoid a race
     * condition with the w_obj_unref() in the accept-serve loop.
     */
    st->io  = w_obj_ref (io);
    st->hnd = handler;

    /*
     * FIXME This lacks proper error handling... If a thread cannot be
     * created, the accept-loop just bails out, which is definitely not
     * the most sane thing to do.
     */
    return (pthread_create (&st->thread, NULL,
                            w_io_socket_serve_thread_run,
                            st) == 0);
}
#endif /* W_CONF_PTHREAD */


static bool
w_io_socket_serve_fork (w_io_socket_t *io,
                        bool (*handler) (w_io_socket_t*))
{
    pid_t pid;

    w_assert (handler);
    w_assert (io);

    if ((pid = fork ()) != 0) {
        return pid != -1;
    }
    exit (((*handler) (io)) ? EXIT_SUCCESS : EXIT_FAILURE);

    /* Keep compiler happy */
    return false;
}


static bool
w_io_socket_serve_single (w_io_socket_t *io,
                          bool (*handler) (w_io_socket_t*))
{
    w_assert (handler);
    w_assert (io);
    return (*handler) (io);
}


/*~f bool w_io_socket_connect (w_io_socket_t *socket)
 *
 * Connect a `socket` to a server.
 *
 * This makes a connection to the host specified when creating the
 * socket with :func:`w_io_socket_open()`, and puts it in client mode.
 * Once the socket is successfully connected, read and write operations
 * can be performed in the socket.
 *
 * The return value indicates whether the connection was successful.
 */
bool
w_io_socket_connect (w_io_socket_t *io)
{
    w_assert (io);
    int fd = w_io_get_fd ((w_io_t*) io);
    return fd >= 0 && (connect (fd, (struct sockaddr*) io->sa, io->slen) != -1);
}


/*~f bool w_io_socket_serve (w_io_socket_t *socket, w_io_socket_serve_mode_t mode, bool (*handler) (w_io_socket_t*))
 *
 * Serves requests using a `socket`. This function will start a loop accepting
 * connections, and for each connection an open socket will be passed to the
 * given `handler` function. The `mode` in which each request is served can be
 * specified:
 *
 * - ``W_IO_SOCKET_SINGLE``: Each request is served by calling directly and
 *   waiting for it to finish. This makes impossible to serve more than one
 *   request at a time.
 *
 * - ``W_IO_SOCKET_THREAD``: Each request is served in a new thread. The
 *   handler is invoked in that thread.
 *
 * - ``W_IO_SOCKET_FORK``: A new process is forked for each request. The
 *   handler is invoked in the child process.
 */
bool
w_io_socket_serve (w_io_socket_t *io,
                   w_io_socket_serve_mode_t mode,
                   bool (*handler) (w_io_socket_t*))
{
    bool (*mode_handler) (w_io_socket_t*, bool (*) (w_io_socket_t*));
    struct sockaddr sa;
    socklen_t slen;
    bool ret;

#ifdef W_CONF_PTHREAD
    w_assert (mode == W_IO_SOCKET_SINGLE ||
              mode == W_IO_SOCKET_THREAD ||
              mode == W_IO_SOCKET_FORK);
#else  /* W_CONF_PTHREAD */
    w_assert (mode == W_IO_SOCKET_SINGLE ||
              mode == W_IO_SOCKET_FORK);
#endif /* W_CONF_PTHREAD */

    w_assert (handler);
    w_assert (io);

    switch (mode) {
#ifdef W_CONF_PTHREAD
        case W_IO_SOCKET_THREAD:
            mode_handler = w_io_socket_serve_thread;
            break;
#else /* !W_CONF_PTHREAD */
            W_WARN ("libwheel was built without pthread support. "
                    "Using forking mode instead as a fall-back.\n");
#endif /* W_CONF_PTHREAD */
        case W_IO_SOCKET_FORK:
            mode_handler = w_io_socket_serve_fork;
            break;
        case W_IO_SOCKET_SINGLE:
            mode_handler = w_io_socket_serve_single;
            break;
        default:
            mode_handler = NULL;
    }
    w_assert (mode_handler);

    int fd = w_io_get_fd ((w_io_t*) io);
    if (fd < 0)
        return false;

    if (bind (fd, (struct sockaddr*) io->sa, io->slen) == -1) {
        return false;
    }
    io->bound = true;

    if (listen (fd, W_IO_SOCKET_BACKLOG) == -1) {
        return false;
    }

    int new_fd = -1;
    slen = W_IO_SOCKET_SA_LEN;
    while ((new_fd = accept (fd, &sa, &slen)) != -1) {
        w_io_socket_t *nio = w_obj_new (w_io_socket_t);

        w_io_unix_init_fd ((w_io_unix_t*) nio, new_fd);
        memcpy (io->sa, &sa, slen);
        nio->kind = io->kind;
        nio->slen = slen;

        ret = (*mode_handler) (nio, handler);
        w_obj_unref (nio);
        if (!ret) {
            break;
        }
        slen = W_IO_SOCKET_SA_LEN;
    }

    return true;
}


/*~f bool w_io_socket_send_eof (w_io_socket_t *socket)
 *
 * Half-close a `socket` on the write direction. This closes the socket,
 * but only in writing one direction, so other endpoint will think that
 * the end of the stream was reached (thus the operation is conceptually
 * equivalent to sending and “end of file marker”). Read operations can
 * still be performed in a socket which was half-closed using this
 * function.
 *
 * Note that for completely closing the socket, :func:`w_io_close()`
 * should be used instead.
 *
 * The return value indicates whether the half-close was successful.
 */
bool
w_io_socket_send_eof (w_io_socket_t *io)
{
    w_assert (io);
    int fd = w_io_get_fd ((w_io_t*) io);
    return fd >= 0 && shutdown (fd, SHUT_WR) != -1;
}


/*~f const char* w_io_socket_unix_path (w_io_socket_t *socket)
 *
 * Obtain the path in the filesystem for an Unix socket.
 *
 * Note that the result is undefined if the socket is of any other kind than
 * an Unix socket.
 */
const char*
w_io_socket_unix_path (w_io_socket_t *io)
{
    struct sockaddr_un *un;
    w_assert (io);

    un = (struct sockaddr_un*) io->sa;
    return un->sun_path;
}
