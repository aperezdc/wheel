/*
 * wvariant.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdarg.h>

static inline void
_w_variant_clear (w_variant_t *v)
{
    switch (v->type) {
        case W_VARIANT_TYPE_INVALID:
        case W_VARIANT_TYPE_NUMBER:
        case W_VARIANT_TYPE_FLOAT:
        case W_VARIANT_TYPE_BOOL:
        case W_VARIANT_TYPE_NULL:
            /* Do nothing */
            break;

        case W_VARIANT_TYPE_BUFFER:
        case W_VARIANT_TYPE_STRING:
            w_buf_clear (&v->value.stringbuf);
            break;

        case W_VARIANT_TYPE_LIST:
            w_obj_unref (v->value.list);
            break;

        case W_VARIANT_TYPE_DICT:
            w_obj_unref (v->value.dict);
            break;

        case W_VARIANT_TYPE_OBJECT:
            w_obj_unref (v->value.obj);
            break;
    }
}


static void
_w_variant_dtor (void *obj)
{
    _w_variant_clear ((w_variant_t*) obj);
}


w_variant_t*
w_variant_new (w_variant_type_t type, ...)
{
    va_list args;
    w_variant_t *variant = w_obj_new (w_variant_t);
    memset (&variant->value, 0x00, sizeof (w_variant_value_t));

    va_start (args, type);
    switch ((variant->type = type)) {
        case W_VARIANT_TYPE_INVALID:
        case W_VARIANT_TYPE_NULL:
            /* Nothing to do */
            break;

        case W_VARIANT_TYPE_NUMBER:
            variant->value.number = va_arg (args, long);
            break;

        case W_VARIANT_TYPE_FLOAT:
            variant->value.fpnumber = va_arg (args, double);
            break;

        case W_VARIANT_TYPE_BOOL:
            variant->value.boolean = (bool) va_arg (args, int);
            break;

        case W_VARIANT_TYPE_STRING:
            w_buf_set_str (&variant->value.stringbuf, va_arg (args, const char*));
            break;

        case W_VARIANT_TYPE_BUFFER:
            w_buf_append_buf (&variant->value.stringbuf, va_arg (args, const w_buf_t*));
            variant->type = W_VARIANT_TYPE_STRING;
            break;

        case W_VARIANT_TYPE_DICT:
            variant->value.dict = w_obj_ref (va_arg (args, w_dict_t*));
            break;

        case W_VARIANT_TYPE_LIST:
            variant->value.list = w_obj_ref (va_arg (args, w_list_t*));
            break;

        case W_VARIANT_TYPE_OBJECT:
            variant->value.obj = w_obj_ref (va_arg (args, w_obj_t*));
            break;
    }
    va_end (args);

    return w_obj_dtor (variant, _w_variant_dtor);
}


w_variant_t*
w_variant_clear (w_variant_t *variant)
{
    w_assert (variant);
    _w_variant_clear (variant);
    variant->type = W_VARIANT_TYPE_INVALID;
    return variant;
}
