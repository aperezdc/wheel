/*
 * check-wstr.c
 * Copyright (C) 2010-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <check.h>
#include "../wheel.h"


START_TEST (test_wstr_hash_neq)
{
    fail_if (w_str_hash ("foo") == w_str_hash ("bar"),
             "same hash produced for different strings");
}
END_TEST


START_TEST (test_wstr_hash_eq)
{
    fail_if (w_str_hash ("foo") != w_str_hashl ("foo", 3),
             "w_str_hashl and w_str_hash produced different hashes");
    fail_if (w_str_hash ("foo") != w_str_hashl ("foobar", 3),
             "w_str_hashl and w_str_hash produced different hashes");
}
END_TEST


START_TEST (test_wstr_dup)
{
    fail_if (strcmp ("foo", w_str_dup ("foo")),
             "copied string does not match");
}
END_TEST


START_TEST (test_wstr_ndup)
{
    fail_if (strcmp ("foo", w_str_dupl ("foobar", 3)),
             "copied string does not match");
}
END_TEST


START_TEST (test_wstr_dup_misc)
{
    fail_if (strcmp ("", w_str_dup ("")),
             "copied string is not empty");
    fail_if (strcmp ("", w_str_dupl ("foofoo", 0)),
             "copied string is not empty");
}
END_TEST


START_TEST (test_wstr_dup_null)
{
    fail_if (w_str_dup (NULL) != NULL,
             "duplicating NULL resulted in non-NULL");
    fail_if (w_str_dupl (NULL, 5) != NULL,
             "duplicating NULL resulted in non-NULL");
}
END_TEST


START_TEST (test_wstr_casecmp)
{
    fail_unless (w_str_casecmp ("foo", "foo") == 0,
                 "comparing 'foo' with itself failed");
    fail_unless (w_str_casecmp ("FOO", "foo") == 0,
                 "comparing 'FOO' with 'foo' failed");
    fail_unless (w_str_casecmp ("F o", "f O") == 0,
                 "comparing 'F o' with 'f O' failed");
    fail_unless (w_str_casecmp ("0.¬", "0.¬") == 0,
                 "comparing '0.¬' with itself failed");
}
END_TEST


START_TEST (test_wstr_casecmp_misc)
{
    fail_unless (w_str_casecmp ("", "") == 0,
                 "comparing an empty string with itself failed");
}
END_TEST


START_TEST (test_wstr_cpy)
{
    char copy[10];
    w_strncpy (copy, "foobar", 3);
    fail_unless (strcmp ("foo", copy) == 0,
                 "copied string is not 'foo'");
}
END_TEST
