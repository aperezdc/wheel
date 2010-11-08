/*
 * wbuf.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"

#ifndef W_BUF_CHUNK_SIZE
#define W_BUF_CHUNK_SIZE 512
#endif /* !W_BUF_CHUNK_SIZE */


static inline void
_buf_setlen (w_buf_t *buf, size_t len)
{
    if (len) {
        size_t nsz = W_BUF_CHUNK_SIZE * ((len / W_BUF_CHUNK_SIZE) + 1);
        if (nsz < len) {
            nsz = len;
        }
        if (nsz != buf->bsz) {
            buf->buf = w_resize (buf->buf, char, nsz + 1);
            buf->bsz = nsz;
        }
    }
    else {
        if (buf->buf) {
            w_free (buf->buf);
        }
        buf->bsz = 0;
    }
}


static inline void
_buf_xsetlen(w_buf_t *buf, size_t len)
{
    _buf_setlen (buf, len);
    buf->len = len;
}


void
w_buf_length_set (w_buf_t *buf, size_t len)
{
    w_assert (buf);
    _buf_xsetlen (buf, len);
}


void
w_buf_set_str (w_buf_t *buf, const char *str)
{
    size_t slen;

    w_assert (buf);
    w_assert (str);

    slen = strlen (str);
    _buf_xsetlen (buf, slen);
    memcpy (buf->buf, str, slen);
}


void
w_buf_append_mem (w_buf_t *buf, const void *ptr, size_t len)
{
    size_t blen;

    w_assert (buf);
    w_assert (ptr);

    blen = buf->len;
    _buf_xsetlen (buf, blen + len);
    memcpy (buf->buf + blen, ptr, len);
}


void
w_buf_append_str (w_buf_t *buf, const char *str)
{
    size_t blen;
    size_t slen;

    w_assert (buf);
    w_assert (str);

    blen = buf->len;
    slen = strlen (str);
    _buf_xsetlen (buf, blen + slen);
    memcpy (buf->buf + blen, str, slen);
}


void
w_buf_append_char (w_buf_t *buf, int chr)
{
    w_assert (buf);
    _buf_xsetlen (buf, buf->len + 1);
    buf->buf[buf->len-1] = chr;
}


void
w_buf_append_buf (w_buf_t *buf, const w_buf_t *src)
{
    w_assert (buf);
    w_assert (src);

    size_t blen = buf->len;
    _buf_xsetlen (buf, blen + src->len);
    memcpy (buf->buf + blen, src->buf, src->len);
}


const char*
w_buf_str (const w_buf_t *buf)
{
    w_assert (buf);

    if (buf->len) {
        buf->buf[buf->len] = '\0';
        return buf->buf;
    }
    else {
        return "";
    }
}


void
w_buf_free (w_buf_t *buf)
{
    w_assert (buf);
    _buf_xsetlen (buf, 0);
}

