/*
 * wio.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <errno.h>


wbool
w_io_close (const w_io_t *io)
{
    w_assert (io);
    return (io->close) ? (*io->close) (W_IO_UDATA (io, void)) : W_YES;
}


ssize_t
w_io_read (const w_io_t *io, void *buf, size_t len)
{
    w_assert (io);
    w_assert (buf);

    return (io->read)
        ? (*io->read) (W_IO_UDATA (io, void), buf, len)
        : (errno = EBADF, -1);
}


ssize_t
w_io_write (const w_io_t *io, const void *buf, size_t len)
{
    w_assert (io);
    w_assert (buf);

    return (io->write)
        ? (*io->write) (W_IO_UDATA (io, void), buf, len)
        : (errno = EBADF, -1);
}


int
w_io_getchar (const w_io_t *io)
{
    ssize_t ret;
    char ch;

    if ((ret = w_io_read (io, &ch, 1)) == 1)
        return ch;

    return (ret == 0) ? W_IO_EOF : W_IO_ERR;
}


wbool
w_io_putchar (const w_io_t *io, int ch)
{
    char bch = ch;
    return (w_io_write (io, &bch, 1) != 1);
}


ssize_t
w_io_format (const w_io_t *io, const char *fmt, ...)
{
    ssize_t ret;
    va_list args;

    w_assert (io);
    w_assert (fmt);

    va_start (args, fmt);
    ret = w_io_formatv (io, fmt, args);
    va_end (args);
    return ret;
}


ssize_t
w_io_formatv (const w_io_t *io, const char *fmt, va_list args)
{
    w_assert (io);
    w_assert (fmt);

    /* TODO */
    w_unused (io);
    w_unused (fmt);
    w_unused (args);

    return -1;
}

