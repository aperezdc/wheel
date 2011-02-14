/*
 * wioformat.c
 * Copyright (C) 2011 Adrian Perez <aperez@igalia.com>
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
w_io_format_ulong_base (w_io_t *io, unsigned long value, unsigned base)
{
    ssize_t ret = 1;
    if (value >= base) {
        ret += w_io_format_ulong_base (io, value / base, base);
    }
    w_io_putchar (io, _map_digit (value % base));
    return ret;
}


ssize_t
w_io_format_long (w_io_t *io, long value)
{
    w_assert (io);

    if (value < 0) {
        w_io_putchar (io, '-');
        return 1 + w_io_format_ulong_base (io, -value, 10);
    }
    else {
        return w_io_format_ulong_base (io, value, 10);
    }
}


ssize_t
w_io_format_ulong (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return w_io_format_ulong_base (io, value, 10);
}


ssize_t
w_io_format_ulong_hex (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return w_io_format_ulong_base (io, value, 16);
}


ssize_t
w_io_format_ulong_oct (w_io_t *io, unsigned long value)
{
    w_assert (io);
    return w_io_format_ulong_base (io, value, 8);
}
