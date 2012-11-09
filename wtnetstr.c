/*
 * wtnetstr.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"

enum {
    /* Five digits maximum may be used for specifying value lengths. */
    _W_TNS_MIN_LENGTH  = 3, /* 1 ch for length + colon + 1 ch for type tag */
    _W_TNS_MAX_PAYLOAD = 99999,
    _W_TNS_SIZE_DIGITS = 5,
    _W_TNS_TAG_NULL    = '~',
    _W_TNS_TAG_BOOLEAN = '!',
    _W_TNS_TAG_STRING  = ',',
    _W_TNS_TAG_NUMBER  = '#',
    _W_TNS_TAG_FLOAT   = '^',
    _W_TNS_TAG_LIST    = ']',
    _W_TNS_TAG_DICT    = '}',
};

/* Some values are always encoded as those constant strings. */
static const char _w_tns_false[] = "5:false!";
static const char _w_tns_true [] = "4:true!";
static const char _w_tns_null [] = "0:~";


w_bool_t
w_tnetstr_dump_null (w_buf_t *buffer)
{
    w_assert (buffer);
    w_buf_append_mem (buffer, _w_tns_null, w_lengthof (_w_tns_null) - 1);
    return W_NO;
}


w_bool_t
w_tnetstr_write_null (w_io_t *io)
{
    static const unsigned len = w_lengthof (_w_tns_null) - 1;
    w_assert (io);
    return w_io_write (io, _w_tns_null, len) != len;
}


w_bool_t
w_tnetstr_dump_boolean (w_buf_t *buffer, w_bool_t value)
{
    w_assert (buffer);
    if (value)
        w_buf_append_mem (buffer, _w_tns_true, w_lengthof (_w_tns_true) - 1);
    else
        w_buf_append_mem (buffer, _w_tns_false, w_lengthof (_w_tns_false) - 1);
    return W_NO;
}


w_bool_t
w_tnetstr_write_boolean (w_io_t *io, w_bool_t value)
{
    w_assert (io);
    if (value) {
        static const unsigned len = w_lengthof (_w_tns_true) - 1;
        return w_io_write (io, _w_tns_true, len) != len;
    }
    else {
        static const unsigned len = w_lengthof (_w_tns_false) - 1;
        return w_io_write (io, _w_tns_false, len) != len;
    }
}


w_bool_t
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


w_bool_t
w_tnetstr_write_string (w_io_t *io, const char *value)
{
    size_t len;

    w_assert (io);
    w_assert (value);

    len = strlen (value);
    if (w_unlikely ((len) > _W_TNS_MAX_PAYLOAD))
        return W_YES;

    return w_io_format (io, "$L:$S,", len, len, value) == W_IO_ERR;
}


w_bool_t
w_tnetstr_dump_buffer (w_buf_t *buffer, const w_buf_t *value)
{
    w_assert (buffer);
    w_assert (value);

    if (w_unlikely (w_buf_size (value) > _W_TNS_MAX_PAYLOAD))
        return W_YES;

    w_buf_format (buffer, "$L:$B,", w_buf_size (value), value);
    return W_NO;
}


w_bool_t
w_tnetstr_write_buffer (w_io_t *io, const w_buf_t *value)
{
    w_assert (io);
    w_assert (value);

    if (w_unlikely (w_buf_size (value) > _W_TNS_MAX_PAYLOAD))
        return W_YES;

    return w_io_format (io, "$L:$B,", w_buf_size (value), value) == W_IO_ERR;
}


w_bool_t
w_tnetstr_dump_number (w_buf_t *buffer, long value)
{
    w_buf_t buf = W_BUF;
    w_io_buf_t iobuf;

    w_assert (buffer);

    w_io_buf_init (&iobuf, &buf, W_NO);
    w_io_format_long ((w_io_t*) &iobuf, value);

    if (w_unlikely (w_buf_size (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B#", w_buf_size (&buf), &buf);
    w_buf_clear (&buf);
    return W_NO;

return_error:
    w_buf_clear (&buf);
    return W_YES;
}


w_bool_t
w_tnetstr_dump_float (w_buf_t *buffer, double value)
{
    w_buf_t buf = W_BUF;
    w_io_buf_t iobuf;

    w_assert (buffer);

    w_io_buf_init (&iobuf, &buf, W_NO);
    w_io_format_double ((w_io_t*) &iobuf, value);

    if (w_unlikely (w_buf_size (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B^", w_buf_size (&buf), &buf);
    w_buf_clear (&buf);
    return W_NO;

return_error:
    w_buf_clear (&buf);
    return W_YES;
}


w_bool_t
w_tnetstr_dump_list (w_buf_t *buffer, const w_list_t *value)
{
    w_buf_t buf = W_BUF;
    w_iterator_t item;

    w_assert (buffer);
    w_assert (value);

    w_list_foreach (value, item)
        if (w_unlikely (w_tnetstr_dump (&buf, (w_variant_t*) *item)))
            goto return_error;

    if (w_unlikely (w_buf_size (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B]", w_buf_size (&buf), &buf);
    w_buf_clear (&buf);
    return W_NO;

return_error:
    w_buf_clear (&buf);
    return W_YES;
}


w_bool_t
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

    if (w_unlikely (w_buf_size (&buf) > _W_TNS_MAX_PAYLOAD))
        goto return_error;

    w_buf_format (buffer, "$L:$B}", w_buf_size (&buf), &buf);
    w_buf_clear (&buf);
    return W_NO;

return_error:
    w_buf_clear (&buf);
    return W_YES;
}


w_bool_t
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

        case W_VARIANT_OBJECT:  /* Can't serialize object values. */
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


w_bool_t
w_tnetstr_write (w_io_t *io, const w_variant_t *value)
{
    w_assert (io);
    w_assert (value);

    /* This should *never* happen. */
    w_assert (w_variant_type (value) != W_VARIANT_BUFFER);

    switch (w_variant_type (value)) {
        case W_VARIANT_BUFFER:
            w_io_format (w_stderr,
                         "w_variant_t with type = W_VARIANT_BUFFER detected.\n"
                         "This is a bug, execution aborted!\n");
            w_io_flush (w_stderr);
            abort();

        case W_VARIANT_OBJECT:  /* Can't serialize object values. */
        case W_VARIANT_INVALID: /* Can't serialize invalid values. */
            return W_YES;
        case W_VARIANT_NULL:
            return w_tnetstr_write_null (io);
        case W_VARIANT_BOOL:
            return w_tnetstr_write_boolean (io, w_variant_bool (value));
        case W_VARIANT_STRING:
            return w_tnetstr_write_buffer (io, w_variant_string_buf (value));
        case W_VARIANT_NUMBER:
            return w_tnetstr_write_number (io, w_variant_number (value));
        case W_VARIANT_FLOAT:
            return w_tnetstr_write_float (io, w_variant_float (value));
        case W_VARIANT_LIST:
            return w_tnetstr_write_list (io, w_variant_list (value));
        case W_VARIANT_DICT:
            return w_tnetstr_write_dict (io, w_variant_dict (value));
    }

    /* This point should never be reached. */
    return W_YES;
}


static inline size_t
peek_item_size (const w_buf_t *buffer)
{
    unsigned plen = 0;
    unsigned blen;
    char *pbuf;

    w_assert (buffer);

    for (pbuf = w_buf_data (buffer), blen = w_buf_size (buffer);
         blen-- && *pbuf != ':';
         pbuf++, plen *= 10)
        plen += *pbuf - '0';

    plen /= 10;

    return (pbuf - w_buf_data (buffer)) + plen + 2;
}


static inline w_bool_t
slice_payload (const w_buf_t *buffer, w_buf_t *slice, int type_tag)
{
    unsigned plen = 0;
    unsigned blen;
    char *pbuf;

    w_assert (buffer);

    if (w_buf_size (buffer) < _W_TNS_MIN_LENGTH)
        return W_YES;

    for (pbuf = w_buf_data (buffer), blen = w_buf_size (buffer);
         blen-- && *pbuf != ':';
         pbuf++, plen *= 10)
        plen += *pbuf - '0';

    plen /= 10;

    if (plen > _W_TNS_MAX_PAYLOAD ||
        w_buf_size (buffer) < (size_t) ((pbuf - w_buf_data (buffer)) + plen + 2) ||
        *(pbuf + plen + 1) != type_tag)
        return W_YES;

    /*
     * XXX We cheat here: we make the "slice" buffer point to the same
     *     memory area of the input buffer, to avoid copying data.
     */
    w_buf_alloc_size (slice) = 0;
    w_buf_data (slice) = ++pbuf;
    w_buf_size (slice) = plen;

    return W_NO;
}


w_bool_t
w_tnetstr_read_to_buffer (w_io_t *io, w_buf_t *buffer)
{
    unsigned blen = _W_TNS_SIZE_DIGITS + 1; /* number + colon */
    unsigned plen;
    int ch = '\0';

    w_assert (io);
    w_assert (buffer);

    for (plen = 0; blen-- && (ch = w_io_getchar (io)) != ':'; plen *= 10) {
        w_buf_append_char (buffer, ch);
        plen += ch - '0';
    }
    plen /= 10;

    if (w_unlikely (plen > _W_TNS_MAX_PAYLOAD || ch != ':'))
        goto return_error;

    w_buf_append_char (buffer, ':');

    /*
     * Extend buffer to fit the payload data plus the terminating character.
     */
    blen = w_buf_size (buffer);
    w_buf_resize (buffer, ++plen + blen);

    return w_io_read (io, w_buf_data (buffer) + blen, plen) != plen;

return_error:
    w_buf_clear (buffer);
    return W_YES;
}


w_bool_t
w_tnetstr_parse_null (const w_buf_t *buffer)
{
    w_assert (buffer);
    return w_buf_size (buffer) < 3
        || w_buf_data (buffer)[0] != '0'
        || w_buf_data (buffer)[1] != ':'
        || w_buf_data (buffer)[2] != _W_TNS_TAG_NULL;
}


w_bool_t
w_tnetstr_parse_float (const w_buf_t *buffer, double *value)
{
    w_buf_t payload;
    w_io_buf_t iobuf;

    w_assert (buffer);
    w_assert (value);

    /* Shortest length value: 1:0^ */
    if (w_buf_size (buffer) < 4 ||
        slice_payload (buffer, &payload, _W_TNS_TAG_FLOAT))
        return W_YES;

    w_io_buf_init (&iobuf, &payload, W_NO);
    return w_io_fscan_double ((w_io_t*) &iobuf, value);
}


w_bool_t
w_tnetstr_parse_number (const w_buf_t *buffer, long *value)
{
    w_buf_t payload;
    w_io_buf_t iobuf;

    w_assert (buffer);
    w_assert (value);

    /* Shortest length value: 1:0# */
    if (w_buf_size (buffer) < 4 ||
        slice_payload (buffer, &payload, _W_TNS_TAG_NUMBER))
        return W_YES;

    w_io_buf_init (&iobuf, &payload, W_NO);
    return w_io_fscan_long ((w_io_t*) &iobuf, value);
}


w_bool_t
w_tnetstr_parse_boolean (const w_buf_t *buffer, w_bool_t *value)
{
    w_assert (buffer);
    w_assert (value);

    /* possible values:
     *   4:true!
     *   5:false!
     */
    switch (w_buf_size (buffer)) {
        case 7:
            *value = W_YES;
            return w_buf_data (buffer)[0] != '4'
                || w_buf_data (buffer)[1] != ':'
                || w_buf_data (buffer)[2] != 't'
                || w_buf_data (buffer)[3] != 'r'
                || w_buf_data (buffer)[4] != 'u'
                || w_buf_data (buffer)[5] != 'e'
                || w_buf_data (buffer)[6] != _W_TNS_TAG_BOOLEAN;
        case 8:
            *value = W_NO;
            return w_buf_data (buffer)[0] != '5'
                || w_buf_data (buffer)[1] != ':'
                || w_buf_data (buffer)[2] != 'f'
                || w_buf_data (buffer)[3] != 'a'
                || w_buf_data (buffer)[4] != 'l'
                || w_buf_data (buffer)[5] != 's'
                || w_buf_data (buffer)[6] != 'e'
                || w_buf_data (buffer)[7] != _W_TNS_TAG_BOOLEAN;
        default:
            return W_YES;
    }
}


w_bool_t
w_tnetstr_parse_string (const w_buf_t *buffer, w_buf_t *value)
{
    w_buf_t payload;

    w_assert (buffer);
    w_assert (value);

    if (slice_payload (buffer, &payload, _W_TNS_TAG_STRING))
        return W_YES;

    w_buf_append_buf (value, &payload);
    return W_NO;
}


w_bool_t
w_tnetstr_parse_list (const w_buf_t *buffer, w_list_t *value)
{
    w_buf_t payload;
    size_t szdone = 0;

    w_assert (buffer);
    w_assert (value);

    if (slice_payload (buffer, &payload, _W_TNS_TAG_LIST))
        return W_YES;

    while (szdone < w_buf_size (&payload)) {
        w_buf_t curitem = W_BUF;
        w_variant_t *variant;

        w_buf_data (&curitem) = w_buf_data (&payload) + szdone;
        w_buf_size (&curitem) = w_buf_size (&payload) - szdone;

        if (!(variant = w_tnetstr_parse (&curitem)))
            return W_YES;

        szdone += peek_item_size (&curitem);

        w_list_append (value, variant);
        w_obj_unref (variant);
    }

    return W_NO;
}


w_bool_t
w_tnetstr_parse_dict (const w_buf_t *buffer, w_dict_t *value)
{
    w_buf_t payload;
    size_t szdone = 0;

    w_assert (buffer);
    w_assert (value);

    if (slice_payload (buffer, &payload, _W_TNS_TAG_DICT))
        return W_YES;

    while (szdone < w_buf_size (&payload)) {
        w_buf_t curitem = W_BUF;
        w_buf_t key = W_BUF;
        w_variant_t *variant;

        w_buf_data (&curitem) = w_buf_data (&payload) + szdone;
        w_buf_size (&curitem) = w_buf_size (&payload) - szdone;

        if (w_tnetstr_parse_string (&curitem, &key))
            return W_YES;

        szdone += peek_item_size (&curitem);

        w_buf_data (&curitem) = w_buf_data (&payload) + szdone;
        w_buf_size (&curitem) = w_buf_size (&payload) - szdone;

        if (!(variant = w_tnetstr_parse (&curitem))) {
            w_buf_clear (&key);
            return W_YES;
        }

        szdone += peek_item_size (&curitem);

        w_dict_setn (value, w_buf_data (&key), w_buf_size (&key), variant);
        w_obj_unref (variant);
        w_buf_clear (&key);
    }

    return W_NO;
}


w_variant_t*
w_tnetstr_parse (const w_buf_t *buffer)
{
    w_variant_t *ret = NULL;
    size_t item_len;
    union {
        w_buf_t   vbuf;
        w_bool_t  vbool;
        double    vdouble;
        long      vlong;
        w_list_t *vlist;
        w_dict_t *vdict;
    } v = { W_BUF };

    w_assert (buffer);

    item_len = peek_item_size (buffer);

    if (item_len < _W_TNS_MIN_LENGTH)
        return NULL;

    switch (w_buf_data (buffer)[item_len-1]) {
        case _W_TNS_TAG_NULL:
            if (!w_tnetstr_parse_null (buffer))
                ret = w_variant_new (W_VARIANT_NULL);
            break;

        case _W_TNS_TAG_FLOAT:
            if (!w_tnetstr_parse_float (buffer, &v.vdouble))
                ret = w_variant_new (W_VARIANT_FLOAT, v.vdouble);
            break;

        case _W_TNS_TAG_NUMBER:
            if (!w_tnetstr_parse_number (buffer, &v.vlong))
                ret = w_variant_new (W_VARIANT_NUMBER, v.vlong);
            break;

        case _W_TNS_TAG_STRING:
            if (!w_tnetstr_parse_string (buffer, &v.vbuf))
                ret = w_variant_new (W_VARIANT_BUFFER, &v.vbuf);
            w_buf_clear (&v.vbuf);
            break;

        case _W_TNS_TAG_BOOLEAN:
            if (!w_tnetstr_parse_boolean (buffer, &v.vbool))
                ret = w_variant_new (W_VARIANT_BOOL, v.vbool);
            break;

        case _W_TNS_TAG_LIST:
            v.vlist = w_list_new (W_YES);
            if (!w_tnetstr_parse_list (buffer, v.vlist))
                ret = w_variant_new (W_VARIANT_LIST, v.vlist);
            w_obj_unref (v.vlist);
            break;

        case _W_TNS_TAG_DICT:
            v.vdict = w_dict_new (W_YES);
            if (!w_tnetstr_parse_dict (buffer, v.vdict))
                ret = w_variant_new (W_VARIANT_DICT, v.vdict);
            w_obj_unref (v.vdict);
            break;
    }

    return ret;
}

