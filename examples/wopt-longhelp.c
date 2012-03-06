/*
 * wopt-longhelp.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>

static w_bool_t dofoo = W_NO;

static const w_opt_t options[] = {
    { 0, 'l', "long-option", W_OPT_BOOL, &dofoo,
        "This is a long option with a very long description, whose "
        "purpose is to test how well does line wraping for the help "
        "text of command line options works. Also, this will test "
        "how subsequent lines are properly indented and aligned." },
    W_OPT_END
};


int main (int argc, char **argv)
{
    w_unused (argc);
    w_unused (argv);

    w_opt_help (options, w_stdout, argv[0], NULL);

    return EXIT_SUCCESS;
}
