/*
 * wio-buf.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <string.h>


static wbool
w_io_buf_close (void *udata)
{
    w_buf_free (&(((struct w_io_buf_data*) udata)->buf));
    return W_YES;
}


static ssize_t
w_io_buf_write (void *udata, const void *buf, size_t len)
{
    struct w_io_buf_data *data = (struct w_io_buf_data*) udata;

    w_buf_length_set (&data->buf, data->pos);
    w_buf_append_mem (&data->buf, buf, len);
    data->pos += len;
    return len;
}


static ssize_t
w_io_buf_read (void *udata, void *buf, size_t len)
{
    size_t to_read;
    struct w_io_buf_data *data = (struct w_io_buf_data*) udata;

    if (data->pos >= data->buf.len) {
        return 0;
    }

    to_read = w_min (len, data->buf.len - data->pos);
    memcpy (buf, data->buf.buf + data->pos, to_read);
    data->pos += to_read;
    return to_read;
}


void
w_io_buf_open (w_io_t *io, w_buf_t *buf)
{
    struct w_io_buf_data *data;
    w_assert (io);

    data = W_IO_UDATA (io, struct w_io_buf_data);
    data->pos = 0;

    if (buf) {
        /*
         * XXX This makes the data memory area *shared* by the passed buffer
         *     and the I/O object. Should not be a problem but having a note
         *     here is nice in case problems arise. Quick solution would be
         *     to just *copy* the data in the passed buffer.
         */
        memcpy (&data->buf, buf, sizeof (w_buf_t));
    }
    else {
        /*
         * FIXME This has knowledge of w_buf_t internals! It assumes that
         *       setting everything to zero does the right thing.
         */
        memset (&data->buf, 0x00, sizeof (w_buf_t));
    }

    io->close = w_io_buf_close;
    io->write = w_io_buf_write;
    io->read  = w_io_buf_read;
}


