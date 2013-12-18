/*
 * check-wstr-conv.c
 * Copyright (C) 2010-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <check.h>
#include <limits.h>
#include "../wheel.h"

START_TEST (test_wstr_conv_bool_t)
{
    w_bool_t val;
    fail_unless (w_str_bool ("t", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_T)
{
    w_bool_t val;
    fail_unless (w_str_bool ("T", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_y)
{
    w_bool_t val;
    fail_unless (w_str_bool ("y", &val), "Could no convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_Y)
{
    w_bool_t val;
    fail_unless (w_str_bool ("Y", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_1)
{
    w_bool_t val;
    fail_unless (w_str_bool ("1", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_f)
{
    w_bool_t val;
    fail_unless (w_str_bool ("f", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_F)
{
    w_bool_t val;
    fail_unless (w_str_bool ("F", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_n)
{
    w_bool_t val;
    fail_unless (w_str_bool ("n", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_N)
{
    w_bool_t val;
    fail_unless (w_str_bool ("N", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_0)
{
    w_bool_t val;
    fail_unless (w_str_bool ("0", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_1_letter_error)
{
    w_bool_t dummy;
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
    w_bool_t val;
    fail_unless (w_str_bool ("no", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_ok)
{
    w_bool_t val;
    fail_unless (w_str_bool ("ok", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_yes)
{
    w_bool_t val;
    fail_unless (w_str_bool ("yes", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_nah)
{
    w_bool_t val;
    fail_unless (w_str_bool ("nah", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_nop)
{
    w_bool_t val;
    fail_unless (w_str_bool ("nop", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_true)
{
    w_bool_t val;
    fail_unless (w_str_bool ("true", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_yeah)
{
    w_bool_t val;
    fail_unless (w_str_bool ("yeah", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_okay)
{
    w_bool_t val;
    fail_unless (w_str_bool ("okay", &val), "Could not convert");
    fail_unless (val, "Converted value is not true");
}
END_TEST

START_TEST (test_wstr_conv_bool_nope)
{
    w_bool_t val;
    fail_unless (w_str_bool ("nope", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_false)
{
    w_bool_t val;
    fail_unless (w_str_bool ("false", &val), "Could not convert");
    fail_if (val, "Converted value is not false");
}
END_TEST

START_TEST (test_wstr_conv_bool_fail)
{
    w_bool_t dummy;
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

START_TEST (test_wstr_conv_size_bytes_zero)
{
    unsigned long long val = 42;

    fail_unless (w_str_size_bytes ("0", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0b", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0k", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0m", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0g", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0B", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0K", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0M", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_size_bytes ("0G", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;
}
END_TEST

START_TEST (test_wstr_conv_size_bytes_ok)
{
    unsigned long long val = 0;

    fail_unless (w_str_size_bytes ("16", &val), "Could not convert");
    ck_assert_int_eq (16, val);

    fail_unless (w_str_size_bytes ("42k", &val), "Could not convert");
    ck_assert_int_eq (42 * 1024, val);

    fail_unless (w_str_size_bytes ("43K", &val), "Could not convert");
    ck_assert_int_eq (43 * 1024, val);

    fail_unless (w_str_size_bytes ("32m", &val), "Could not convert");
    ck_assert_int_eq (32 * 1024 * 1024, val);

    fail_unless (w_str_size_bytes ("33M", &val), "Could not convert");
    ck_assert_int_eq (33 * 1024 * 1024, val);

    fail_unless (w_str_size_bytes ("64g", &val), "Could not convert");
    ck_assert_int_eq (64UL * 1024 * 1024 * 1024, val);

    fail_unless (w_str_size_bytes ("65G", &val), "Could not convert");
    ck_assert_int_eq (65UL * 1024 * 1024 * 1024, val);
}
END_TEST

START_TEST (test_wstr_conv_size_bytes_inval_suff)
{
    unsigned long long val = 0;

    fail_if (w_str_size_bytes ("16j", &val), "Could convert");
    ck_assert_int_eq (0, val);
}
END_TEST

START_TEST (test_wstr_conv_time_period_zero)
{
    unsigned long long val = 42;

    fail_unless (w_str_time_period ("0", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_time_period ("0s", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_time_period ("0m", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_time_period ("0d", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_time_period ("0w", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_time_period ("0M", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;

    fail_unless (w_str_time_period ("0y", &val), "Could not convert");
    ck_assert_int_eq (0, val); val = 42;
}
END_TEST

START_TEST (test_wstr_conv_time_period_ok)
{
    unsigned long long val = 0;

    fail_unless (w_str_time_period ("42", &val), "Could not convert");
    ck_assert_int_eq (42, val);

    fail_unless (w_str_time_period ("43s", &val), "Could not convert");
    ck_assert_int_eq (43, val);

    fail_unless (w_str_time_period ("23m", &val), "Could not convert");
    ck_assert_int_eq (23 * 60, val);

    fail_unless (w_str_time_period ("78m", &val), "Could not convert");
    ck_assert_int_eq (78 * 60, val);

    fail_unless (w_str_time_period ("11h", &val), "Could not convert");
    ck_assert_int_eq (11 * 60 * 60, val);

    fail_unless (w_str_time_period ("30h", &val), "Could not convert");
    ck_assert_int_eq (30 * 60 * 60, val);

    fail_unless (w_str_time_period ("12d", &val), "Could not convert");
    ck_assert_int_eq (12 * 24 * 60 * 60, val);

    fail_unless (w_str_time_period ("8M", &val), "Could not convert");
    ck_assert_int_eq (8 * 30 * 24 * 60 * 60, val);

    fail_unless (w_str_time_period ("2y", &val), "Could not convert");
    ck_assert_int_eq (2 * 365 * 24 * 60 * 60, val);
}
END_TEST

START_TEST (test_wstr_conv_time_period_inval_suff)
{
    unsigned long long val = 0;

    fail_if (w_str_time_period ("12f", &val), "Could convert");
    ck_assert_int_eq (0, val);
}
END_TEST
