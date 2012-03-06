/*
 * wio-buf.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <string.h>


static w_bool_t
w_io_buf_close (w_io_t *iobase)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;

    if (io->bufp == &io->buf)
        w_buf_clear (&io->buf);

    return W_YES;
}


static ssize_t
w_io_buf_write (w_io_t *iobase, const void *buf, size_t len)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;

    w_buf_resize (io->bufp, io->pos);
    w_buf_append_mem (io->bufp, buf, len);
    io->pos += len;
    return len;
}


static ssize_t
w_io_buf_read (w_io_t *iobase, void *buf, size_t len)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;
    size_t to_read;

    if (io->pos >= w_buf_size (io->bufp)) {
        return W_IO_EOF;
    }

    to_read = w_min (len, w_buf_size (io->bufp) - io->pos);
    memcpy (buf, w_buf_data (io->bufp) + io->pos, to_read);
    io->pos += to_read;
    return to_read;
}


void
w_io_buf_init (w_io_buf_t *io, w_buf_t *buf, w_bool_t append)
{
    w_assert (io);

    w_io_init ((w_io_t*) io);

    /*
     * XXX This has knowledge of w_buf_t internals! It assumes that
     *     setting everything to zero does the right thing.
     */
    memset (&io->buf, 0x00, sizeof (w_buf_t));

    io->parent.close = w_io_buf_close;
    io->parent.write = w_io_buf_write;
    io->parent.read  = w_io_buf_read;
    io->bufp = buf ? buf : &io->buf;
    io->pos = append ? w_buf_size (io->bufp) : 0;
}


w_io_t*
w_io_buf_open (w_buf_t *buf)
{
    w_io_buf_t *io = w_obj_new (w_io_buf_t);
    w_io_buf_init (io, buf, W_NO);
    return (w_io_t*) io;
}

