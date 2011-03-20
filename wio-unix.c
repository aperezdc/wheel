/*
 * wio-unix.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
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
w_io_unix_write (w_io_t *io, const void *buf, size_t len)
{
    ssize_t ret, n = len;

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

    return ret;
}


w_io_t*
w_io_unix_open (int fd)
{
    w_io_unix_t *io = w_obj_new (w_io_unix_t);
    w_io_unix_init (io, fd);
    return (w_io_t*) io;
}


void
w_io_unix_init (w_io_unix_t *io, int fd)
{
    w_assert (io);
    w_assert (fd >= 0);

    w_io_init ((w_io_t*) io);

    io->parent.close = w_io_unix_close;
    io->parent.write = w_io_unix_write;
    io->parent.read  = w_io_unix_read;
    io->fd = fd;
}

