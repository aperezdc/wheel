/*!
 * \file wparse.c
 * \brief Support functions for building simple parsers.
 *
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdarg.h>
#include <ctype.h>


/*!
 * Skips whitespace in the input. All characters for which \c isspace()
 * returns true will be ignored until the first non-blank or the end of
 * the input stream is found.
 */
void
w_parse_skip_ws (w_parse_t *p)
{
    w_assert (p != NULL);

    while (isspace (p->look) && !feof (p->input))
        w_parse_getchar (p);
}


/*!
 * Gets the next character in the input, skipping over comments. If comments
 * are enabled i.e. the \ref w_parse_t::comment field is different than
 * \c 0, then when a comment character is found, the entire line is skipped.
 */
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


/*!
 * Format an error string. The formatted string will be saved in the
 * \ref w_parse_t::error field. A typical formats format is the following
 * one, including the line and column numbers:
 *
 * \code
 *    w_parse_ferror (p, "%u:%u: Your error message", p->line, p->lpos);
 * \endcode
 *
 * \param fmt Format string (printf-like).
 * \param ... Arguments for the format string.
 */
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


/*!
 * Raise a parsing error. Make sure you free intermediate strucutures and
 * data parsing may have created before calling this function, otherwise
 * memory will leak.
 *
 * \sa w_parse_ferror(), w_parse_error()
 */
void
w_parse_rerror (w_parse_t *p)
{
    w_assert (p != NULL);
    longjmp (p->jbuf, 1);
}


/*!
 * Formats an error string and raises an error.
 *
 * \sa w_parse_ferror(), w_parse_rerror()
 * \param fmt Format string (printf-like)
 * \param ... Arguments for the format string.
 */
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



/*!
 * Gets a C-like identifier. Identifiers are the same as in C: a sequence of
 * non-blank character, being the first one a letter or an underscore, and
 * the rest letters, numbers and underscores. This function never raises
 * errors, but returns \c NULL when there is some error in the input. The
 * caller is responsible for calling \ref w_free() on the returned string.
 */
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



/*!
 * Parses a aigned integer as an <tt>unsigned long</tt> value. Note that
 * prefix \c 0x will cause the number to be parsed as hexadecimal, and
 * prefix \c 0 as octal.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
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


/*!
 * Parses a aigned integer as a \c long value. Note that prefix \c 0x will
 * cause the number to be parsed as hexadecimal, and prefix \c 0 as octal.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
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


/*!
 * Parses a floating point value as \c double.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
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


/*!
 * Parses an input file.
 *
 * \param input     Input file stream.
 * \param comment   Character used as comment delimiter. When the character
 *                  is found, the remainder of the line will be ignored. To
 *                  disable this behavior, pass \c 0.
 * \param parse_fun Function used for parsing.
 * \param context   Context passed as user data to the parsing function.
 * \param msg       If not \c NULL, where to store an error string in case
 *                  an error is generated.
 * \return          Whether parsing was successful.
 */

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


/*!
 * Gets a single word from the input. A \e word here is any sequence of
 * non-whitespace characters. This function will never raise errors, but
 * returns \c NULL when the word cannot be read. The caller is responsible
 * for calling \ref w_free() on the returned string.
 */
char*
w_parse_word (w_parse_t *p)
{
    unsigned long pos = 0;
    unsigned long sz  = 32;
    char         *buf = w_alloc (char, sz);

    w_assert (p != NULL);

    while (!isspace (p->look) && !feof (p->input)) {
        buf[pos++] = p->look;
        if (pos >= sz) {
            sz += 32;
            buf = w_resize (buf, char, sz);
        }
        w_parse_getchar (p);
    }

    w_parse_getchar (p);
    w_parse_skip_ws (p);

    buf[pos] = '\0';
    return buf;
}


/*!
 * Gets a string enclosed in double-quotes from the input. Escape characters
 * in the string are interpreted, same way as the C compiler does. This
 * function never raises errors, but returns \c NULL when there is some
 * error in the input. The caller is responsible for calling \ref w_free()
 * on the returned string.
 */
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

