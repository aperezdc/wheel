/*
 * wio.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdint.h>
#include <errno.h>


#ifndef W_IO_READ_UNTIL_BYTES
#define W_IO_READ_UNTIL_BYTES 4096
#endif /* !W_IO_READ_UNTIL_BYTES */


static void
w_io_cleanup (void *obj)
{
    w_io_close ((w_io_t*) obj);
}


void
w_io_init (w_io_t *io)
{
    w_assert (io);

    io->backch = W_IO_EOF;
    w_obj_dtor (io, w_io_cleanup);
}


w_bool_t
w_io_close (w_io_t *io)
{
    w_bool_t ret;
    w_assert (io);

    if (io->close) {
        ret = (*io->close) (io);
        io->close = NULL;
    }
    else {
        ret = W_YES;
    }
    return ret;
}


ssize_t
w_io_read (w_io_t *io, void *buf, size_t len)
{
    w_assert (io);

    if (w_unlikely (len == 0))
        return 0;

    w_assert (buf);

    /* Handle the putback character... makes things a bit messier */
    if (w_unlikely (io->backch != W_IO_EOF)) {
        *((char*) buf) = io->backch;
        buf = (char*) buf + 1;
        io->backch = W_IO_EOF;

        /* Check whether more characters are to be read */
        if (!--len) return 1;
    }

    return (io->read)
        ? (*io->read) (io, buf, len)
        : (errno = EBADF, -1);
}


ssize_t
w_io_write (w_io_t *io, const void *buf, size_t len)
{
    w_assert (io);

    if (w_unlikely (len == 0))
        return 0;

    w_assert (buf);

    return (io->write)
        ? (*io->write) (io, buf, len)
        : (errno = EBADF, -1);
}


int
w_io_getchar (w_io_t *io)
{
    ssize_t ret;
    char ch;

    w_assert (io);

    switch ((ret = w_io_read (io, &ch, 1))) {
        case 1: /* One byte read, return character */
            return ch;

        case W_IO_EOF: /* Concrete return values */
        case W_IO_ERR:
            return ret;

        default: /* Any other value is an error */
            return W_IO_ERR;
    }
}


w_bool_t
w_io_putchar (w_io_t *io, int ch)
{
    char bch = ch;
    return (w_io_write (io, &bch, 1) != 1);
}


void
w_io_putback (w_io_t *io, char ch)
{
    w_assert (io);
    io->backch = ch;
}


w_bool_t
w_io_flush (w_io_t *io)
{
    w_assert (io);
    return (io->flush)
        ? (*io->flush) (io)
        : W_NO;
}


ssize_t
w_io_format (w_io_t *io, const char *fmt, ...)
{
    ssize_t ret;
    va_list args;

    w_assert (io);
    w_assert (fmt);

    va_start (args, fmt);
    ret = w_io_formatv (io, fmt, args);
    va_end (args);
    return ret;
}


ssize_t
w_io_formatv (w_io_t *io, const char *fmt, va_list args)
{
    int last_errno = errno;
    size_t len_aux;
    union {
        int           vint;
        unsigned int  vuint;
        long          vlong;
        unsigned long vulong;
        const char   *vcharp;
        w_buf_t      *vbufp;
        intptr_t      vpointer;
        double        vfpnum;
    } v;

    w_assert (io);
    w_assert (fmt);

    for (; *fmt; fmt++) {
        if (*fmt != '$') {
            w_io_putchar (io, *fmt);
            continue;
        }

        switch (*(++fmt)) {
            case 'l':
                v.vlong = va_arg (args, long);
                w_io_format_long (io, v.vlong);
                break;
            case 'i':
                v.vint = va_arg (args, int);
                w_io_format_long (io, v.vint);
                break;
            case 'c':
                v.vint = va_arg (args, int);
                w_io_putchar (io, v.vint);
                break;
            case 'I':
                v.vuint = va_arg (args, unsigned int);
                w_io_format_ulong (io, v.vuint);
                break;
            case 'L':
                v.vulong = va_arg (args, unsigned long);
                w_io_format_ulong (io, v.vulong);
                break;
            case 'X':
                v.vulong = va_arg (args, unsigned long);
                w_io_format_ulong_hex (io, v.vulong);
                break;
            case 'O':
                v.vulong = va_arg (args, unsigned long);
                w_io_format_ulong_oct (io, v.vulong);
                break;
            case 'f':
            case 'F':
                v.vfpnum = va_arg (args, double);
                w_io_format_double (io, v.vfpnum);
                break;
            case 'p':
                v.vpointer = (intptr_t) va_arg (args, void*);
                w_io_format_ulong_hex (io, v.vpointer);
                break;
            case 's':
                v.vcharp = va_arg (args, const char*);
                w_io_write (io, v.vcharp, strlen (v.vcharp));
                break;
            case 'B':
                v.vbufp = va_arg (args, w_buf_t*);
                w_io_write (io, w_buf_str (v.vbufp), w_buf_size (v.vbufp));
                break;
            case 'S':
                len_aux  = va_arg (args, size_t);
                v.vcharp = va_arg (args, const char*);
                w_io_write (io, v.vcharp, len_aux);
                break;
            case 'e':
                w_io_format_long (io, last_errno);
                break;
            case 'E':
                v.vcharp = strerror (last_errno);
                w_io_write (io, v.vcharp, strlen (v.vcharp));
                break;
            default:
                w_io_putchar (io, *fmt);
        }
    }

    return -1;
}


ssize_t
w_io_fscan (w_io_t *io, const char *fmt, ...)
{
    ssize_t ret;
    va_list args;

    w_assert (io);
    w_assert (fmt);

    va_start (args, fmt);
    ret = w_io_fscanv (io, fmt, args);
    va_end (args);

    return ret;
}


ssize_t
w_io_fscanv (w_io_t *io, const char *fmt, va_list args)
{
    w_bool_t (*rfun) (w_io_t*, void*);
    ssize_t retval = 0;
    int ch;

    w_assert (io);
    w_assert (fmt);

#define CHAR_TO_FUN(_c, _f) \
        case _c : rfun = (w_bool_t (*)(w_io_t*, void*)) _f; break

    for (; *fmt ; fmt++) {
        if (*fmt == '$') {
            void *dptr = va_arg (args, void*);

            switch (*(++fmt)) {
                CHAR_TO_FUN ('i', w_io_fscan_int);
                CHAR_TO_FUN ('l', w_io_fscan_long);
                CHAR_TO_FUN ('I', w_io_fscan_uint);
                CHAR_TO_FUN ('L', w_io_fscan_ulong);
                CHAR_TO_FUN ('X', w_io_fscan_ulong_hex);
                CHAR_TO_FUN ('O', w_io_fscan_ulong_oct);
                CHAR_TO_FUN ('f', w_io_fscan_float);
                CHAR_TO_FUN ('F', w_io_fscan_double);
                default: rfun = NULL;
            }

            if (rfun) {
                if (!(*rfun) (io, dptr))
                    retval++;
                continue;
            }
        }

        if ((ch = w_io_getchar (io)) != *fmt) {
            w_io_putback (io, ch);
            break;
        }
    }

    return retval;
}


ssize_t
w_io_read_until (w_io_t  *io,
                 w_buf_t *buffer,
                 w_buf_t *overflow,
                 int      stopchar,
                 unsigned readbytes)
{
    w_assert (io);
    w_assert (buffer);
    w_assert (overflow);

    if (!readbytes)
        readbytes = W_IO_READ_UNTIL_BYTES;

    for (;;) {
        ssize_t c;
        char *pos;

        pos = memchr (w_buf_data (overflow),
                      stopchar,
                      w_buf_size (overflow));

        if (pos != NULL) {
            /*
             * Stop character is in overflow buffer: remove it from the
             * overflow buffer, copy data to result buffer.
             */
            unsigned len = pos - w_buf_data (overflow) + 1;
            w_buf_append_mem (buffer, w_buf_data (overflow), len);
            w_buf_size (overflow) -= len;
            memmove (w_buf_data (overflow),
                     w_buf_data (overflow) + len,
                     w_buf_size (overflow));
            w_buf_resize (buffer, w_buf_size (buffer) - 1);
            return w_buf_size (buffer);
        }

        if (w_buf_alloc_size (overflow) < (w_buf_size (overflow) + readbytes))
        {
            /*
             * XXX Calling w_buf_resize() will *both* resize the buffer
             * data area and set overflow->bsz *and* overflow->len. But we
             * do not want the later to be changed we save and restore it.
             */
            size_t oldlen = w_buf_size (overflow);
            w_buf_resize (overflow, w_buf_size (overflow) + readbytes);
            w_buf_size (overflow) = oldlen;
        }

        c = w_io_read (io,
                       w_buf_data (overflow) + w_buf_size (overflow),
                       readbytes);

        if (c > 0)
            w_buf_size (overflow) += c;
        else
            return c;
    }
}

