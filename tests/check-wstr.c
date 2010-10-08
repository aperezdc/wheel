/*
 * check-wstr.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <check.h>
#include "wheel.h"


START_TEST (test_wstr_hash_neq) {
    fail_if (w_hashstr ("foo") == w_hashstr ("bar"),
             "same hash produced for different strings");
} END_TEST

START_TEST (test_wstr_hash_eq) {
    fail_if (w_hashstr ("foo") != w_hashstrn ("foo", 3),
             "w_hashstrn and w_hashstr produced different hashes");
} END_TEST

