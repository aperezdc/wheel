/*
 * check-wopt.c
 * Copyright (C) 2010-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <check.h>


START_TEST (test_wopt_set_bool)
{
    bool value = false;
    const w_opt_t options[] = {
        { 0, 'b', "bool", W_OPT_BOOL, &value, "Bool value" },
        W_OPT_END
    };
    char *argv[] = { "prog", "-b" };

    w_opt_parse (options, NULL, NULL, NULL, w_lengthof (argv), argv);
    fail_unless (value, "boolean value was not set");

    /* Parsing for second time should re-set the thing. */
    w_opt_parse (options, NULL, NULL, NULL, w_lengthof (argv), argv);
    fail_unless (value, "boolean value was not set");
}
END_TEST


START_TEST (test_wopt_set_int)
{
    int value;
    const w_opt_t options[] = {
        { 1, 'i', "int", W_OPT_INT, &value, "Int value" },
        W_OPT_END
    };
    static char *argv1[] = { "prog", "-i", "42" };
    static char *argv2[] = { "prog", "-i", "-42" };
    static char *argv3[] = { "prog", "-i", "0x42" };

    w_opt_parse (options, NULL, NULL, NULL, w_lengthof (argv1), argv1);
    fail_unless (value == 42, "expected 42, got %i", value);

    w_opt_parse (options, NULL, NULL, NULL, w_lengthof (argv2), argv2);
    fail_unless (value == -42, "expected -42, got %i", value);

    w_opt_parse (options, NULL, NULL, NULL, w_lengthof (argv3), argv3);
    fail_unless (value == 0x42, "expected 0x42, got %#x", value);
}
END_TEST


