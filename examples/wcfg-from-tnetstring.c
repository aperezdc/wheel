/*
 * wcfg-from-tnetstring.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"


int
main (int argc, char *argv[])
{
    w_cfg_t *cfg = w_cfg_new ();

    w_unused (argc);
    w_unused (argv);

    if (w_tnetstr_read_dict (w_stdin, cfg))
        w_die ("Could not parse input tnetstring");

    w_cfg_dump (cfg, w_stdout);
    w_obj_unref (cfg);

    return EXIT_SUCCESS;
}

