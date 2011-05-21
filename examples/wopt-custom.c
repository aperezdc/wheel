/*
 * wopt-custom.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>


/*
 * Internally, store time always in seconds.
 */
static unsigned long seconds = 0;


/*
 * This is a custom command line argument parsing callback, which picks
 * a number with a suffix which multiplies it (e.g.: 5w == 5 (w)eeks),
 * and stores the corresponding seconds value.
 */
static w_opt_status_t
parse_time_argument (const w_opt_context_t *ctx)
{
    unsigned long val;
    char *endpos;

    w_assert (ctx != NULL);
    w_assert (ctx->option != NULL);
    w_assert (ctx->option->narg == 1);
    w_assert (ctx->option->extra != NULL);

    val = strtoul (ctx->argument[0], &endpos, 0);

    /* Returning W_OPT_BAD_ARG means that there was some error */
    if (val == ULONG_MAX && errno == ERANGE)
        return W_OPT_BAD_ARG;

    if (!endpos || *endpos == '\0')
        goto save_and_exit;

    switch (*endpos) {
        case 'y': val *= 60 * 60 * 24 * 365; break;
        case 'M': val *= 60 * 60 * 24 * 30; break;
        case 'w': val *= 60 * 60 * 24 * 7; break;
        case 'd': val *= 60 * 60 * 24; break;
        case 'h': val *= 60 * 60; break;
        case 'm': val *= 60; break;
        default: return W_OPT_BAD_ARG;
    }

save_and_exit:
    *((unsigned*) ctx->option->extra) = val;
    /* Returning this signals the argument as correctly parsed */
    return W_OPT_OK;
}


static const w_opt_t option_spec[] = {
    /* Just an option, with our custom parser added */
    { 1, 't', "time", parse_time_argument, &seconds, "Time value" },
    W_OPT_END
};


int
main (int argc, char **argv)
{
    w_opt_parse (option_spec, NULL, NULL, NULL, argc, argv);
    w_io_format (w_stdout, "Time value, in seconds: $L\n", seconds);

    return EXIT_SUCCESS;
}


