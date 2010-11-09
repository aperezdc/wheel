/*
 * wio-stdio.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>


wbool
w_io_stdio_close (void *udata)
{
    return (fclose ((FILE*) udata) == 0);
}


ssize_t
w_io_stdio_write (void *udata, const void *buf, size_t len)
{
    size_t ret;

    if ((ret = fwrite (buf, sizeof (char), len, (FILE*) udata)) < len) {
        if (ferror ((FILE*) udata)) {
            return -1;
        }
        else {
            return ret;
        }
    }
    w_assert (ret == len);
    return ret;
}


ssize_t
w_io_stdio_read (void *udata, void *buf, size_t len)
{
    size_t ret;

    if ((ret = fread (buf, sizeof (char), len, (FILE*) udata)) < len) {
        if (ferror ((FILE*) udata)) {
            return -1;
        }
        else {
            w_assert (feof ((FILE*) udata));
            return ret;
        }
    }
    w_assert (ret == len);
    return ret;
}

