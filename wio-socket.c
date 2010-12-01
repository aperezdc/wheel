/*
 * wio-socket.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
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


#ifndef W_IO_SOCKET_BACKLOG
#define W_IO_SOCKET_BACKLOG 1024
#endif /* !W_IO_SOCKET_BACKLOG */


static void
w_io_socket_cleanup (void *obj)
{
    w_io_socket_t *io = (w_io_socket_t*) obj;

    if (io->bound && io->kind == W_IO_SOCKET_UNIX) {
        struct sockaddr_un *un = (struct sockaddr_un*) io->sa;
        unlink (un->sun_path);
    }
    w_io_close ((w_io_t*) obj);
}


static wbool
w_io_socket_init_unix (w_io_socket_t *io, const char *path)
{
    struct sockaddr_un *un;
    int fd;

    w_assert (io);
    w_assert (path);
    w_assert (*path);

    /* Create socket fd. */
    if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        return W_NO;
    }

    un = (struct sockaddr_un*) io->sa;

    w_io_unix_init ((w_io_unix_t*) io, fd);

    strcpy (un->sun_path, path);
    un->sun_family = AF_UNIX;
    io->kind = W_IO_SOCKET_UNIX;
    io->slen = SUN_LEN (un);

    w_assert (memcmp (un, io->sa, io->slen) == 0);

    w_obj_dtor (io, w_io_socket_cleanup);
    return W_YES;
}


static wbool
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
            return W_NO;
        }
    }
    else {
        in->sin_addr.s_addr = INADDR_ANY;
    }

    in->sin_port = htons (port);
    in->sin_family = AF_INET;

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        return W_NO;
    }

    w_io_unix_init ((w_io_unix_t*) io, fd);

    io->slen = sizeof (struct sockaddr_in);
    io->kind = W_IO_SOCKET_TCP4;

    w_assert (memcmp (in, io->sa, io->slen) == 0);

    w_obj_dtor (io, w_io_socket_cleanup);
    return W_YES;
}


static wbool
w_io_socket_initv (w_io_socket_t *io, enum w_io_flag kind, va_list args)
{
    w_assert (io);

    switch (kind) {
        case W_IO_SOCKET_UNIX:
            return w_io_socket_init_unix (io, va_arg (args, const char*));
        case W_IO_SOCKET_TCP4:
            return w_io_socket_init_tcp4 (io, va_arg (args, const char*),
                                              va_arg (args, int));
        default:
            return W_NO;
    }

    return W_YES;
}


wbool
w_io_socket_init (w_io_socket_t *io, w_io_socket_kind_t kind, ...)
{
    wbool ret;
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
    wbool ret;

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


struct w_io_socket_thread
{
    wbool   ret;
    wbool (*hnd) (w_io_t*);
    w_io_t d[0];
};


static wbool
w_io_socket_serve_thread (w_io_socket_t *io,
                          wbool (*handler) (w_io_socket_t*))
{
    w_unused (io);
    w_unused (handler);
    return W_YES; /* Stop serving */
}


static wbool
w_io_socket_serve_fork (w_io_socket_t *io,
                        wbool (*handler) (w_io_socket_t*))
{
    pid_t pid;

    w_assert (handler);
    w_assert (io);

    if ((pid = fork ()) != 0) {
        return pid != -1;
    }
    exit (((*handler) (io)) ? EXIT_SUCCESS : EXIT_FAILURE);

    /* Keep compiler happy */
    return W_NO;
}


static wbool
w_io_socket_serve_single (w_io_socket_t *io,
                          wbool (*handler) (w_io_socket_t*))
{
    w_assert (handler);
    w_assert (io);
    return (*handler) (io);
}


wbool
w_io_socket_connect (w_io_socket_t *io)
{
    w_assert (io);

    return (connect (W_IO_SOCKET_FD (io),
                     (struct sockaddr*) io->sa,
                     io->slen) != -1);
}


wbool
w_io_socket_serve (w_io_socket_t *io,
                   w_io_socket_serve_mode_t mode,
                   wbool (*handler) (w_io_socket_t*))
{
    wbool (*mode_handler) (w_io_socket_t*, wbool (*) (w_io_socket_t*));
    struct sockaddr sa;
    socklen_t slen;
    wbool ret;
    int fd;

    w_assert (handler == W_IO_SOCKET_SINGLE ||
              handler == W_IO_SOCKET_THREAD ||
              handler == W_IO_SOCKET_FORK);
    w_assert (handler);
    w_assert (io);

    switch (mode) {
        case W_IO_SOCKET_THREAD:
            mode_handler = w_io_socket_serve_thread;
            break;
        case W_IO_SOCKET_SINGLE:
            mode_handler = w_io_socket_serve_single;
            break;
        case W_IO_SOCKET_FORK:
            mode_handler = w_io_socket_serve_fork;
            break;
    }

    if (bind (W_IO_SOCKET_FD (io), (struct sockaddr*) io->sa, io->slen) == -1) {
        return W_NO;
    }
    io->bound = W_YES;

    if (listen (W_IO_SOCKET_FD (io), W_IO_SOCKET_BACKLOG) == -1) {
        return W_NO;
    }

    slen = W_IO_SOCKET_SA_LEN;
    while ((fd = accept (W_IO_SOCKET_FD(io), &sa, &slen)) != -1) {
        w_io_socket_t *nio = w_obj_new (w_io_socket_t);

        w_io_unix_init ((w_io_unix_t*) nio, fd);
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

    return W_YES;
}


wbool
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