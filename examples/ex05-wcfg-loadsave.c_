/*
 * ex05-wcfg-loadsave.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"

int
main (int argc, char **argv)
{
    w_cfg_t *cf;
    char *msg;

    w_unused (argc);
    w_unused (argv);

    if (argc < 2) {
        fprintf (stderr, "usage: %s file.conf\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ((cf = w_cfg_load_file (argv[1], &msg)) == NULL) {
        fprintf (stderr, "stdin: %s\n", (msg) ? msg : "(unknown error)");
        if (msg) w_free (msg);
    }
    else {
        w_cfg_dump (cf, stdout);
    }
    w_cfg_free (cf);

    return EXIT_SUCCESS;
}


