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
	w_vdie(fmt, al);
	va_end(al);
}


void
w_vdie(const char *fmt, va_list al)
{
    w_io_unix_t err;
    w_io_unix_init (&err, STDERR_FILENO);

    w_io_formatv ((w_io_t*) &err, fmt, al);
    fsync (STDERR_FILENO);

    exit(EXIT_FAILURE);
}


void
__w_dprintf(const char *fmt, ...)
{
    va_list al;
    w_io_unix_t err;

    w_io_unix_init (&err, STDERR_FILENO);

    va_start(al, fmt);
    w_io_format  ((w_io_t*) &err, "DEBUG: ");
    w_io_formatv ((w_io_t*) &err, fmt, al);
    w_io_putchar ((w_io_t*) &err, '\n');
    va_end(al);
}

