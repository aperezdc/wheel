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
    if (w_unlikely (p == NULL)) {
        w_die("virtual memory exhausted (tried to allocate $L bytes)\n",
              (unsigned long) sz);
    }

    // FIXME This should be removed
    memset(p, 0x00, sz);
    return p;
}


void*
w_realloc(void *ptr, size_t sz)
{
    if (w_likely (sz)) {
        if (w_unlikely (ptr == NULL)) {
            return w_malloc(sz);
        }
        else {
            ptr = realloc(ptr, sz);
            if (w_unlikely (ptr == NULL)) {
                w_die("virtual memory exhausted (tried to allocate $L bytes)\n",
                      (unsigned long) sz);
            }
            return ptr;
        }
    }
    else {
        if (w_unlikely (ptr == NULL)) {
            return NULL;
        }
        else {
            free(ptr);
            return NULL;
        }
    }
}


