/*
 * check-wtnetstr.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <check.h>
#include "wheel.h"


START_TEST (test_wtnetstr_dump_basetypes)
{
    w_buf_t bv = W_BUF;
    w_buf_t b = W_BUF;

    fail_if (w_tnetstr_dump_null (&b), "could not dump null");
    ck_assert_str_eq ("0:~", w_buf_str (&b));
    ck_assert_int_eq (3, w_buf_length (&b));
    w_buf_free (&b);

    fail_if (w_tnetstr_dump_boolean (&b, W_YES), "could not dump boolean");
    ck_assert_str_eq ("4:true!", w_buf_str (&b));
    ck_assert_int_eq (7, w_buf_length (&b));
    w_buf_free (&b);

    fail_if (w_tnetstr_dump_boolean (&b, W_NO), "could not dump boolean");
    ck_assert_str_eq ("5:false!", w_buf_str (&b));
    ck_assert_int_eq (8, w_buf_length (&b));
    w_buf_free (&b);

    fail_if (w_tnetstr_dump_string (&b, "Hello, world!"), "could not dump string");
    ck_assert_str_eq ("13:Hello, world!,", w_buf_str (&b));
    ck_assert_int_eq (17, w_buf_length (&b));
    w_buf_free (&b);

    w_buf_append_str (&bv, "Hello, buffer!");
    fail_if (w_tnetstr_dump_buffer (&b, &bv), "could not dump buffer");
    ck_assert_str_eq ("14:Hello, buffer!,", w_buf_str (&b));
    ck_assert_int_eq (18, w_buf_length (&b));
    w_buf_free (&bv);
    w_buf_free (&b);

    fail_if (w_tnetstr_dump_number (&b, 42), "could not dump number");
    ck_assert_str_eq ("2:42#", w_buf_str (&b));
    ck_assert_int_eq (5, w_buf_length (&b));
    w_buf_free (&b);

    fail_if (w_tnetstr_dump_float (&b, 3.14), "could not dump float");
    ck_assert_str_eq ("4:3.14^", w_buf_str (&b));
    ck_assert_int_eq (7, w_buf_length (&b));
    w_buf_free (&b);
}
END_TEST


START_TEST (test_wtnetstr_dump_list)
{
    w_variant_t *variant;
    w_list_t *list = w_list_new (W_YES);
    w_buf_t b = W_BUF;

    /* empty list */
    fail_if (w_tnetstr_dump_list (&b, list), "could not dump list");
    ck_assert_str_eq ("0:]", w_buf_str (&b));
    ck_assert_int_eq (3, w_buf_length (&b));
    w_buf_free (&b);

    /* list with one item */
    w_list_append (list, (variant = w_variant_new (W_VARIANT_NULL)));
    w_obj_unref (variant);
    fail_if (w_tnetstr_dump_list (&b, list), "could not dump list");
    ck_assert_str_eq ("3:0:~]", w_buf_str (&b));
    ck_assert_int_eq (6, w_buf_length (&b));
    w_buf_free (&b);

    /* Now with two items */
    w_list_append (list, (variant = w_variant_new (W_VARIANT_NUMBER, 42)));
    w_obj_unref (variant);
    fail_if (w_tnetstr_dump_list (&b, list), "could not dump list");
    ck_assert_str_eq ("8:0:~2:42#]", w_buf_str (&b));
    ck_assert_int_eq (11, w_buf_length (&b));
    w_buf_free (&b);

    w_obj_unref (list);
}
END_TEST


START_TEST (test_wtnetstr_dump_dict)
{
    w_variant_t *variant;
    w_dict_t *dict = w_dict_new (W_YES);
    w_buf_t b = W_BUF;

    /* empty dict */
    fail_if (w_tnetstr_dump_dict (&b, dict), "could not dump dict");
    ck_assert_str_eq ("0:}", w_buf_str (&b));
    ck_assert_int_eq (3, w_buf_length (&b));
    w_buf_free (&b);

    /* Dictionary with one item */
    w_dict_set (dict, "Null", (variant = w_variant_new (W_VARIANT_NULL)));
    w_obj_unref (variant);
    fail_if (w_tnetstr_dump_dict (&b, dict), "could not dump dict");
    ck_assert_str_eq ("10:4:Null,0:~}", w_buf_str (&b));
    ck_assert_int_eq (14, w_buf_length (&b));
    w_buf_free (&b);

    w_obj_unref (dict);
}
END_TEST
