/*
 * Support functions for building simple parsers.
 *
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdarg.h>
#include <ctype.h>


void
w_parse_skip_ws (w_parse_t *p)
{
    w_assert (p != NULL);

    while (p->look != W_IO_EOF && isspace (p->look))
        w_parse_getchar (p);
}


void
w_parse_getchar (w_parse_t *p)
{
    w_assert (p != NULL);

    do {
        p->look = w_io_getchar (p->input);

        if (p->look == '\n') {
            p->lpos = 0;
            p->line++;
        }
        p->lpos++;

        if (p->comment && p->look == p->comment) {
            while ((p->look = w_io_getchar (p->input)) != '\n' &&
                   (p->look != W_IO_EOF)) /* empty statement */;

            if (w_unlikely (p->look != W_IO_EOF)) {
                w_io_putback (p->input, '\n');
            }
        }
    } while (p->look != W_IO_EOF && p->comment && p->look == p->comment);
}


void
w_parse_ferror (w_parse_t  *p,
                const char *fmt,
                ...)
{
    w_assert (p != NULL);
    w_assert (fmt != NULL);

    w_buf_t buf = W_BUF;
    va_list args;

    w_buf_format (&buf, "$I:$I ", p->line, p->lpos);
    va_start (args, fmt);
    w_buf_formatv (&buf, fmt, args);
    va_end (args);
    p->error = w_buf_str (&buf);
}


void
w_parse_rerror (w_parse_t *p)
{
    w_assert (p != NULL);
    longjmp (p->jbuf, 1);
}


void
w_parse_error (w_parse_t *p, const char *fmt, ...)
{
    w_assert (p != NULL);
    w_assert (fmt != NULL);

    w_buf_t buf = W_BUF;
    va_list args;

    w_buf_format (&buf, "$I:$I ", p->line, p->lpos);
    va_start (args, fmt);
    w_buf_formatv (&buf, fmt, args);
    va_end (args);
    p->error = w_buf_str (&buf);

    w_parse_rerror (p);
}


char*
w_parse_ident (w_parse_t *p)
{
    w_buf_t buf = W_BUF;

    w_assert (p != NULL);

    if (!isalpha (p->look) && p->look != '_')
        return NULL;

    while (isalnum (p->look) || p->look == '_') {
        w_buf_append_char (&buf, p->look);
        w_parse_getchar (p);
    }

    w_parse_skip_ws (p);
    return w_buf_str (&buf);
}


bool
w_parse_ulong (w_parse_t      *p,
               unsigned long  *value)
{
    w_assert (p != NULL);
    w_assert (value != NULL);

    if (p->look == '0') {
        w_parse_getchar (p);
        if (p->look == 'x' || p->look == 'X') {
            if (w_io_fscan_ulong_hex (p->input, value))
                return false;
        }
        else if (isdigit (p->look)) {
            w_io_putback (p->input, p->look);
            if (w_io_fscan_ulong_oct (p->input, value))
                return false;
        }
        else {
            w_io_putback (p->input, p->look);
            *value = 0;
        }
    }
    else {
        w_io_putback (p->input, p->look);
        if (w_io_fscan_ulong (p->input, value))
            return false;
    }

    w_parse_getchar (p);
    w_parse_skip_ws (p);
    return true;
}


bool
w_parse_long (w_parse_t *p, long *value)
{
    unsigned long uval;

    w_assert (p != NULL);
    w_assert (value != NULL);

    if (p->look == '0') {
        w_parse_getchar (p);
        if (p->look == 'x' || p->look == 'X') {
            if (w_io_fscan_ulong_hex (p->input, &uval))
                return false;
            *value = uval;
        }
        else if (isdigit (p->look)) {
            w_io_putback (p->input, p->look);
            if (w_io_fscan_ulong_oct (p->input, &uval))
                return false;
            *value = uval;
        }
        else {
            w_io_putback (p->input, p->look);
            *value = 0;
        }
    }
    else {
        w_io_putback (p->input, p->look);
        if (w_io_fscan_long (p->input, value))
            return false;
    }

    w_parse_getchar (p);
    w_parse_skip_ws (p);
    return true;
}


bool
w_parse_double (w_parse_t *p, double *value)
{
    w_assert (p != NULL);
    w_assert (value != NULL);

    w_io_putback (p->input, p->look);
    if (w_io_fscan_double (p->input, value))
        return false;

    w_parse_getchar (p);
    w_parse_skip_ws (p);

    return true;
}


void*
w_parse_run (w_parse_t    *p,
             w_io_t       *input,
             int           comment,
             w_parse_fun_t parse_fun,
             void         *context,
             char        **msg)
{
    w_assert (p != NULL);
    w_assert (input != NULL);
    w_assert (parse_fun != NULL);

    /* Initialize parser structure */
    memset (p, 0x00, sizeof (w_parse_t));
    p->comment = comment;
    p->input   = input;
    p->line    = 1;

    if (msg) {
        *msg = NULL;
    }

    if (setjmp (p->jbuf)) {
        /* Cleanup and return error. */
        if (msg) {
            *msg = p->error;
        }
        else {
            w_free (p->error);
        }
    }
    else {
        /* Do parsing */
        w_parse_getchar (p);
        w_parse_skip_ws (p);
        (*parse_fun) (p, context);
    }

    p->input = NULL;
    return p->result;
}


char*
w_parse_word (w_parse_t *p)
{
    w_buf_t buf = W_BUF;
    w_assert (p != NULL);

    while (!isspace (p->look) && p->look != W_IO_EOF) {
        w_buf_append_char (&buf, p->look);
        w_parse_getchar (p);
    }

    w_parse_getchar (p);
    w_parse_skip_ws (p);

    return w_buf_str (&buf);
}


char*
w_parse_string (w_parse_t *p)
{
    w_buf_t buf = W_BUF;
    int     chr = w_io_getchar (p->input);

    w_assert (p != NULL);

    for (; chr != '"' && chr != W_IO_EOF; chr = w_io_getchar (p->input)) {
        if (chr == '\\') {
            /* escaped sequences */
            switch ((chr = w_io_getchar (p->input))) {
                case 'n': chr = '\n'; break; /* carriage return */
                case 'r': chr = '\r'; break; /* line feed */
                case 'b': chr = '\b'; break; /* backspace */
                case 'e': chr = 0x1b; break; /* escape */
                case 'a': chr = '\a'; break; /* bell */
                case 't': chr = '\t'; break; /* tab */
                case 'v': chr = '\v'; break; /* vertical tab */
                case 'X': /* hex number */
                case 'x': { char num[3];
                            num[0] = w_io_getchar (p->input);
                            num[1] = w_io_getchar (p->input);
                            num[2] = '\0';
                            if (!isxdigit (num[0]) || !isxdigit (num[1])) {
                                w_buf_clear (&buf);
                                w_parse_error (p, "Invalid hex sequence");
                            }
                            chr = strtol (num, NULL, 16);
                          } break;
            }
        }
        w_buf_append_char (&buf, chr);
    }

    /* Premature end of string. */
    if (chr == W_IO_EOF) {
        w_buf_clear (&buf);
        return NULL;
    }

    /* I/O error. */
    if (chr < 0) {
        w_buf_clear (&buf);
        w_parse_error (p, "I/O error: $E");
    }

    w_assert (chr == '"');
    w_parse_getchar (p);
    w_parse_skip_ws (p);
    return w_buf_str (&buf);
}

