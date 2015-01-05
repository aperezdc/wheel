/*
 * wio-buf.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * .. _wio-buf:
 *
 * Input/Output on Buffers
 * =======================
 *
 * Provides support for using the :ref:`stream functions <wio-functions>` to
 * read and write to and from :type:`w_buf_t` buffers.
 *
 * Types
 * -----
 */

/*~t w_io_buf_t
 *
 * Performs input/output on a :type:`w_buf_t` buffer.
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"
#include <string.h>


static w_io_result_t
w_io_buf_close (w_io_t *iobase)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;

    if (io->bufp == &io->buf)
        w_buf_clear (&io->buf);

    return W_IO_RESULT_SUCCESS;
}


static w_io_result_t
w_io_buf_write (w_io_t *iobase, const void *buf, size_t len)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;

    w_buf_resize (io->bufp, io->pos);
    w_buf_append_mem (io->bufp, buf, len);
    io->pos += len;

    return W_IO_RESULT (len);
}


static w_io_result_t
w_io_buf_read (w_io_t *iobase, void *buf, size_t len)
{
    w_io_buf_t *io = (w_io_buf_t*) iobase;

    if (io->pos >= w_buf_size (io->bufp))
        return W_IO_RESULT_EOF;

    size_t to_read = w_min (len, w_buf_size (io->bufp) - io->pos);
    memcpy (buf, w_buf_data (io->bufp) + io->pos, to_read);
    io->pos += to_read;

    return W_IO_RESULT (to_read);
}


/*~f void w_io_buf_init (w_io_buf_t *stream, w_buf_t *buffer, bool append)
 *
 * Initialize a `stream` object (possibly allocated in the stack) to be
 * used with a `buffer`.
 *
 * Passing a ``NULL`` `buffer` will create a new buffer owned by the stream
 * object, which can be retrieved using :func:`w_io_buf_get_buffer()`. The
 * memory used by this buffer will be freed automatically when the stream
 * object is freed. On the contrary, when a valid buffer is supplied, the
 * caller is responsible for calling :func:`w_buf_clear()` on it.
 *
 * Optionally, the stream position can be setup to `append` data to the
 * contents already present in the given `buffer`, insted of overwriting
 * them.
 */
void
w_io_buf_init (w_io_buf_t *io, w_buf_t *buf, bool append)
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


/*~f w_io_t* w_io_buf_open (w_buf_t *buffer)
 *
 * Creates a stream object to be used with a `buffer`.
 *
 * Passing a ``NULL`` `buffer` will create a new buffer owned by the stream
 * object, which can be retrieved using :func:`w_io_buf_get_buffer()`. The
 * memory used by this buffer will be freed automatically when the stream
 * object is freed. On the contrary, when a valid buffer is supplied, the
 * caller is responsible for calling :func:`w_buf_clear()` on it.
 */
w_io_t*
w_io_buf_open (w_buf_t *buf)
{
    w_io_buf_t *io = w_obj_new (w_io_buf_t);
    w_io_buf_init (io, buf, false);
    return (w_io_t*) io;
}

/*~f w_buf_t* w_io_buf_get_buffer (w_io_buf_t *stream)
 *
 * Obtain a pointer to the buffer being used by a `stream`.
 */

/*~f char* w_io_buf_str (w_io_buf_t *stream)
 *
 * Obtain a string representation of the contents of the buffer being used by
 * a `stream`.
 */
