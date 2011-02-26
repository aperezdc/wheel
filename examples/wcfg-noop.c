/*
 * wcfg-noop.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"


int
main (int argc, char **argv)
{
    char    *err = NULL;
    w_cfg_t *cfg;

    if (argc != 2)
        w_die ("usage: %s <conf-file>\n", argv[0]);

    if ((cfg = w_cfg_load_file (argv[1], &err)) == NULL) {
        w_assert (err);
        w_die ("%s:%s\n", argv[1], err);
    }

    w_free (err);

    w_cfg_dump (cfg, w_io_stdio_open (stdout));

    return EXIT_SUCCESS;
}


