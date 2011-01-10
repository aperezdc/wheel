/*
 * wiofscan.c
 * Copyright (C) 2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <limits.h>
#include <ctype.h>


wbool
w_io_fscan_double (w_io_t *io, double *result)
{
    w_assert (io);
    w_unused (io);
    w_unused (result);
    return W_NO;
}


wbool
w_io_fscan_long (w_io_t *io, long *result)
{
    wbool signchange = W_NO;
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


wbool
w_io_fscan_ulong (w_io_t *io, unsigned long *result)
{
    int chr;
    unsigned long temp = 0;

    w_assert (io);

    /* If the next character is not a digit, signal it as format error */
start_digit:
    if (!isdigit ((chr = w_io_getchar (io)))) {
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
    } while (isdigit (chr));

    if (chr != W_IO_EOF && chr != W_IO_ERR) {
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


wbool
w_io_fscan_ulong_hex (w_io_t *io, unsigned long *result)
{
    int chr;
    unsigned long temp = 0;

    w_assert (io);

    if ((chr = w_io_getchar (io)) != '0') {
        w_io_putback (io, chr);
        return W_YES;
    }

    if ((chr = w_io_getchar (io)) != 'x' && chr != 'X') {
        goto end;
    }

    while (isxdigit ((chr = w_io_getchar (io)))) {
        temp *= 16;
        temp += _map_hexchar (chr);
    }

    if (chr != W_IO_EOF && chr != W_IO_ERR) {
        w_io_putback (io, chr);
    }

end:
    if (result) {
        *result = temp;
    }

    return W_NO;
}


wbool
w_io_fscan_ulong_oct (w_io_t *io, unsigned long *result)
{
    int chr;
    unsigned long temp = 0;

    w_assert (io);

    if ((chr = w_io_getchar (io)) != '0') {
        w_io_putback (io, chr);
        return W_YES;
    }

    while (isdigit ((chr = w_io_getchar (io))) && chr < '8') {
        temp *= 8;
        temp += chr - '0';
    }

    if (chr != W_IO_EOF && chr != W_IO_ERR) {
        w_io_putback (io, chr);
    }

    if (result) {
        *result = temp;
    }

    return W_NO;
}

