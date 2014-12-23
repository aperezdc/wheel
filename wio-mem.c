/*
 * wio-mem.c
 * Copyright (C) 2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
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


w_io_t*
w_io_mem_open (uint8_t *data, size_t size)
{
    w_assert (data);

    w_io_mem_t *io = w_obj_new (w_io_mem_t);
    w_io_mem_init (io, data, size);
    return (w_io_t*) io;
}
