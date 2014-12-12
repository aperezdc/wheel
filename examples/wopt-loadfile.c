/*
 * wopt-loadfile.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <unistd.h>


static w_bool_t verbose   = W_NO;
static int      sleeptime = 5;


static const w_opt_t option_spec[] = {
    { 0, 'v', "verbose", W_OPT_BOOL, &verbose,
      "Activate verbose operation" },
    { 1, 's', "sleep", W_OPT_INT, &sleeptime,
      "Time to sleep before exiting" },
    W_OPT_END
};


int
main (int argc, char **argv)
{
    char *errmsg = NULL;

    w_opt_parse_io (option_spec, w_stdin, &errmsg);
    if (errmsg != NULL) {
        w_io_format (w_stderr, "<stdin>:$s\n", errmsg);
        w_free (errmsg);
        return EXIT_FAILURE;
    }

    w_opt_parse (option_spec, NULL, NULL, NULL, argc, argv);

    if (verbose) {
        printf ("sleeping %i seconds\n", sleeptime);
    }
    sleep (sleeptime);

    return EXIT_SUCCESS;
}

