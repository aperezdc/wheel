/*
 * wparse.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
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

    while (isspace (p->look) && !feof (p->input))
        w_parse_getchar (p);
}


void
w_parse_getchar (w_parse_t *p)
{
    w_assert (p != NULL);

    do {
        p->look = fgetc (p->input);

        if (p->look == '\n') {
            p->lpos = 0;
            p->line++;
        }
        p->lpos++;

        if (p->comment && p->look == p->comment) {
            while (fgetc (p->input) != '\n')
                if (feof (p->input))
                    p->look = EOF;
            ungetc ('\n', p->input);
        }
    } while (p->look != EOF && p->comment && p->look == p->comment);
}


void
w_parse_ferror (w_parse_t  *p,
                const char *fmt,
                ...)
{
    va_list args;

    w_assert (p != NULL);
    w_assert (fmt != NULL);

    va_start (args, fmt);
    p->error = w_strfmtv (fmt, args);
    va_end (args);
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
    va_list args;

    w_assert (p != NULL);
    w_assert (fmt != NULL);

    va_start (args, fmt);
    p->error = w_strfmtv (fmt, args);
    va_end (args);

    w_parse_rerror (p);
}



char*
w_parse_ident (w_parse_t *p)
{
    char *buf;
    unsigned long pos = 0;
    unsigned long sz  = 50;

    w_assert (p != NULL);

    if (!isalpha (p->look) && p->look != '_')
        return NULL;

    buf = w_alloc (char, sz);
    while (isalnum (p->look) || p->look == '_') {
        buf[pos++] = p->look;
        if (pos >= sz) {
            sz += 16;
            buf = w_resize (buf, char, sz);
        }
        w_parse_getchar (p);
    }

    w_parse_skip_ws (p);
    buf[pos] = '\0';
    return buf;
}



wbool
w_parse_ulong (w_parse_t      *p,
               unsigned long  *value)
{
    w_assert (p != NULL);
    w_assert (value != NULL);

    if (p->look == '0') {
        w_parse_getchar (p);
        if (p->look == 'x' || p->look == 'X') {
            if (!fscanf (p->input, "%lx", value))
                return W_NO;
        }
        else if (isdigit (p->look)) {
            ungetc (p->look, p->input);
            if (!fscanf (p->input, "%lo", value))
                return W_NO;
        }
        else {
            ungetc (p->look, p->input);
            *value = 0;
        }
    }
    else {
        ungetc (p->look, p->input);
        if (!fscanf (p->input, "%lu", value))
            return W_NO;
    }

    w_parse_getchar (p);
    w_parse_skip_ws (p);
    return W_YES;
}


wbool
w_parse_long (w_parse_t *p, long *value)
{
    unsigned long uval;

    w_assert (p != NULL);
    w_assert (value != NULL);

    if (p->look == '0') {
        w_parse_getchar (p);
        if (p->look == 'x' || p->look == 'X') {
            if (!fscanf (p->input, "%lx", &uval))
                return W_NO;
            *value = uval;
        }
        else if (isdigit (p->look)) {
            ungetc (p->look, p->input);
            if (!fscanf (p->input, "%lo", &uval))
                return W_NO;
            *value = uval;
        }
        else {
            ungetc (p->look, p->input);
            *value = 0;
        }
    }
    else {
        ungetc (p->look, p->input);
        if (!fscanf (p->input, "%li", value))
            return W_NO;
    }

    w_parse_getchar (p);
    w_parse_skip_ws (p);
    return W_YES;
}


wbool
w_parse_double (w_parse_t *p, double *value)
{
    w_assert (p != NULL);
    w_assert (value != NULL);

    ungetc (p->look, p->input);
    if (!fscanf (p->input, "%lf", value))
        return W_NO;

    w_parse_getchar (p);
    w_parse_skip_ws (p);

    return W_YES;
}


void*
w_parse_run (w_parse_t    *p,
             FILE         *input,
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
    return p->result;
}


char*
w_parse_string (w_parse_t *p)
{
    unsigned long pos = 0;
    unsigned long sz  = 100;
    char         *buf = w_alloc (char, sz);
    int           chr = fgetc (p->input);

    w_assert (p != NULL);

    for (; chr != '"' && !feof (p->input); chr = fgetc (p->input)) {
        if (chr == '\\') {
            /* escaped sequences */
            switch ((chr = fgetc (p->input))) {
                case 'n': chr = '\n'; break; /* carriage return */
                case 'r': chr = '\r'; break; /* line feed */
                case 'b': chr = '\b'; break; /* backspace */
                case 'e': chr = 0x1b; break; /* escape */
                case 'a': chr = '\a'; break; /* bell */
                case 't': chr = '\t'; break; /* tab */
                case 'v': chr = '\v'; break; /* vertical tab */
                case 'X': /* hex number */
                case 'x': { char num[3];
                            num[0] = fgetc (p->input);
                            num[1] = fgetc (p->input);
                            num[2] = '\0';
                            if (!isxdigit (num[0]) || !isxdigit (num[1])) {
                                /* XXX Would making 2 ungetc() calls work? */
                                ungetc (num[0], p->input);
                                ungetc (num[1], p->input);
                                continue;
                            }
                            chr = strtol (num, NULL, 16);
                          } break;
            }
        }
        buf[pos++] = chr;
        if (pos >= sz) {
            sz += 50;
            buf = w_resize (buf, char, sz);
        }
    }

    w_parse_getchar (p);
    w_parse_skip_ws (p);

    buf[pos] = '\0';
    return buf;
}

