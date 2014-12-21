/*
 * wioformat.c
 * Copyright (C) 2011-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"


static inline int
_map_digit (unsigned n)
{
    if (n < 10) {
        return '0' + n;
    }
    if (n < 36) {
        return 'A' + (n - 10);
    }
    return '?';
}


static inline w_io_result_t
format_ulong (w_io_t *io, unsigned long value, unsigned base)
{
    w_io_result_t r = W_IO_RESULT (0);

    if (value >= base) {
        W_IO_CHAIN (r, format_ulong (io, value / base, base));
    }
    W_IO_CHAIN (r, w_io_putchar (io, _map_digit (value % base)));
    return r;
}


w_io_result_t
w_io_format_long (w_io_t *io, long value)
{
    w_assert (io);

    if (value < 0) {
        w_io_result_t r = W_IO_RESULT (0);
        W_IO_CHAIN (r, w_io_putchar (io, '-'));
        W_IO_CHAIN (r, format_ulong (io, -value, 10));
        return r;
    }
    else {
        return format_ulong (io, value, 10);
    }
}


w_io_result_t
w_io_format_ulong (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return format_ulong (io, value, 10);
}


w_io_result_t
w_io_format_ulong_hex (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return format_ulong (io, value, 16);
}


w_io_result_t
w_io_format_ulong_oct (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return format_ulong (io, value, 8);
}


w_io_result_t
w_io_format_double (w_io_t *io, double value)
{
    w_assert (io);

    /*
     * FIXME Avoid head allocation of the temporary string.
     */
    char *str = w_strfmt ("%g", value);
    w_io_result_t r = w_io_write (io, str, strlen (str));
    w_free (str);
    return r;
}
