/*
 * wobj.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"


void*
w_obj_ref (void *obj)
{
    w_assert (obj);
    ((w_obj_t*) obj)->__refs++;
    return obj;
}


void*
w_obj_unref (void *obj)
{
    w_assert (obj);

    if (--((w_obj_t*) obj)->__refs == 0) {
        w_obj_destroy (obj);
        return NULL;
    }
    return obj;
}


void
w_obj_destroy (void *obj)
{
    w_obj_t *o = (w_obj_t*) obj;

    w_assert (obj);

    if (o->__dtor) {
        (*o->__dtor) (obj);
    }
    w_free (o);
}


void*
w_obj_dtor (void *obj, void (*dtor) (void*))
{
    w_assert (obj);

    ((w_obj_t*) obj)->__dtor = dtor;
    return obj;
}
