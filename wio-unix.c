/*
 * wio-unix.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


static w_io_result_t
w_io_unix_close (w_io_t *iobase)
{
    w_io_unix_t *io = (w_io_unix_t*) iobase;
    if (io->fd >= 0) {
        if (close (io->fd) == -1)
            return W_IO_RESULT_ERROR (errno);
    }
    return W_IO_RESULT_SUCCESS;
}


static w_io_result_t
w_io_unix_write (w_io_t *io, const void *bufp, size_t len)
{
    ssize_t ret, n = len;
    const char *buf = bufp;

    while (len > 0) {
        do {
            ret = write (((w_io_unix_t*) io)->fd, buf, len);
        } while (ret < 0 && errno == EINTR);

        if (ret == -1)
            return W_IO_RESULT_ERROR (errno);

        buf += ret;
        len -= ret;
    }

    return W_IO_RESULT (n);
}


static w_io_result_t
w_io_unix_read (w_io_t *io, void *buf, size_t len)
{
    ssize_t ret;

    do {
        ret = read (((w_io_unix_t*) io)->fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    if (ret == -1)
        return W_IO_RESULT_ERROR (errno);
    if (ret == 0)
        return W_IO_RESULT_EOF;

    return W_IO_RESULT (ret);
}


static w_io_result_t
w_io_unix_flush (w_io_t *io)
{
    if (fsync (((w_io_unix_t*) io)->fd) == -1)
        return W_IO_RESULT_ERROR (errno);

    return W_IO_RESULT_SUCCESS;
}


static int
w_io_unix_getfd (w_io_t *io)
{
    return ((w_io_unix_t*) io)->fd;
}


w_io_t*
w_io_unix_open (const char *path,
                int         mode,
                unsigned    perm)
{
    int fd;
    w_assert (path);

    return ((fd = open (path, mode, perm)) < 0) ? NULL : w_io_unix_open_fd (fd);
}

w_io_t*
w_io_unix_open_fd (int fd)
{
    w_io_unix_t *io = w_obj_new (w_io_unix_t);
    w_io_unix_init_fd (io, fd);
    return (w_io_t*) io;
}


bool
w_io_unix_init (w_io_unix_t *io,
                const char  *path,
                int          mode,
                unsigned     perm)
{
    int fd;
    w_assert (io);
    w_assert (path);

    if ((fd = open (path, mode, perm)) < 0)
        return false;

    w_io_unix_init_fd (io, fd);
    return true;
}


void
w_io_unix_init_fd (w_io_unix_t *io, int fd)
{
    w_assert (io);
    w_assert (fd >= 0);

    w_io_init ((w_io_t*) io);

    io->parent.close = w_io_unix_close;
    io->parent.write = w_io_unix_write;
    io->parent.read  = w_io_unix_read;
    io->parent.flush = w_io_unix_flush;
    io->parent.getfd = w_io_unix_getfd;
    io->fd = fd;
}


static w_io_unix_t stdout_data =
{
    .parent = {
        .parent = W_OBJ_STATIC (w_io_unix_close),
        .backch = W_IO_EOF,
        .close  = w_io_unix_close,
        .write  = w_io_unix_write,
        .read   = w_io_unix_read,
        .flush  = w_io_unix_flush,
        .getfd  = w_io_unix_getfd,
    },
    .fd = STDOUT_FILENO,
};

static w_io_unix_t stderr_data =
{
    .parent = {
        .parent = W_OBJ_STATIC (w_io_unix_close),
        .backch = W_IO_EOF,
        .close  = w_io_unix_close,
        .write  = w_io_unix_write,
        .read   = w_io_unix_read,
        .flush  = w_io_unix_flush,
        .getfd  = w_io_unix_getfd,
    },
    .fd = STDERR_FILENO,
};

static w_io_unix_t stdin_data =
{
    .parent = {
        .parent = W_OBJ_STATIC (w_io_unix_close),
        .backch = W_IO_EOF,
        .close  = w_io_unix_close,
        .write  = w_io_unix_write,
        .read   = w_io_unix_read,
        .flush  = w_io_unix_flush,
        .getfd  = w_io_unix_getfd,
    },
    .fd = STDIN_FILENO,
};

/* Public standard I/O objects */
w_io_t *w_stdout = &stdout_data.parent;
w_io_t *w_stderr = &stderr_data.parent;
w_io_t *w_stdin  = &stdin_data.parent;
