/*
 * wio-unix.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


static wbool
w_io_unix_close (w_io_t *iobase)
{
    w_io_unix_t *io = (w_io_unix_t*) iobase;
    if (io->fd >= 0) {
        return (close (io->fd) == 0);
    }
    return W_YES;
}


static ssize_t
w_io_unix_write (w_io_t *io, const void *bufp, size_t len)
{
    ssize_t ret, n = len;
    const char *buf = bufp;

    while (len > 0) {
        do {
            ret = write (((w_io_unix_t*) io)->fd, buf, len);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            return ret;
        }

        buf += ret;
        len -= ret;
    }

    return n;
}


static ssize_t
w_io_unix_read (w_io_t *io, void *buf, size_t len)
{
    ssize_t ret;

    do {
        ret = read (((w_io_unix_t*) io)->fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    if (ret == 0)
        ret = W_IO_EOF;

    return ret;
}


static wbool
w_io_unix_flush (w_io_t *io)
{
    return fsync (((w_io_unix_t*) io)->fd) != 0;
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


wbool
w_io_unix_init (w_io_unix_t *io,
                const char  *path,
                int          mode,
                unsigned     perm)
{
    int fd;
    w_assert (io);
    w_assert (path);

    if ((fd = open (path, mode, perm)) < 0)
        return W_NO;

    w_io_unix_init_fd (io, fd);
    return W_YES;
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
    io->fd = fd;
}


static w_io_unix_t stdout_data =
{
    /* w_io_t */
    {
        W_OBJ_STATIC (w_io_unix_close),

        W_IO_EOF,        /* backch */
        w_io_unix_close, /* close  */
        w_io_unix_write, /* write  */
        w_io_unix_read,  /* read   */
        w_io_unix_flush, /* flush  */
    },
    STDOUT_FILENO,       /* fd     */
};

static w_io_unix_t stderr_data =
{
    /* w_io_t */
    {
        W_OBJ_STATIC (w_io_unix_close),

        W_IO_EOF,        /* backch */
        w_io_unix_close, /* close  */
        w_io_unix_write, /* write  */
        w_io_unix_read,  /* read   */
        w_io_unix_flush, /* flush  */
    },
    STDERR_FILENO,       /* fd     */
};

static w_io_unix_t stdin_data =
{
    /* w_io_t */
    {
        W_OBJ_STATIC (w_io_unix_close),

        W_IO_EOF,        /* backch */
        w_io_unix_close, /* close  */
        w_io_unix_write, /* write  */
        w_io_unix_read,  /* read   */
        w_io_unix_flush, /* flush  */
    },
    STDIN_FILENO,        /* fd     */
};

/* Public standard I/O objects */
w_io_t *w_stdout = (w_io_t*) &stdout_data;
w_io_t *w_stderr = (w_io_t*) &stderr_data;
w_io_t *w_stdin  = (w_io_t*) &stdin_data;
