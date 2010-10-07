/*
 * werr.c
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>
#include <stdlib.h>
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
	vfprintf(stderr, fmt, al);
	fflush(stderr);
	exit(EXIT_FAILURE);
}


void
__w_dprintf(const char *fmt, ...)
{
	va_list al;

	va_start(al, fmt);
	fprintf(stderr, "DEBUG: ");
	vfprintf(stderr, fmt, al);
	fputc('\n', stderr);
	va_end(al);
}

