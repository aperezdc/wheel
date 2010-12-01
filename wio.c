/*
 * wio.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <errno.h>


static void
w_io_cleanup (void *obj)
{
    w_io_close ((w_io_t*) obj);
}


void
w_io_init (w_io_t *io)
{
    w_assert (io);

    io->backch = W_IO_EOF;
    w_obj_dtor (io, w_io_cleanup);
}


wbool
w_io_close (w_io_t *io)
{
    wbool ret;
    w_assert (io);

    if (io->close) {
        ret = (*io->close) (io);
        io->close = NULL;
    }
    else {
        ret = W_YES;
    }
    return ret;
}


ssize_t
w_io_read (w_io_t *io, void *buf, size_t len)
{
    w_assert (io);
    w_assert (buf);
    w_assert (len > 0);

    /* Handle the putback character... makes things a bit messier */
    if (w_unlikely (io->backch != W_IO_EOF)) {
        *((char*) buf++) = io->backch;
        io->backch = W_IO_EOF;

        /* Check whether more characters are to be read */
        if (!--len) {
            return W_YES;
        }
    }

    return (io->read)
        ? (*io->read) (io, buf, len)
        : (errno = EBADF, -1);
}


ssize_t
w_io_write (w_io_t *io, const void *buf, size_t len)
{
    w_assert (io);
    w_assert (buf);
    w_assert (len > 0);

    return (io->write)
        ? (*io->write) (io, buf, len)
        : (errno = EBADF, -1);
}


int
w_io_getchar (w_io_t *io)
{
    ssize_t ret;
    char ch;

    w_assert (io);

    /* If there is a putback character, just return it */
    if (w_unlikely (io->backch != W_IO_EOF)) {
        ch = io->backch;
        io->backch = W_IO_EOF;
        return ch;
    }

    if ((ret = w_io_read (io, &ch, 1)) == 1)
        return ch;

    return (ret == 0) ? W_IO_EOF : W_IO_ERR;
}


wbool
w_io_putchar (w_io_t *io, int ch)
{
    char bch = ch;
    return (w_io_write (io, &bch, 1) != 1);
}


void
w_io_putback (w_io_t *io, char ch)
{
    w_assert (io);
    io->backch = ch;
}


ssize_t
w_io_format (w_io_t *io, const char *fmt, ...)
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
w_io_formatv (w_io_t *io, const char *fmt, va_list args)
{
    w_assert (io);
    w_assert (fmt);

    /* TODO */
    w_unused (io);
    w_unused (fmt);
    w_unused (args);

    return -1;
}

