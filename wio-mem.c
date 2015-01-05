/*
 * wio-mem.c
 * Copyright (C) 2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * .. _wio-mem:
 *
 * Input/Output on memory
 * ======================
 *
 * Provides support for using the :ref:`stream functions <wio-functions>` to
 * read and write to and from regions of memory of fixed sizes.
 *
 *
 * Types
 * -----
 */

/*~t w_io_mem_t
 *
 * Performs input/output on a region of memory of a fixed size.
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"
#include <errno.h>


static w_io_result_t
w_io_mem_close (w_io_t *iobase)
{
    w_io_mem_t *io = (w_io_mem_t*) iobase;

    io->data = NULL;
    io->size = 0;

    return W_IO_RESULT_SUCCESS;
}


static w_io_result_t
w_io_mem_write (w_io_t *iobase, const void *buf, size_t len)
{
    w_io_mem_t *io = (w_io_mem_t*) iobase;

    if (!(len = w_min (len, io->size - io->pos)))
        return W_IO_RESULT_ERROR (errno = ENOSPC);

    memcpy (io->data + io->pos, buf, len);
    io->pos += len;

    return W_IO_RESULT (len);
}


static w_io_result_t
w_io_mem_read (w_io_t *iobase, void *buf, size_t len)
{
    w_io_mem_t *io = (w_io_mem_t*) iobase;

    if (!(len = w_min (len, io->size - io->pos)))
        return W_IO_RESULT_EOF;

    memcpy (buf, io->data + io->pos, len);
    io->pos += len;

    return W_IO_RESULT (len);
}


/*~f void w_io_mem_init (w_io_mem_t *stream, uint8_t *address, size_t size)
 *
 * Initializes a `stream` object (possibly located in the stack) to be used
 * with a region of memory of a given `size` located at `address`.
 */
void
w_io_mem_init (w_io_mem_t *io, uint8_t *data, size_t size)
{
    w_assert (io);
    w_assert (data);

    w_io_init ((w_io_t*) io);

    io->parent.close = w_io_mem_close;
    io->parent.write = w_io_mem_write;
    io->parent.read  = w_io_mem_read;
    io->data         = data;
    io->size         = size;
    io->pos          = 0;
}


/*~f w_io_t* w_io_mem_open (uint8_t *address, size_t size)
 *
 * Creates a stream object to be used with a region of memory of a given
 * `size` located at `address`.
 */
w_io_t*
w_io_mem_open (uint8_t *data, size_t size)
{
    w_assert (data);

    w_io_mem_t *io = w_obj_new (w_io_mem_t);
    w_io_mem_init (io, data, size);
    return (w_io_t*) io;
}


/*~f uint8_t* w_io_mem_data (w_io_mem_t *stream)
 *
 * Obtains the base address to the memory region on which a `stream` operates.
 */

/*~f size_t w_io_mem_size (w_io_mem_t *stream)
 *
 * Obtains the size of the memory region on which a `stream` operates.
 */
