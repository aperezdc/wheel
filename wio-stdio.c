/*
 * wio-stdio.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>


static wbool
w_io_stdio_close (w_io_t *io)
{
    return (fclose (((w_io_stdio_t*) io)->fp) == 0);
}


static ssize_t
w_io_stdio_write (w_io_t *io, const void *buf, size_t len)
{
    size_t ret;

    if ((ret = fwrite (buf, sizeof (char), len, ((w_io_stdio_t*) io)->fp)) < len) {
        if (ferror (((w_io_stdio_t*) io)->fp)) {
            return -1;
        }
        else {
            return ret;
        }
    }
    w_assert (ret == len);
    return ret;
}


static ssize_t
w_io_stdio_read (w_io_t *io, void *buf, size_t len)
{
    size_t ret;

    if ((ret = fread (buf, sizeof (char), len, ((w_io_stdio_t*) io)->fp)) < len) {
        if (ferror (((w_io_stdio_t*) io)->fp)) {
            return -1;
        }
        else {
            w_assert (feof (((w_io_stdio_t*) io)->fp));
            return ret;
        }
    }
    w_assert (ret == len);
    return ret;
}


static wbool
w_io_stdio_flush (w_io_t *io)
{
    return fflush (((w_io_stdio_t*) io)->fp) != 0;
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
    io->fp = fp;
}


w_io_t*
w_io_stdio_open (FILE *fp)
{
    w_io_stdio_t *io = w_obj_new (w_io_stdio_t);
    w_io_stdio_init (io, fp);
    return (w_io_t*) io;
}
