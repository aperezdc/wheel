/*
 * wio-socket.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
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
    (void) w_io_close ((w_io_t*) obj);
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
    char *host;
    int   port;

    w_assert (io);

    switch (kind) {
        case W_IO_SOCKET_UNIX:
            host = va_arg (args, char *);
            return w_io_socket_init_unix (io, (const char*) host);
        case W_IO_SOCKET_TCP4:
            host = va_arg (args, char *);
            port = va_arg (args, int);
            return w_io_socket_init_tcp4 (io, (const char*) host, port);
        default:
            return false;
    }

    return true;
}


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


w_io_t*
w_io_socket_open (w_io_socket_kind_t kind, ...)
{
    w_io_socket_t *io = w_obj_new (w_io_socket_t);
    va_list args;
    bool ret;

    /* Start with an invalid socket fd */
    W_IO_SOCKET_FD (io) = -1;

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


bool
w_io_socket_connect (w_io_socket_t *io)
{
    w_assert (io);

    return (connect (W_IO_SOCKET_FD (io),
                     (struct sockaddr*) io->sa,
                     io->slen) != -1);
}


bool
w_io_socket_serve (w_io_socket_t *io,
                   w_io_socket_serve_mode_t mode,
                   bool (*handler) (w_io_socket_t*))
{
    bool (*mode_handler) (w_io_socket_t*, bool (*) (w_io_socket_t*));
    struct sockaddr sa;
    socklen_t slen;
    bool ret;
    int fd;

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
#else /* !W_CONF_PTHREAD */
            w_die ("libwheel was built without pthread support\n");
#endif /* W_CONF_PTHREAD */
            break;
        case W_IO_SOCKET_SINGLE:
            mode_handler = w_io_socket_serve_single;
            break;
        case W_IO_SOCKET_FORK:
            mode_handler = w_io_socket_serve_fork;
            break;
        default:
            mode_handler = NULL;
    }
    w_assert (mode_handler);

    if (bind (W_IO_SOCKET_FD (io), (struct sockaddr*) io->sa, io->slen) == -1) {
        return false;
    }
    io->bound = true;

    if (listen (W_IO_SOCKET_FD (io), W_IO_SOCKET_BACKLOG) == -1) {
        return false;
    }

    slen = W_IO_SOCKET_SA_LEN;
    while ((fd = accept (W_IO_SOCKET_FD(io), &sa, &slen)) != -1) {
        w_io_socket_t *nio = w_obj_new (w_io_socket_t);

        w_io_unix_init_fd ((w_io_unix_t*) nio, fd);
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


bool
w_io_socket_send_eof (w_io_socket_t *io)
{
    w_assert (io);
    return shutdown (W_IO_SOCKET_FD (io), SHUT_WR) != -1;
}


const char*
w_io_socket_unix_path (w_io_socket_t *io)
{
    struct sockaddr_un *un;
    w_assert (io);

    un = (struct sockaddr_un*) io->sa;
    return un->sun_path;
}
