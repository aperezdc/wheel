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
        W_FATAL ("virtual memory exhausted (tried to allocate $L bytes)\n",
                 (unsigned long) sz);
    }

    /* FIXME This should be removed */
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
                W_FATAL ("virtual memory exhausted (tried to allocate $L bytes)\n",
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


void
_w_lmem_cleanup (void *ptr)
{
    void **location = ptr;
    if (location) {
        /* w_free already sets the address to NULL */
        if (*location) {
            w_free (*location);
        }
    }
}


void
_w_lobj_cleanup (void *ptr)
{
    void **location = ptr;
    if (location) {
        w_obj_t *obj = *location;
        if (obj) {
            w_obj_unref (obj);
            *location = NULL;
        }
    }
}

