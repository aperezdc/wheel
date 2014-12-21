/*
 * wbuf.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"

#ifndef W_BUF_CHUNK_SIZE
#define W_BUF_CHUNK_SIZE 512
#endif /* !W_BUF_CHUNK_SIZE */


static inline void
_buf_resize (w_buf_t *buf, size_t size)
{
    if (size) {
        size_t nsz = W_BUF_CHUNK_SIZE * ((size / W_BUF_CHUNK_SIZE) + 1);
        if (nsz < size) {
            nsz = size;
        }
        if (nsz != buf->alloc) {
            buf->data  = w_resize (buf->data, char, nsz + 1);
            buf->alloc = nsz;
        }
    }
    else {
        if (buf->data) {
            w_free (buf->data);
        }
        buf->alloc = 0;
    }
}


static inline void
_buf_xresize (w_buf_t *buf, size_t size)
{
    _buf_resize (buf, size);
    buf->size = size;
}


void
w_buf_resize (w_buf_t *buf, size_t size)
{
    w_assert (buf);
    _buf_xresize (buf, size);
}


void
w_buf_set_str (w_buf_t *buf, const char *str)
{
    w_assert (buf);
    w_assert (str);

    size_t slen = strlen (str);
    _buf_xresize (buf, slen);
    memcpy (buf->data, str, slen);
}


void
w_buf_append_mem (w_buf_t *buf, const void *ptr, size_t len)
{
    size_t bsize;

    w_assert (buf);
    w_assert (ptr);

    bsize = buf->size;
    _buf_xresize (buf, bsize + len);
    memcpy (buf->data + bsize, ptr, len);
}


void
w_buf_append_str (w_buf_t *buf, const char *str)
{
    size_t bsize;
    size_t slen;

    w_assert (buf);
    w_assert (str);

    bsize = buf->size;
    slen = strlen (str);
    _buf_xresize (buf, bsize + slen);
    memcpy (buf->data + bsize, str, slen);
}


void
w_buf_append_char (w_buf_t *buf, int chr)
{
    w_assert (buf);
    _buf_xresize (buf, buf->size + 1);
    buf->data[buf->size - 1] = chr;
}


void
w_buf_append_buf (w_buf_t *buf, const w_buf_t *src)
{
    w_assert (buf);
    w_assert (src);

    size_t bsize = buf->size;
    _buf_xresize (buf, bsize + src->size);
    memcpy (buf->data + bsize, src->data, src->size);
}


char*
w_buf_str (w_buf_t *buf)
{
    w_assert (buf);

    if (!buf->size) {
        _buf_xresize (buf, 1);
        buf->size = 0;
    }
    buf->data[buf->size] = '\0';
    return buf->data;
}


void
w_buf_clear (w_buf_t *buf)
{
    w_assert (buf);
    _buf_xresize (buf, 0);
}


w_io_result_t
w_buf_format (w_buf_t *buf, const char *fmt, ...)
{
    w_assert (buf);
    w_assert (fmt);

    w_io_buf_t io;
    w_io_buf_init (&io, buf, W_YES);

    va_list al;
    va_start (al, fmt);

    w_io_result_t r = w_io_formatv ((w_io_t*) &io, fmt, al);

    va_end (al);
    return r;
}

