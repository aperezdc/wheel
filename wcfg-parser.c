/*
 * wcfg-parser.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wcfg-parser.h"
#include "wstr.h"
#include "wmem.h"
#include <stdarg.h>
#include <ctype.h>


#define W_CFG_PARSER_ERROR(_s)   longjmp ((_s)->jbuf, 1)


void
w_cfg_parser_skip_white (w_cfg_parser_t *self)
{
    w_assert (self != NULL);
    while (isspace (self->look))
        W_CFG_PARSER_GETCHAR ();
}


void
w_cfg_parser_error (w_cfg_parser_t *self,
                    const char     *fmt,
                    ...)
{
    va_list args;

    w_assert (self != NULL);

    va_start (args, fmt);
    self->err = w_strfmtv (fmt, args);
    va_end (args);

    W_CFG_PARSER_ERROR (self);
}



char*
w_cfg_parser_get_id (w_cfg_parser_t *self)
{
    char *buf;
    unsigned long pos = 0;
    unsigned long sz  = 50;

    w_assert (self != NULL);

    if (!isalpha (self->look) && self->look != '_') {
        w_cfg_parser_error (self, "%u:%u: identifier expected",
                            self->line, self->lpos);
    }

    buf = w_alloc (char, sz);
    while (isalnum (self->look) || self->look == '_') {
        buf[pos++] = self->look;
        if (pos >= sz) {
            sz += 16;
            buf = w_resize (buf, char, sz);
        }
        W_CFG_PARSER_GETCHAR ();
    }

    w_cfg_parser_skip_white (self);

    buf[pos] = '\0';
    return buf;
}



double
w_cfg_parser_get_number (w_cfg_parser_t *self,
                         void           *fof)
{
    const char *err;
    double val;
    unsigned uval;

    w_assert (self != NULL);

    if (self->look == '0') {
        W_CFG_PARSER_GETCHAR ();
        if (self->look == 'x' || self->look == 'X') {
            if (!fscanf (self->input, "%x", &uval)) {
                err = "%u:%u: hexadecimal number expected";
                goto error_out;
            }
        }
        else if (isdigit (self->look)) {
            ungetc (self->look, self->input);
            if (!fscanf (self->input, "%o", &uval)) {
                err = "%u:%u: octal number expected";
                goto error_out;
            }
        }
        else {
            ungetc (self->look, self->input);
            uval = 0;
        }
        val = (double) uval;
    }
    else {
        ungetc (self->look, self->input);
        if (!fscanf (self->input, "%lf", &val)) {
            err = "%u:%u: numeric value expected";
            goto error_out;
        }
    }

    W_CFG_PARSER_GETCHAR ();
    w_cfg_parser_skip_white (self);
    return val;

error_out:
    if (fof != NULL) w_free (fof);
    w_cfg_parser_error (self, err, self->line, self->lpos);
    return 0.0; /* Make dodgy compilers happy */
}



char*
w_cfg_parser_parse (w_cfg_parser_t *self)
{
    w_assert (self != NULL);

    if (setjmp (self->jbuf)) {
        /* Do cleanup */
        w_cfg_free (self->result);
        return self->err;
    }
    else {
        /* Put parser in sane state */
        W_CFG_PARSER_GETCHAR ();
        w_cfg_parser_skip_white (self);

        /* Do real parsing here */
        self->result = w_cfg_new ();
        w_cfg_parser_get_cfg (self, self->result);
        return NULL;
    }
}



void
w_cfg_parser_init (w_cfg_parser_t *self,
                   FILE           *input)
{
    w_assert (self != NULL);
    w_assert (input != NULL);

    /* Initialize parser structure */
    memset (self, 0x00, sizeof (w_cfg_parser_t));
    self->input = input;
    self->line  = 1;
}


void
w_cfg_parser_parse_items (w_cfg_parser_t *self,
                          w_cfg_t        *cfg)
{
    char *sval;
    double dval;
    w_cfg_t *cval;

    w_assert (self != NULL);
    w_assert (cfg != NULL);

    while (self->look != '}') {
        /* Parse item, get identifier */
        char *ident = w_cfg_parser_get_id (self);
        W_CFG_PARSER_MATCH_I ('=', w_free (ident));
        switch (self->look) {
            case '{': /* subnode */
                /*
                 * Insert subnode before trying to parse: that way the
                 * cleanup code will w_free() without us having to worry
                 */
                cval = w_cfg_new ();
                w_cfg_set_node (cfg, ident, cval);
                w_cfg_parser_get_cfg (self, cval);
                break;
            case '"': /* string */
                sval = w_cfg_parser_get_string (self);
                W_CFG_PARSER_MATCH_I (';', (w_free (sval), w_free (ident)));
                w_cfg_set_string (cfg, ident, sval);
                w_free (sval);
                break;
            default: /* number */
                dval = w_cfg_parser_get_number (self, ident);
                W_CFG_PARSER_MATCH_I (';', w_free (ident));
                w_cfg_set_number (cfg, ident, dval);
        }
        w_free (ident);
    }
}



void
w_cfg_parser_get_cfg (w_cfg_parser_t *self,
                      w_cfg_t        *cfg)
{
    w_assert (self != NULL);
    w_assert (cfg != NULL);

    W_CFG_PARSER_MATCH ('{');
    w_cfg_parser_parse_items (self, cfg);
    W_CFG_PARSER_MATCH ('}');
    W_CFG_PARSER_MATCH (';');
}


char*
w_cfg_parser_get_string (w_cfg_parser_t *self)
{
    unsigned long pos = 0;
    unsigned long sz  = 100;
    char         *buf = w_alloc (char, sz);
    int           chr = fgetc (self->input);

    w_assert (self != NULL);

    for (; chr != '"'; chr = fgetc (self->input)) {
        if (chr == '\\') {
            /* escaped sequences */
            switch ((chr = fgetc (self->input))) {
                case 'n': chr = '\n'; break; /* carriage return */
                case 'r': chr = '\r'; break; /* line feed */
                case 'b': chr = '\b'; break; /* backspace */
                case 'e': chr = 0x1b; break; /* escape */
                case 'a': chr = '\a'; break; /* bell */
                case 't': chr = '\t'; break; /* tab */
                case 'v': chr = '\v'; break; /* vertical tab */
                case 'X': /* hex number */
                case 'x': { char num[3];
                            num[0] = fgetc (self->input);
                            num[1] = fgetc (self->input);
                            num[2] = '\0';
                            if (!isxdigit (num[0]) || !isxdigit (num[1])) {
                                /* XXX Would making 2 ungetc() calls work? */
                                ungetc (num[0], self->input);
                                ungetc (num[1], self->input);
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

    W_CFG_PARSER_GETCHAR ();
    w_cfg_parser_skip_white (self);

    buf[pos] = '\0';
    return buf;
}
