/*
 * wopt-basic.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <string.h>
#include <stdlib.h>


static wbool verbose = W_NO;

static const w_opt_t option_spec[] = {
    { 0, 'v', "verbose", W_OPT_BOOL, &verbose,
      "Activate super-verbose behaviour" },

    W_OPT_END
};


static void
file_arg_cb (void *filename, void *ctx)
{
    w_assert (filename != NULL);
    w_unused (ctx);

    if (verbose) {
        w_io_format (w_stdout, "File name: $s\n", filename);
    }
}


int
main (int argc, char **argv)
{
    w_opt_parse (option_spec, file_arg_cb, NULL, "<filename>", argc, argv);

    return EXIT_SUCCESS;
}


