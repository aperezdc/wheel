/*
 * wio-buf.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <string.h>


static wbool
w_io_buf_close (w_io_t *iobase)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;

    if (io->own) {
        w_buf_free (&io->buf);
    }

    return W_YES;
}


static ssize_t
w_io_buf_write (w_io_t *iobase, const void *buf, size_t len)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;

    w_buf_length_set (&io->buf, io->pos);
    w_buf_append_mem (&io->buf, buf, len);
    io->pos += len;
    return len;
}


static ssize_t
w_io_buf_read (w_io_t *iobase, void *buf, size_t len)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;
    size_t to_read;

    if (io->pos >= io->buf.len) {
        return 0;
    }

    to_read = w_min (len, io->buf.len - io->pos);
    memcpy (buf, io->buf.buf + io->pos, to_read);
    io->pos += to_read;
    return to_read;
}


w_io_t*
w_io_buf_open (w_buf_t *buf)
{
    w_io_buf_t *io = w_obj_new (w_io_buf_t);

    w_io_init ((w_io_t*) io);

    if (buf) {
        /*
         * XXX This makes the data memory area *shared* by the passed buffer
         *     and the I/O object. Should not be a problem but having a note
         *     here is nice in case problems arise. Quick solution would be
         *     to just *copy* the data in the passed buffer.
         */
        memcpy (&io->buf, buf, sizeof (w_buf_t));
        io->own = W_NO;
    }
    else {
        /*
         * FIXME This has knowledge of w_buf_t internals! It assumes that
         *       setting everything to zero does the right thing.
         */
        memset (&io->buf, 0x00, sizeof (w_buf_t));
        io->own = W_YES;
    }

    io->parent.close = w_io_buf_close;
    io->parent.write = w_io_buf_write;
    io->parent.read  = w_io_buf_read;
    io->pos = 0;

    return (w_io_t*) io;
}

