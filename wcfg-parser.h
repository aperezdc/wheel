/*
 * wcfg-parser.h
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wcfg_parse_h__
#define __wcfg_parse_h__

#include "wdef.h"
#include "wcfg.h"
#include <stdio.h>
#include <setjmp.h>



/*
 * Parser type
 */
struct w_cfg_parser_s
{
    unsigned line;   /* current line number      */
    unsigned lpos;   /* position in current line */
    int      look;   /* lookahead character      */
    FILE    *input;  /* input stream             */
    char    *err;    /* last error, or NULL      */
    jmp_buf  jbuf;   /* jump buffer              */
    w_cfg_t *result; /* parsed result            */
};
typedef struct w_cfg_parser_s w_cfg_parser_t;



/*
 * Verifies whether a expected character is matched in the input. Second
 * argument is some cleanup statement executed just before calling the
 * w_cfg_p_error function.
 */
#define W_CFG_PARSER_MATCH_I(_c, _stmt)                           \
    do {                                                          \
        if ((_c) == self->look) {                                 \
            W_CFG_PARSER_GETCHAR ();                              \
            w_cfg_parser_skip_white (self);                       \
        }                                                         \
        else {                                                    \
            _stmt;                                                \
            w_cfg_parser_error (self,                             \
                                "%u:%u: character '%c' expected", \
                                self->line, self->lpos, (_c));    \
        }                                                         \
    } while (0)


/*
 * Same as the above, but with this one without cleanup statetement.
 */
#define W_CFG_PARSER_MATCH(__c) \
        W_CFG_PARSER_MATCH_I((__c), (void)0)


/*
 * Gets the next character from the input streamm stripping comments.
 */
#define W_CFG_PARSER_GETCHAR( )                     \
    do {                                            \
        self->look = fgetc (self->input);           \
        if (self->look == '\n') {                   \
            self->lpos = 0;                         \
            self->line++;                           \
        }                                           \
        self->lpos++;                               \
        if (self->look == '-') {                    \
            int __tmp = fgetc (self->input);        \
            if (__tmp == '-') {                     \
                while (fgetc (self->input) != '\n') \
                    if (feof (self->input))         \
                        self->look = EOF;           \
            }                                       \
            else {                                  \
                ungetc (__tmp, self->input);        \
                break;                              \
            }                                       \
        }                                           \
    } while (self->look == '-')



/**
 * Initializes the parser to read from the given file handle.
 */
void   w_cfg_parser_init        (w_cfg_parser_t *self,
                                 FILE           *input);

char*  w_cfg_parser_parse       (w_cfg_parser_t *self);

/**
 * Parses "key=value" pairs of a node. Stores parsed items in the given
 * configuration object.
 */
void   w_cfg_parser_parse_items (w_cfg_parser_t *self,
                                 w_cfg_t        *cfg);

void   w_cfg_parser_error       (w_cfg_parser_t *self,
                                 const char     *fmt,
                                 ...);

void   w_cfg_parser_skip_white  (w_cfg_parser_t *self);

/**
 * Reads an identifier. Identifiers are defined as in C: first character
 * must be either a letter or an underscore, following characters must be
 * letters, digits or underscores. There is no limit in the length of an
 * identifier. You must w_free() the returned string.
 */
char*  w_cfg_parser_get_id      (w_cfg_parser_t *self);

/**
 * Parses a string with escape sequences that may contain arbitrary data
 * encoded as hex numbers using the \xNN or \XNN escape sequences.
 */
char  *w_cfg_parser_get_string  (w_cfg_parser_t *self);

/**
 * Reads a number. Four formats are recognized:
 *
 *   - Floats, scientific or dotted format ("%lf")
 *   - Integers in hex format (trailing "0x")
 *   - Integers in octal format (trailing "0")
 *   - Integers.
 */
double w_cfg_parser_get_number  (w_cfg_parser_t *self,
                                 void           *fof);

/**
 * Parses a subnode, storing parsed items in the given configuration object.
 */
void   w_cfg_parser_get_cfg     (w_cfg_parser_t *self,
                                 w_cfg_t        *cfg);



#endif /* !__wcfg_parse_h__ */

