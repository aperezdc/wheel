/*
 * werr.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


void
w_die(const char *fmt, ...)
{
	va_list al;

	va_start(al, fmt);
	w_diev(fmt, al);
	va_end(al);
}


void
w_diev(const char *fmt, va_list al)
{
    if (fmt) {
        /* Unfortunately, errors cannot be reported from here. */
        W_IO_NORESULT (w_io_formatv (w_stderr, fmt, al));
        fsync (w_io_unix_get_fd ((w_io_unix_t*) w_stderr));
    }
    exit(EXIT_FAILURE);
}


void
__w_debug(const char *fmt, ...)
{
    va_list al;

    va_start(al, fmt);
    /* Unfortunately, errors cannot be reported from here. */
    W_IO_NORESULT (w_io_format  (w_stderr, "DEBUG: "));
    W_IO_NORESULT (w_io_formatv (w_stderr, fmt, al));
    W_IO_NORESULT (w_io_putchar (w_stderr, '\n'));
    va_end(al);
}

