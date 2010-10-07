/*
 * ex03-wcfg-parser.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include "wheel-private.h"
#include <stdlib.h>
#include <stdio.h>


int
main (int argc, char **argv)
{
    w_cfg_parser_t parser;
    char *msg;

    (void) argc;
    (void) argv;

    w_cfg_parser_init (&parser, stdin);
    msg = w_cfg_parser_parse (&parser);

    if (msg != NULL) {
        fprintf (stderr, "<stdin>:%s\n", msg);
        w_free (msg);
        exit (EXIT_FAILURE);
    }
    else {
        w_cfg_free (parser.result);
    }

    exit (EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

