/*
 * wio-stdio.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/*
 * In this particular compilation unit we always want to incllude the
 * declarations for the C library stdio-related functions.
 */
#ifndef W_CONF_STDIO
#define W_CONF_STDIO 1
#endif /* W_CONF_STDIO */

#include "wheel.h"
#include <stdio.h>
#include <errno.h>


static w_io_result_t
w_io_stdio_close (w_io_t *io)
{
    if (fclose (((w_io_stdio_t*) io)->fp) == EOF)
        return W_IO_RESULT_ERROR (errno);

    return W_IO_RESULT_SUCCESS;
}


static w_io_result_t
w_io_stdio_write (w_io_t *io, const void *buf, size_t len)
{
    size_t ret;

    if ((ret = fwrite (buf, sizeof (char), len, ((w_io_stdio_t*) io)->fp)) < len) {
        if (ferror (((w_io_stdio_t*) io)->fp))
            return W_IO_RESULT_ERROR (errno > 0 ? errno : EIO);
    }

    return W_IO_RESULT (ret);
}


static w_io_result_t
w_io_stdio_read (w_io_t *io, void *buf, size_t len)
{
    size_t ret;

    if ((ret = fread (buf, sizeof (char), len, ((w_io_stdio_t*) io)->fp)) < len) {
        if (ret == 0 && feof (((w_io_stdio_t*) io)->fp))
            return W_IO_RESULT_EOF;
        if (ferror (((w_io_stdio_t*) io)->fp))
            return W_IO_RESULT_ERROR (errno > 0 ? errno : EIO);
    }

    return W_IO_RESULT (ret);
}


static w_io_result_t
w_io_stdio_flush (w_io_t *io)
{
    if (fflush (((w_io_stdio_t*) io)->fp) == EOF)
        return W_IO_RESULT_ERROR (errno);

    return W_IO_RESULT_SUCCESS;
}


static int
w_io_stdio_getfd (w_io_t *io)
{
    return fileno (((w_io_stdio_t*) io)->fp);
}


void
w_io_stdio_init (w_io_stdio_t *io, FILE *fp)
{
    w_assert (io);
    w_assert (fp);

    w_io_init ((w_io_t*) io);

    io->parent.close = w_io_stdio_close;
    io->parent.write = w_io_stdio_write;
    io->parent.read  = w_io_stdio_read;
    io->parent.flush = w_io_stdio_flush;
    io->parent.getfd = w_io_stdio_getfd;
    io->fp = fp;
}


w_io_t*
w_io_stdio_open (FILE *fp)
{
    w_io_stdio_t *io = w_obj_new (w_io_stdio_t);
    w_io_stdio_init (io, fp);
    return (w_io_t*) io;
}
