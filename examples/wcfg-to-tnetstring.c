/*
 * wcfg-to-tnetstring.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"


int main (int argc, char *argv[])
{
    char *errmsg = NULL;
    w_buf_t  buf = W_BUF;
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

    w_tnetstr_dump_dict (&buf, cfg);
    w_io_format (w_stdout, "$B\n", &buf);
    w_buf_free (&buf);
    w_obj_unref (cfg);
    return EXIT_SUCCESS;
}


