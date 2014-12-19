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


static inline ssize_t
format_ulong (w_io_t *io, unsigned long value, unsigned base)
{
    ssize_t result = 1;

    if (value >= base) {
        ssize_t t = format_ulong (io, value / base, base);
        if (t < 0)
            return t;
        result += t;
    }

    return w_io_putchar (io, _map_digit (value % base)) ? result : -1;
}


ssize_t
w_io_format_long (w_io_t *io, long value)
{
    w_assert (io);

    if (value < 0) {
        if (!w_io_putchar (io, '-'))
            return -1;
        ssize_t ret = format_ulong (io, -value, 10);
        return (ret < 0) ? ret : ++ret;
    }
    else {
        return format_ulong (io, value, 10);
    }
}


ssize_t
w_io_format_ulong (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return format_ulong (io, value, 10);
}


ssize_t
w_io_format_ulong_hex (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return format_ulong (io, value, 16);
}


ssize_t
w_io_format_ulong_oct (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return format_ulong (io, value, 8);
}


ssize_t
w_io_format_double (w_io_t *io, double value)
{
    w_assert (io);

    /*
     * FIXME Avoid head allocation of the temporary string.
     */
    char *str = w_strfmt ("%g", value);
    ssize_t ret = w_io_write (io, str, strlen (str));
    w_free (str);
    return ret;
}
