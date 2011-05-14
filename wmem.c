/*
 * wmem.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void*
w_malloc(size_t sz)
{
	void *p = malloc(sz);
	if (p == NULL)
		w_die("virtual memory exhausted (tried to allocate $L bytes)\n",
				(unsigned long) sz);

	memset(p, 0x00, sz);
	return p;
}


void*
w_realloc(void *ptr, size_t sz)
{
	if (sz) {
		if (ptr == NULL)
			return w_malloc(sz);
		else {
			ptr = realloc(ptr, sz);
			if (ptr == NULL)
				w_die("virtual memory exhausted (tried to allocate $L bytes)\n",
						(unsigned long) sz);
			return ptr;
		}
	}
	else {
		if (ptr == NULL)
			return NULL;
		else {
			free(ptr);
			return NULL;
		}
	}
}


