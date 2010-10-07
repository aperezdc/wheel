/*
 * ex06-wopt-loadfile.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>
#include <unistd.h>


static wbool verbose   = W_NO;
static int   sleeptime = 5;


static const w_opt_t option_spec[] = {
    { 0, 'v', "verbose", W_OPT_BOOL, &verbose,
      "Activate verbose operation" },
    { 1, 's', "sleep", W_OPT_INT, &sleeptime,
      "Time to sleep before exiting" },
    W_OPT_END
};


int
main (int argc, const char **argv)
{
    char *errmsg = NULL;

    w_opt_parse_file (option_spec, stdin, &errmsg);
    if (errmsg != NULL) {
        fprintf (stderr, "stdin:%s\n", errmsg);
        w_free (errmsg);
        return EXIT_FAILURE;
    }

    w_opt_parse (option_spec, NULL, NULL, argc, argv);

    if (verbose) {
        printf ("sleeping %i seconds\n", sleeptime);
    }
    sleep (sleeptime);

    return EXIT_SUCCESS;
}

