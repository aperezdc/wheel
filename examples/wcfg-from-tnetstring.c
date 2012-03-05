/*
 * wcfg-from-tnetstring.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"


int
main (int argc, char *argv[])
{
    w_cfg_t *cfg = w_cfg_new ();
    w_buf_t buf = W_BUF;
    int ch;

    w_unused (argc);
    w_unused (argv);

    while ((ch = w_io_getchar (w_stdin)) != W_IO_EOF)
        w_buf_append_char (&buf, ch);

    if (w_tnetstr_parse_dict (&buf, cfg))
        w_die ("Could not parse input tnetstring");

    w_buf_free (&buf);
    w_cfg_dump (cfg, w_stdout);
    w_obj_unref (cfg);

    return EXIT_SUCCESS;
}

