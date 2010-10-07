/*
 * ex04-wcfg-parser-dump.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include "wheel-private.h"
#include <stdio.h>
#include <stdlib.h>


int
main (int argc, char **argv)
{
    w_cfg_parser_t parser;
    char *msg;

    w_unused (argc);
    w_unused (argv);

    w_cfg_parser_init (&parser, stdin);
    msg = w_cfg_parser_parse (&parser);

    if (msg != NULL) {
        fprintf (stderr, "<stdin>:%s\n", msg);
        w_free (msg);
        return EXIT_SUCCESS;
    }

    w_cfg_dump (parser.result, stdout);
    w_cfg_free (parser.result);

    return EXIT_SUCCESS;
}



