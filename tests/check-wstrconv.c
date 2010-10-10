/*
 * check-wstr-conv.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <check.h>
#include "wheel.h"

START_TEST (test_wstr_conv_bool_t)
{
    wbool val;
    fail_unless (w_str_bool ("t", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_T)
{
    wbool val;
    fail_unless (w_str_bool ("T", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_y)
{
    wbool val;
    fail_unless (w_str_bool ("y", &val), "Could no convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_Y)
{
    wbool val;
    fail_unless (w_str_bool ("Y", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_1)
{
    wbool val;
    fail_unless (w_str_bool ("1", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_f)
{
    wbool val;
    fail_unless (w_str_bool ("f", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_F)
{
    wbool val;
    fail_unless (w_str_bool ("F", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_n)
{
    wbool val;
    fail_unless (w_str_bool ("n", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_N)
{
    wbool val;
    fail_unless (w_str_bool ("N", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_0)
{
    wbool val;
    fail_unless (w_str_bool ("0", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_1_letter_error)
{
    wbool dummy;
    fail_if (w_str_bool ("g", &dummy), "Could convert invalid value");
    fail_if (w_str_bool ("6", &dummy), "Could convert invalid value");
    fail_if (w_str_bool ("$", &dummy), "Could convert invalid value");
    fail_if (w_str_bool ("_", &dummy), "Could convert invalid value");
    fail_if (w_str_bool ("'", &dummy), "Could convert invalid value");
    fail_if (w_str_bool (".", &dummy), "Could convert invalid value");
    fail_if (w_str_bool ("R", &dummy), "Could convert invalid value");
}
END_TEST

START_TEST (test_wstr_conv_bool_no)
{
    wbool val;
    fail_unless (w_str_bool ("no", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_ok)
{
    wbool val;
    fail_unless (w_str_bool ("ok", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_yes)
{
    wbool val;
    fail_unless (w_str_bool ("yes", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_nah)
{
    wbool val;
    fail_unless (w_str_bool ("nah", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_nop)
{
    wbool val;
    fail_unless (w_str_bool ("nop", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_true)
{
    wbool val;
    fail_unless (w_str_bool ("true", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_yeah)
{
    wbool val;
    fail_unless (w_str_bool ("yeah", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_okay)
{
    wbool val;
    fail_unless (w_str_bool ("okay", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_nope)
{
    wbool val;
    fail_unless (w_str_bool ("nope", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_false)
{
    wbool val;
    fail_unless (w_str_bool ("false", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_fail)
{
    wbool dummy;
    fail_if (w_str_bool ("", &dummy), "Could convert ''");
    fail_if (w_str_bool (" ", &dummy), "Could convert ' '");
    fail_if (w_str_bool ("foo", &dummy), "Could convert 'foo'");
}
END_TEST

