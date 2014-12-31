/*
 * wio.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * Input/Output Streams
 * ====================
 *
 * .. _formatted-output:
 *
 * Formatted output
 * ----------------
 */

/**
 * Types
 * -----
 */

/*~t w_io_result_t
 *
 * Represents the result of an input/output operation, which is one of:
 *
 * - A successful operation, optionally indicating the amount of data
 *   involved.
 *
 * - An indication that the end-of-file marker has been reached — typically
 *   used when reading data from a stream.
 *
 * - An error, which signals the failure of the operation.
 */

/*~t w_io_t
 *
 * Represents an input/output stream.
 */

/**
 * Macros
 * ------
 */

/*~M W_IO_RESULT(bytes)
 *
 * Makes a :type:`w_io_result_t` value which indicates a successful operation
 * which handled the given amount of `bytes`.
 */

/*~M W_IO_RESULT_ERROR(error)
 *
 * Makes a :type:`w_io_result_t` value which indicates a failure due to given
 * `error`.
 */

/*~M W_IO_RESULT_EOF
 *
 * Makes a :type:`w_io_result_t` value which indicates a successful operation
 * that reached the end-of-file marker.
 */

/*~M W_IO_RESULT_SUCCESS
 *
 * Makes a :type:`w_io_result_t` value which indicates a successful operation.
 */


/**
 * .. _wio-functions:
 *
 * Functions
 * ---------
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
    /* Unfortunately, errors can't be reported here. */
    W_IO_NORESULT (w_io_close ((w_io_t*) obj));
}


/*~f void w_io_init (w_io_t *stream)
 *
 * Initializes a base input/output `stream`.
 */
void
w_io_init (w_io_t *io)
{
    w_assert (io);

    memset (io, 0x00, sizeof (w_io_t));
    io->backch = W_IO_EOF;
    w_obj_dtor (io, w_io_cleanup);
}


/*~f w_io_result_t w_io_close (w_io_t *stream)
 *
 * Closes an input/output `stream`.
 */
w_io_result_t
w_io_close (w_io_t *io)
{
    w_assert (io);
    w_io_result_t r = W_IO_RESULT_SUCCESS;

    if (io->close) {
        r = (*io->close) (io);
        io->close = NULL;
    }
    return r;
}


/*~f w_io_result_t w_io_read (w_io_t *stream, void *buffer, size_t count)
 *
 * Reads up to `count` bytes from the an input `stream`, placing the data in
 * in memory starting at `buffer`.
 *
 * Passing a `count` of zero always succeeds and has no side effects.
 *
 * If reading succeeds, the amount of bytes read may be smaller than the
 * requested `count`. The reason may be that the end-of-file marker has been
 * reached (and it will be notified at the next attempt of reading data), or
 * because no more data is available for reading at the moment.
 */
w_io_result_t
w_io_read (w_io_t *io, void *buf, size_t len)
{
    w_assert (io);
    w_io_result_t r = W_IO_RESULT (0);

    if (w_unlikely (len == 0))
        return r;

    w_assert (buf);

    /* Handle the putback character... makes things a bit messier */
    if (w_unlikely (io->backch != W_IO_EOF)) {
        *((char*) buf) = io->backch;
        buf = (char*) buf + 1;
        io->backch = W_IO_EOF;

        /* Check whether more characters are to be read */
        if (!--len)
            return W_IO_RESULT (1);
    }

    if (w_likely (io->read != NULL)) {
        r = (*io->read) (io, buf, len);
    } else {
        r = W_IO_RESULT_ERROR (errno = EBADF);
    }
    return r;
}


/*~f w_io_result_t w_io_write (w_io_t *stream, const void *buffer, size_t count)
 *
 * Writes up to `count` bytes from the data in memory starting at `buffer` to
 * an output `stream`.
 *
 * Passing a `count` of zero always succeeds and has no side effects.
 */
w_io_result_t
w_io_write (w_io_t *io, const void *buf, size_t len)
{
    w_assert (io);
    w_io_result_t r = W_IO_RESULT (0);

    if (w_unlikely (len == 0))
        return r;

    w_assert (buf);
    if (w_likely (io->write != NULL)) {
        r = (*io->write) (io, buf, len);
    } else {
        r = W_IO_RESULT_ERROR (errno = EBADF);
    }
    return r;
}


/*~f int w_io_getchar (w_io_t *stream)
 *
 * Reads the next character from a input `stream`.
 *
 * If the enf-of-file marker is reached, :data:`W_IO_EOF` is returned.
 * On errors, negative values are returned.
 */
int
w_io_getchar (w_io_t *io)
{
    w_assert (io);

    char ch;
    w_io_result_t r = w_io_read (io, &ch, 1);

    if (w_io_failed (r))
        return -w_io_result_error (r);
    if (w_io_eof (r))
        return W_IO_EOF;

    return ch;
}


/*~f w_io_result_t w_io_putchar (w_io_t *stream, int character)
 *
 * Writes a `character` to an output `stream`.
 */
w_io_result_t
w_io_putchar (w_io_t *io, int ch)
{
    w_assert (io);

    char bch = ch;
    return w_io_write (io, &bch, 1);
}


/*~f void w_io_putback (w_io_t *stream, int character)
 *
 * Pushes a `character` back into an input `stream`, making it available
 * during the next read operation.
 *
 * .. warning:: Pushing more than one character is not supported, and only
 *    the last pushed one will be saved.
 */
void
w_io_putback (w_io_t *io, int ch)
{
    w_assert (io);
    io->backch = ch;
}


/*~f w_io_result_t w_io_flush (w_io_stream *stream)
 *
 * For an output `stream`, forces writing buffered data to the stream.
 *
 * For in input `stream`, discards data that may have been fetched from the
 * stream but still not consumed by the application.
 */
w_io_result_t
w_io_flush (w_io_t *io)
{
    w_assert (io);

    if (io->flush) {
        return (*io->flush) (io);
    } else {
        return W_IO_RESULT_ERROR (errno = EBADF);
    }
}


/*~f int w_io_get_fd (w_io_t *stream)
 *
 * Obtains the underlying file descriptor used by a `stream`.
 *
 * .. warning:: Not all types of input/output streams have an associated file
 *    descriptor, and a negative value will be returned for those.
 */
int
w_io_get_fd (w_io_t *io)
{
    w_assert (io);

    if (io->getfd) {
        return (*io->getfd) (io);
    } else {
        errno = EBADF;
        return -1;
    }
}


w_io_result_t
w_io_format (w_io_t *io, const char *fmt, ...)
{
    w_assert (io);
    w_assert (fmt);

    va_list args;
    va_start (args, fmt);
    w_io_result_t r = w_io_formatv (io, fmt, args);
    va_end (args);
    return r;
}


w_io_result_t
w_io_formatv (w_io_t *io, const char *fmt, va_list args)
{
    w_assert (io);
    w_assert (fmt);

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
        w_io_result_t viores;
    } v;

    w_io_result_t r = W_IO_RESULT (0);
    for (; *fmt; fmt++) {
        if (*fmt != '$') {
            W_IO_CHAIN (r, w_io_putchar (io, *fmt));
            continue;
        }

        switch (*(++fmt)) {
            case 'l':
                v.vlong = va_arg (args, long);
                W_IO_CHAIN (r, w_io_format_long (io, v.vlong));
                break;
            case 'i':
                v.vint = va_arg (args, int);
                W_IO_CHAIN (r, w_io_format_long (io, v.vint));
                break;
            case 'c':
                v.vint = va_arg (args, int);
                W_IO_CHAIN (r, w_io_putchar (io, v.vint));
                break;
            case 'I':
                v.vuint = va_arg (args, unsigned int);
                W_IO_CHAIN (r, w_io_format_ulong (io, v.vuint));
                break;
            case 'L':
                v.vulong = va_arg (args, unsigned long);
                W_IO_CHAIN (r, w_io_format_ulong (io, v.vulong));
                break;
            case 'X':
                v.vulong = va_arg (args, unsigned long);
                W_IO_CHAIN (r, w_io_format_ulong_hex (io, v.vulong));
                break;
            case 'O':
                v.vulong = va_arg (args, unsigned long);
                W_IO_CHAIN (r, w_io_format_ulong_oct (io, v.vulong));
                break;
            case 'f':
            case 'F':
                v.vfpnum = va_arg (args, double);
                W_IO_CHAIN (r, w_io_format_double (io, v.vfpnum));
                break;
            case 'p':
                v.vpointer = (intptr_t) va_arg (args, void*);
                W_IO_CHAIN (r, w_io_format_ulong_hex (io, v.vpointer));
                break;
            case 's':
                v.vcharp = va_arg (args, const char*);
                W_IO_CHAIN (r, w_io_write (io, v.vcharp, strlen (v.vcharp)));
                break;
            case 'B':
                v.vbufp = va_arg (args, w_buf_t*);
                W_IO_CHAIN (r, w_io_write (io,
                                           w_buf_str (v.vbufp),
                                           w_buf_size (v.vbufp)));
                break;
            case 'S':
                len_aux  = va_arg (args, size_t);
                v.vcharp = va_arg (args, const char*);
                W_IO_CHAIN (r, w_io_write (io, v.vcharp, len_aux));
                break;
            case 'e':
                W_IO_CHAIN (r, w_io_format_long (io, last_errno));
                break;
            case 'E':
                v.vcharp = strerror (last_errno);
                W_IO_CHAIN (r, w_io_write (io, v.vcharp, strlen (v.vcharp)));
                break;
            case 'R':
                v.viores = va_arg (args, w_io_result_t);
                W_IO_CHAIN (r, w_io_write (io, "IO<", 3));
                if (w_io_failed (v.viores)) {
                    v.vcharp = strerror (w_io_result_error (v.viores));
                    W_IO_CHAIN (r, w_io_write (io, v.vcharp, strlen (v.vcharp)));
                } else if (w_io_eof (v.viores)) {
                    W_IO_CHAIN (r, w_io_write (io, "EOF", 3));
                } else {
                    W_IO_CHAIN (r, w_io_format_ulong (io, w_io_result_bytes (v.viores)));
                }
                W_IO_CHAIN (r, w_io_putchar (io, '>'));
                break;
            default:
                W_IO_CHAIN (r, w_io_putchar (io, *fmt));
        }
    }

    return r;
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
    w_assert (io);
    w_assert (fmt);

    ssize_t retval = 0;

#define CHAR_TO_FUN(_c, _f) \
        case _c : rfun = (bool (*)(w_io_t*, void*)) _f; break

    for (; *fmt ; fmt++) {
        if (*fmt == '$') {
            void *dptr = va_arg (args, void*);
            bool (*rfun) (w_io_t*, void*);

            switch (*(++fmt)) {
                CHAR_TO_FUN ('i', w_io_fscan_int);
                CHAR_TO_FUN ('l', w_io_fscan_long);
                CHAR_TO_FUN ('I', w_io_fscan_uint);
                CHAR_TO_FUN ('L', w_io_fscan_ulong);
                CHAR_TO_FUN ('X', w_io_fscan_ulong_hex);
                CHAR_TO_FUN ('O', w_io_fscan_ulong_oct);
                CHAR_TO_FUN ('f', w_io_fscan_float);
                CHAR_TO_FUN ('F', w_io_fscan_double);
                CHAR_TO_FUN ('w', w_io_fscan_word);
                default: rfun = NULL;
            }

            if (rfun) {
                if (!(*rfun) (io, dptr))
                    retval++;
                continue;
            }
        }

        int ch;
        if ((ch = w_io_getchar (io)) != *fmt) {
            w_io_putback (io, ch);
            break;
        }
    }

    return retval;
}


w_io_result_t
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
        char *pos = memchr (w_buf_data (overflow),
                            stopchar,
                            w_buf_size (overflow));

        if (pos != NULL) {
            /*
             * Stop character is in overflow buffer: remove it from the
             * overflow buffer, copy data to result buffer.
             */
            unsigned len = pos - w_buf_data (overflow) + 1;
            w_buf_append_mem (buffer, w_buf_data (overflow), len);
            overflow->size -= len;
            memmove (w_buf_data (overflow),
                     w_buf_data (overflow) + len,
                     w_buf_size (overflow));
            w_buf_resize (buffer, w_buf_size (buffer) - 1);
            return W_IO_RESULT (w_buf_size (buffer));
        }

        if (overflow->alloc < (w_buf_size (overflow) + readbytes))
        {
            /*
             * XXX Calling w_buf_resize() will *both* resize the buffer
             * data area and set overflow->bsz *and* overflow->len. But we
             * do not want the later to be changed we save and restore it.
             */
            size_t oldlen = w_buf_size (overflow);
            w_buf_resize (overflow, w_buf_size (overflow) + readbytes);
            overflow->size = oldlen;
        }

        w_io_result_t r = w_io_read (io,
                                     w_buf_data (overflow) + w_buf_size (overflow),
                                     readbytes);

        if (!w_io_failed (r) && w_io_result_bytes (r) > 0) {
            overflow->size += w_io_result_bytes (r);
        } else {
            /* Handles both EOF and errors. */
            return r;
        }
    }
}

