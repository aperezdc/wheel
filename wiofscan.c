/*
 * wiofscan.c
 * Copyright (C) 2011-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <limits.h>
#include <ctype.h>
#include <float.h>
#include <math.h>


w_bool_t
w_io_fscan_double (w_io_t *io, double *result)
{
    /*
     * Conversion from string to floating point representation is very hard
     * to get right. Instead of rolling our own, the idea is to validate the
     * input, bailing out as soon as possible, and once we have a buffer
     * with the needed data, pass it to strtod() -- thus letting the libc
     * handle the hard part.
     *
     * TODO Have our own strtod/atod routines.
     */
    w_bool_t got_exp = W_NO;
    w_bool_t got_dot = W_NO;
    w_buf_t buf = W_BUF;
    int c;

    w_assert (io);

    switch ((c = w_io_getchar (io))) {
        /* If we get an "N", the only option is reading a NaN.  */
        case 'n':
        case 'N':
            if ((c = w_io_getchar (io)) != 'a' && c != 'A') goto failure;
            if ((c = w_io_getchar (io)) != 'n' && c != 'N') goto failure;
            if (result)
                *result = NAN;
            goto success;

        /* If we get an "I", the only option is reading an INF/INFINITY. */
        case 'i':
        case 'I':
            if ((c = w_io_getchar (io)) != 'n' && c != 'N') goto failure;
            if ((c = w_io_getchar (io)) != 'f' && c != 'F') goto failure;
            if ((c = w_io_getchar (io)) == 'i' && c == 'I') {
                if ((c = w_io_getchar (io)) != 'n' && c != 'N') goto failure;
                if ((c = w_io_getchar (io)) != 'i' && c != 'I') goto failure;
                if ((c = w_io_getchar (io)) != 't' && c != 'T') goto failure;
                if ((c = w_io_getchar (io)) != 'y' && c != 'Y') goto failure;
            }
            if (result)
                *result = INFINITY;
            goto success;

        /* Sign marker is read, next has to be a number in XX.YY[eZZ] format. */
        case '-':
        case '+':
            w_buf_append_char (&buf, c);
            break;
        default:
            w_io_putback (io, c);
    }

    while ((c = w_io_getchar (io)) != W_IO_EOF) {
        if (c == '.') {
            if (got_dot) {
                w_io_putback (io, c);
                break;
            }
            got_dot = W_YES;
        }
        else if (c == 'e' || c == 'E') {
            if (got_exp) break;
            /* Take into account sign in exponent */
            if ((c = w_io_getchar (io)) != W_IO_EOF && (c == '-' || c == '+')) {
                w_buf_append_char (&buf, 'e');
            }
            else {
                w_io_putback (io, c);
                c = 'e';
            }
            got_exp = W_YES;
        }
        else if (!isdigit (c)) {
            w_io_putback (io, c);
            break;
        }
        w_buf_append_char (&buf, c);
    }

    if (!w_buf_size (&buf))
        goto failure;

    if (got_dot && w_buf_size (&buf) == 1) {
        w_assert (w_buf_data (&buf)[0] == '.');
        c = '.';
        goto failure;
    }

    if (result)
        *result = strtod (w_buf_str (&buf), NULL);

success:
    w_buf_clear (&buf);
    return W_NO;

failure:
    w_io_putback (io, c);
    w_buf_clear (&buf);
    return W_YES;
}


w_bool_t
w_io_fscan_long (w_io_t *io, long *result)
{
    w_bool_t signchange = W_NO;
    unsigned long uval;
    int chr;

    w_assert (io);

    switch ((chr = w_io_getchar (io))) {
        case '-':
            signchange = W_YES;
        case '+':
            break;
        default:
            w_io_putback (io, chr);
    }

    if (w_io_fscan_ulong (io, &uval))
        return W_YES;

    *result = 0;

    if (signchange) {
        if (uval > LONG_MAX) {
            *result = LONG_MIN;
            return W_YES;
        }
        *result = -uval;
    }
    else {
        if (uval > LONG_MAX) {
            *result = LONG_MAX;
            return W_YES;
        }
        *result = uval;
    }

    return W_NO;
}


w_bool_t
w_io_fscan_ulong (w_io_t *io, unsigned long *result)
{
    w_assert (io);

    unsigned long temp = 0;
    int chr;

    /* If the next character is not a digit, signal it as format error */
start_digit:
    chr = w_io_getchar (io);
    if (chr == W_IO_EOF || !isdigit (chr)) {
        if (chr == '+') {
            goto start_digit;
        }
        w_io_putback (io, chr);
        return W_YES;
    }

    do {
        temp *= 10;
        temp += chr - '0';
        chr = w_io_getchar (io);
    } while (chr != W_IO_EOF && isdigit (chr));

    if (chr != W_IO_EOF && chr >= 0) {
        w_io_putback (io, chr);
    }

    /*
     * TODO Add ULONG_MAX overflow checking
     */
    if (result) {
        *result = temp;
    }

    return W_NO;
}


static inline int
_map_hexchar (int ch)
{
    switch (ch) {
        case 'a': case 'A': return 0xA;
        case 'b': case 'B': return 0xB;
        case 'c': case 'C': return 0xC;
        case 'd': case 'D': return 0xD;
        case 'e': case 'E': return 0xE;
        case 'f': case 'F': return 0xF;
        default: return ch - '0';
    }
}


w_bool_t
w_io_fscan_ulong_hex (w_io_t *io, unsigned long *result)
{
    w_assert (io);

    unsigned long temp = 0;
    int chr;

    if ((chr = w_io_getchar (io)) != '0') {
        w_io_putback (io, chr);
        return W_YES;
    }

    if ((chr = w_io_getchar (io)) != 'x' && chr != 'X') {
        goto end;
    }

    while ((chr = w_io_getchar (io)) != W_IO_EOF && isxdigit (chr)) {
        temp *= 16;
        temp += _map_hexchar (chr);
    }

    if (chr != W_IO_EOF && chr >= 0) {
        w_io_putback (io, chr);
    }

end:
    if (result) {
        *result = temp;
    }

    return W_NO;
}


w_bool_t
w_io_fscan_ulong_oct (w_io_t *io, unsigned long *result)
{
    w_assert (io);
    unsigned long temp = 0;
    int chr;

    if ((chr = w_io_getchar (io)) != '0') {
        w_io_putback (io, chr);
        return W_YES;
    }

    while ((chr = w_io_getchar (io)) != W_IO_EOF &&
           isdigit (chr) && chr < '8') {
        temp *= 8;
        temp += chr - '0';
    }

    if (chr != W_IO_EOF && chr >= 0) {
        w_io_putback (io, chr);
    }

    if (result) {
        *result = temp;
    }

    return W_NO;
}


w_bool_t
w_io_fscan_int (w_io_t *io, int *result)
{
    long value;
    w_assert (io);

    if (w_io_fscan_long (io, &value))
        return W_YES;

    if (value > INT_MAX) {
        if (result)
            *result = INT_MAX;
        return W_YES;
    }
    if (value < INT_MIN) {
        if (result)
            *result = INT_MIN;
        return W_YES;
    }

    if (result)
        *result = (int) value;

    return W_NO;
}


w_bool_t
w_io_fscan_uint (w_io_t *io, unsigned int *result)
{
    unsigned long value;
    w_assert (io);

    if (w_io_fscan_ulong (io, &value))
        return W_YES;

    if (value > UINT_MAX) {
        if (result)
            *result = UINT_MAX;
        return W_YES;
    }

    if (result)
        *result = (unsigned int) value;

    return W_NO;
}


w_bool_t
w_io_fscan_float (w_io_t *io, float *result)
{
    double value;
    w_assert (io);

    if (w_io_fscan_double (io, &value))
        return W_YES;

    if (isnan (value) || isinf (value))
        goto success;

    if (value > FLT_MAX) {
        if (result)
            *result = FLT_MAX;
        return W_YES;
    }
    if (value < FLT_MIN) {
        if (result)
            *result = FLT_MIN;
        return W_YES;
    }

success:
    if (result)
        *result = (float) value;

    return W_NO;
}


w_bool_t
w_io_fscan_word (w_io_t *io, char **result)
{
    w_buf_t value = W_BUF;
    w_bool_t ret = W_NO;
    int chr;

    while (!isspace ((chr = w_io_getchar (io))) &&
           chr != W_IO_EOF && chr >= 0)
        w_buf_append_char (&value, chr);

    if (chr != W_IO_EOF && chr >= 0)
        w_io_putback (io, chr);

    ret = !w_buf_size (&value);
    if (!ret && result)
        *result = w_buf_str (&value);
    else
        w_buf_clear (&value);

    return ret;
}
