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
        case W_VARIANT_INVALID:
        case W_VARIANT_NUMBER:
        case W_VARIANT_FLOAT:
        case W_VARIANT_BOOL:
        case W_VARIANT_NULL:
            /* Do nothing */
            break;

        case W_VARIANT_BUFFER:
        case W_VARIANT_STRING:
            w_buf_clear (&v->value.stringbuf);
            break;

        case W_VARIANT_LIST:
            w_obj_unref (v->value.list);
            break;

        case W_VARIANT_DICT:
            w_obj_unref (v->value.dict);
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
        case W_VARIANT_INVALID:
        case W_VARIANT_NULL:
            /* Nothing to do */
            break;

        case W_VARIANT_NUMBER:
            variant->value.number = va_arg (args, long);
            break;

        case W_VARIANT_FLOAT:
            variant->value.fpnumber = va_arg (args, double);
            break;

        case W_VARIANT_BOOL:
            variant->value.boolean = va_arg (args, wbool);
            break;

        case W_VARIANT_STRING:
            w_buf_set_str (&variant->value.stringbuf, va_arg (args, const char*));
            break;

        case W_VARIANT_BUFFER:
            w_buf_append_buf (&variant->value.stringbuf, va_arg (args, const w_buf_t*));
            variant->type = W_VARIANT_STRING;
            break;

        case W_VARIANT_DICT:
            variant->value.dict = w_obj_ref (va_arg (args, w_dict_t*));
            break;

        case W_VARIANT_LIST:
            variant->value.list = w_obj_ref (va_arg (args, w_list_t*));
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
    variant->type = W_VARIANT_INVALID;
    return variant;
}
