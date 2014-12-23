/*
 * check-wdict.c
 * Copyright (C) 2010-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <check.h>
#include <string.h>
#include "../wheel.h"


START_TEST (test_wdict_get_set)
{
    w_dict_t *d = w_dict_new (false);

    w_dict_set (d, "foo", "FOO");
    fail_unless (w_dict_size (d) == 1,
                 "Expected one item in dict");
    fail_if (strcmp ("FOO", w_dict_get (d, "foo")),
             "String 'FOO' does not match");

    w_dict_set (d, "bar", "BAR");
    fail_unless (w_dict_size (d) == 2,
                 "Expected two items in dict");
    fail_if (strcmp ("BAR", w_dict_get (d, "bar")),
             "String 'BAR' does not match");

    w_obj_unref (d);
}
END_TEST


START_TEST (test_wdict_getn)
{
    w_dict_t *d = w_dict_new (false);

    w_dict_set (d, "foo", "FOO");
    fail_if (strcmp ("FOO", w_dict_get (d, "foo")),
             "String 'FOO' does not match (get)");
    fail_if (strcmp ("FOO", w_dict_getn (d, "foobar", 3)),
             "String 'FOO' does not match (getn)");
    fail_if (strcmp (w_dict_get (d, "foo"), w_dict_getn (d, "foobar", 3)),
             "String 'FOO' does not match (get/getn)");

    w_obj_unref (d);
}
END_TEST


START_TEST (test_wdict_setn)
{
    w_dict_t *d = w_dict_new (false);

    w_dict_setn (d, "foobar", 3, "FOO");
    fail_unless (w_dict_size (d) == 1,
                 "Expected one item in dict");
    fail_if (strcmp ("FOO", w_dict_get (d, "foo")),
             "String 'FOO' does not match");

    w_obj_unref (d);
}
END_TEST


START_TEST (test_wdict_clear)
{
    w_dict_t *d = w_dict_new (false);

    w_dict_set (d, "no.1", (void*) 1);
    w_dict_set (d, "no.2", (void*) 2);
    w_dict_set (d, "no.3", (void*) 3);
    w_dict_set (d, "no.4", (void*) 4);

    fail_unless (w_dict_size (d) == 4,
                 "Expected 4 items in dict");

    w_dict_clear (d);

    fail_unless (w_dict_size (d) == 0,
                 "Expected 0 items in dict");

    w_obj_unref (d);
}
END_TEST


START_TEST (test_wdict_del)
{
    w_dict_t *d = w_dict_new (false);

    w_dict_set (d, "no.1", (void*) 1);
    w_dict_set (d, "no.2", (void*) 2);
    w_dict_set (d, "no.3", (void*) 3);
    w_dict_set (d, "no.4", (void*) 4);

    fail_unless (w_dict_size (d) == 4,
                 "Expected 4 items in dict");

    w_dict_del (d, "no.2");

    fail_unless (w_dict_size (d) == 3,
                 "Expected 3 items in dict");

    w_dict_del (d, "no.2");

    fail_unless (w_dict_size (d) == 3,
                 "Expected 3 items in dict");

    w_dict_del (d, "no.4");

    fail_unless (w_dict_size (d) == 2,
                 "Expected 2 items in dict");

    w_obj_unref (d);
}
END_TEST


START_TEST (test_wdict_deln)
{
    w_dict_t *d = w_dict_new (false);

    w_dict_set (d, "no.1", (void*) 1);
    w_dict_set (d, "no.2", (void*) 2);
    w_dict_set (d, "no.3", (void*) 3);
    w_dict_set (d, "no.4", (void*) 4);

    fail_unless (w_dict_size (d) == 4,
                 "Expected 4 items in dict");

    w_dict_deln (d, "no.2342432", 4);

    fail_unless (w_dict_size (d) == 3,
                 "Expected 3 items in dict");

    w_obj_unref (d);
}
END_TEST


START_TEST (test_wdict_update)
{
    w_dict_t *d1 = w_dict_new (false);
    w_dict_t *d2 = w_dict_new (false);

    w_dict_set (d1, "foo", "FOO");
    w_dict_set (d2, "bar", "BAR");
    w_dict_set (d1, "baz", "BAZ");
    w_dict_set (d2, "baz", "BAZINGA");

    w_dict_update (d1, d2);

    fail_unless (w_dict_size (d1) == 3,
                 "Expected 3 items in dict");

    fail_if (strcmp ("FOO", w_dict_get (d1, "foo")),
             "String 'FOO' does not match");
    fail_if (strcmp ("BAR", w_dict_get (d1, "bar")),
             "String 'BAR' does not match");
    fail_if (strcmp ("BAZINGA", w_dict_get (d1, "baz")),
             "Strcmp 'BAZINGA' does not match");

    w_obj_unref (d1);
    w_obj_unref (d2);
}
END_TEST


START_TEST (test_wdict_first)
{
    w_dict_t *d = w_dict_new (false);

    w_dict_set (d, "foo", "FOO");
    fail_if (strcmp ("FOO", *w_dict_first (d)),
             "String 'FOO' does not match");

    w_obj_unref (d);
}
END_TEST


START_TEST (test_wdict_iter)
{
    unsigned count = 0;
    w_dict_t *d = w_dict_new (false);
    w_iterator_t i;

    w_dict_set (d, "no.1", (void*) 1);
    w_dict_set (d, "no.2", (void*) 2);
    w_dict_set (d, "no.3", (void*) 3);
    w_dict_set (d, "no.4", (void*) 4);

    w_dict_foreach (d, i)
        count++;

    fail_unless (count == 4,
                 "Count is %u, expected 4", count);
    fail_unless (count == w_dict_size (d),
                 "Number of iterations does not match item count");

    w_obj_unref (d);
}
END_TEST
