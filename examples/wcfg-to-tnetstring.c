/*
 * wcfg-to-tnetstring.c
 * Copyright (C) 2012-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"


int main (int argc, char *argv[])
{
    char *errmsg = NULL;
    w_cfg_t *cfg = w_cfg_load (w_stdin, &errmsg);

    w_unused (argc);
    w_unused (argv);

    if (!cfg) {
        if (errmsg) {
            w_die ("Error loading config: $s\n", errmsg);
            w_free (errmsg);
        }
        else
            w_die ("Unknown error loading config\n");
    }

    w_tnetstr_write_dict (w_stdout, cfg);
    w_obj_unref (cfg);
    return EXIT_SUCCESS;
}


