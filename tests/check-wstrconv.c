/*
 * check-wstr-conv.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <check.h>
#include <limits.h>
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


START_TEST (test_wstr_conv_int_0)
{
    int val;
    fail_unless (w_str_int ("0", &val), "Could not convert");
    fail_unless (val == 0, "Converted value is not 0");
}
END_TEST

START_TEST (test_wstr_conv_int_negative)
{
    int val;
    fail_unless (w_str_int ("-42", &val), "Could not converted");
    fail_unless (val == -42, "Converted value is not -42");
}
END_TEST

START_TEST (test_wstr_conv_int_positive)
{
    int val;
    fail_unless (w_str_int ("42", &val), "Could not converted");
    fail_unless (val == 42, "Converted value is not -42");
}
END_TEST

START_TEST (test_wstr_conv_int_fail)
{
    int val;
    fail_if (w_str_int ("", &val), "Could convert");
    fail_if (w_str_int (" ", &val), "Could convert");
    fail_if (w_str_int ("a", &val), "Could convert");
    fail_if (w_str_int ("21f", &val), "Could convert");
    fail_if (w_str_int ("f12", &val), "Could convert");
}
END_TEST

START_TEST (test_wstr_conv_int_huge)
{
    int val;
    fail_if (w_str_int ("123456789012345678901234567890", &val),
             "Could convert");
    fail_if (w_str_int ("-123456789012345678901234567890", &val),
             "Could convert");
}
END_TEST

START_TEST (test_wstr_conv_int_max)
{
    char buf[200];
    int val;
    sprintf (buf, "%i", INT_MAX);
    fail_unless (w_str_int (buf, &val), "Could not convert");
    fail_unless (val == INT_MAX, "Convertted value is not INT_MAX");
}
END_TEST

START_TEST (test_wstr_conv_int_min)
{
    char buf[200];
    int val;
    sprintf (buf, "%i", INT_MIN);
    fail_unless (w_str_int (buf, &val), "Could not convert");
    fail_unless (val == INT_MIN, "Converted value is not INT_MIN");
}
END_TEST

START_TEST (test_wstr_conv_int_min_minusone)
{
    char buf[200];
    int val;
    sprintf (buf, "%li", (long) INT_MIN - 1);
    fail_if (w_str_int (buf, &val), "Could convert");
}
END_TEST

START_TEST (test_wstr_conv_int_max_plusone)
{
    char buf[200];
    int val;
    sprintf (buf, "%li", (long) INT_MAX + 1);
    fail_if (w_str_int (buf, &val), "Could convert");
}
END_TEST

