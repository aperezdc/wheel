/*
 * check-wopt.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <check.h>


START_TEST (test_wopt_set_bool)
{
    wbool value = W_NO;
    const w_opt_t options[] = {
        { 0, 'b', "bool", W_OPT_BOOL, &value, "Bool value" },
        W_OPT_END
    };
    char *argv[] = { "prog", "-b" };

    w_opt_parse (options, NULL, NULL, w_lengthof (argv), argv);
    fail_unless (value, "boolean value was not set");

    /* Parsing for second time should re-set the thing. */
    w_opt_parse (options, NULL, NULL, w_lengthof (argv), argv);
    fail_unless (value, "boolean value was not set");
}
END_TEST


