/*
 * wio-unix.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
#include <errno.h>


static wbool
w_io_unix_close (void *udata)
{
    return (close (*((int*) udata)) == 0);
}


static ssize_t
w_io_unix_write (void *udata, const void *buf, size_t len)
{
    ssize_t ret, n = len;

    while (len > 0) {
        do {
            ret = write (*((int*) udata), buf, len);
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
w_io_unix_read (void *udata, void *buf, size_t len)
{
    ssize_t ret;

    do {
        ret = read (*((int*) udata), buf, len);
    } while (ret < 0 && errno == EINTR);

    return ret;
}


void
w_io_unix_open (w_io_t *io, int fd)
{
    w_assert (io);
    w_assert (fd >= 0);

    io->close = w_io_unix_close;
    io->write = w_io_unix_write;
    io->read  = w_io_unix_read;

    *W_IO_UDATA (io, int) = fd;
}
