/*
 * wtnetstr.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"

enum {
    /* Five digits maximum may be used for specifying value lengths. */
    _W_TNS_MAX_PAYLOAD = 99999,
    _W_TNS_SIZE_DIGITS = 5,
};

/* Some values are always encoded as those constant strings. */
static const char _w_tns_false[] = "5:false!";
static const char _w_tns_true [] = "4:true!";
static const char _w_tns_null [] = "0:~";


wbool
w_tnetstr_dump_null (w_buf_t *buffer)
{
    w_assert (buffer);
    w_buf_append_mem (buffer, _w_tns_null, w_lengthof (_w_tns_null) - 1);
    return W_NO;
}


wbool
w_tnetstr_dump_boolean (w_buf_t *buffer, wbool value)
{
    w_assert (buffer);
    if (value)
        w_buf_append_mem (buffer, _w_tns_true, w_lengthof (_w_tns_true) - 1);
    else
        w_buf_append_mem (buffer, _w_tns_false, w_lengthof (_w_tns_false) - 1);
    return W_NO;
}


wbool
w_tnetstr_dump_string (w_buf_t *buffer, const char *value)
{
    size_t len;

    w_assert (buffer);
    w_assert (value);

    len = strlen (value);
    if (w_unlikely (len > _W_TNS_MAX_PAYLOAD))
        return W_YES;

    w_buf_format (buffer, "$L:$S,", len, len, value);
    return W_NO;
}


wbool
w_tnetstr_dump_buffer (w_buf_t *buffer, const w_buf_t *value)
{
    w_assert (buffer);
    w_assert (value);

    if (w_unlikely (w_buf_length (value) > _W_TNS_MAX_PAYLOAD))
        return W_YES;

    w_buf_format (buffer, "$L:$B,", w_buf_length (value), value);
    return W_NO;
}


wbool
w_tnetstr_dump_number (w_buf_t *buffer, long value)
{
    w_buf_t buf = W_BUF;
    w_io_buf_t iobuf;

    w_assert (buffer);

    w_io_buf_init (&iobuf, &buf, W_NO);
    w_io_format_long ((w_io_t*) &iobuf, value);

    if (w_unlikely (w_buf_length (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B#", w_buf_length (&buf), &buf);
    w_buf_free (&buf);
    return W_NO;

return_error:
    w_buf_free (&buf);
    return W_YES;
}


wbool
w_tnetstr_dump_float (w_buf_t *buffer, double value)
{
    w_buf_t buf = W_BUF;
    w_io_buf_t iobuf;

    w_assert (buffer);

    w_io_buf_init (&iobuf, &buf, W_NO);
    w_io_format_double ((w_io_t*) &iobuf, value);

    if (w_unlikely (w_buf_length (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B^", w_buf_length (&buf), &buf);
    w_buf_free (&buf);
    return W_NO;

return_error:
    w_buf_free (&buf);
    return W_YES;
}


wbool
w_tnetstr_dump_list (w_buf_t *buffer, const w_list_t *value)
{
    w_buf_t buf = W_BUF;
    w_iterator_t item;

    w_assert (buffer);
    w_assert (value);

    w_list_foreach (value, item)
        if (w_unlikely (w_tnetstr_dump (&buf, (w_variant_t*) *item)))
            goto return_error;

    if (w_unlikely (w_buf_length (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B]", w_buf_length (&buf), &buf);
    w_buf_free (&buf);
    return W_NO;

return_error:
    w_buf_free (&buf);
    return W_YES;
}


wbool
w_tnetstr_dump_dict (w_buf_t *buffer, const w_dict_t *value)
{
    w_buf_t buf = W_BUF;
    w_iterator_t item;

    w_assert (buffer);
    w_assert (value);

    w_dict_foreach (value, item)
        if (w_unlikely (w_tnetstr_dump_string (&buf, w_dict_iterator_get_key (item)) ||
                        w_tnetstr_dump (&buf, (const w_variant_t*) *item)))
            goto return_error;

    if (w_unlikely (w_buf_length (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B}", w_buf_length (&buf), &buf);
    w_buf_free (&buf);
    return W_NO;

return_error:
    w_buf_free (&buf);
    return W_YES;
}


wbool
w_tnetstr_dump (w_buf_t *buffer, const w_variant_t *value)
{
    w_assert (buffer);
    w_assert (value);

    /* This should *never* happen. */
    w_assert (w_variant_type (value) != W_VARIANT_BUFFER);

    switch (w_variant_type (value)) {
        case W_VARIANT_BUFFER:
            w_io_format (w_stderr,
                         "w_variant_t with type = W_VARIANT_BUFFER detected.\n"
                         "This is a bug, execution aborted!\n");
            w_io_flush (w_stderr);
            abort ();

        case W_VARIANT_INVALID: /* Can't serialize invalid values. */
            return W_YES;
        case W_VARIANT_NULL:
            return w_tnetstr_dump_null (buffer);
        case W_VARIANT_BOOL:
            return w_tnetstr_dump_boolean (buffer, w_variant_bool (value));
        case W_VARIANT_STRING:
            return w_tnetstr_dump_buffer (buffer, w_variant_string_buf (value));
        case W_VARIANT_NUMBER:
            return w_tnetstr_dump_number (buffer, w_variant_number (value));
        case W_VARIANT_FLOAT:
            return w_tnetstr_dump_float (buffer, w_variant_float (value));
        case W_VARIANT_LIST:
            return w_tnetstr_dump_list (buffer, w_variant_list (value));
        case W_VARIANT_DICT:
            return w_tnetstr_dump_dict (buffer, w_variant_dict (value));
    }

    /* This point should never be reached. */
    return W_YES;
}

