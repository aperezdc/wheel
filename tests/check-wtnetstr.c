/*
 * check-wtnetstr.c
 * Copyright (C) 2012-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <check.h>


START_TEST (test_wtnetstr_dump_basetypes)
{
    w_buf_t bv = W_BUF;
    w_buf_t b = W_BUF;

    fail_if (w_io_failed (w_tnetstr_dump_null (&b)),
             "could not dump null");
    ck_assert_str_eq ("0:~", w_buf_str (&b));
    ck_assert_int_eq (3, w_buf_size (&b));
    w_buf_clear (&b);

    fail_if (w_io_failed (w_tnetstr_dump_bool (&b, true)),
             "could not dump boolean");
    ck_assert_str_eq ("4:true!", w_buf_str (&b));
    ck_assert_int_eq (7, w_buf_size (&b));
    w_buf_clear (&b);

    fail_if (w_io_failed (w_tnetstr_dump_bool (&b, false)),
             "could not dump boolean");
    ck_assert_str_eq ("5:false!", w_buf_str (&b));
    ck_assert_int_eq (8, w_buf_size (&b));
    w_buf_clear (&b);

    fail_if (w_io_failed (w_tnetstr_dump_string (&b, "Hello, world!")),
             "could not dump string");
    ck_assert_str_eq ("13:Hello, world!,", w_buf_str (&b));
    ck_assert_int_eq (17, w_buf_size (&b));
    w_buf_clear (&b);

    w_buf_append_str (&bv, "Hello, buffer!");
    fail_if (w_io_failed (w_tnetstr_dump_buffer (&b, &bv)),
             "could not dump buffer");
    ck_assert_str_eq ("14:Hello, buffer!,", w_buf_str (&b));
    ck_assert_int_eq (18, w_buf_size (&b));
    w_buf_clear (&bv);
    w_buf_clear (&b);

    fail_if (w_io_failed (w_tnetstr_dump_number (&b, 42)),
             "could not dump number");
    ck_assert_str_eq ("2:42#", w_buf_str (&b));
    ck_assert_int_eq (5, w_buf_size (&b));
    w_buf_clear (&b);

    fail_if (w_io_failed (w_tnetstr_dump_float (&b, 3.14)),
             "could not dump float");
    ck_assert_str_eq ("4:3.14^", w_buf_str (&b));
    ck_assert_int_eq (7, w_buf_size (&b));
    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wtnetstr_dump_list)
{
    w_variant_t *variant;
    w_list_t *list = w_list_new (true);
    w_buf_t b = W_BUF;

    /* empty list */
    fail_if (w_io_failed (w_tnetstr_dump_list (&b, list)),
             "could not dump list");
    ck_assert_str_eq ("0:]", w_buf_str (&b));
    ck_assert_int_eq (3, w_buf_size (&b));
    w_buf_clear (&b);

    /* list with one item */
    w_list_append (list, (variant = w_variant_new (W_VARIANT_TYPE_NULL)));
    w_obj_unref (variant);
    fail_if (w_io_failed (w_tnetstr_dump_list (&b, list)),
             "could not dump list");
    ck_assert_str_eq ("3:0:~]", w_buf_str (&b));
    ck_assert_int_eq (6, w_buf_size (&b));
    w_buf_clear (&b);

    /* Now with two items */
    w_list_append (list, (variant = w_variant_new (W_VARIANT_TYPE_NUMBER, 42)));
    w_obj_unref (variant);
    fail_if (w_io_failed (w_tnetstr_dump_list (&b, list)),
             "could not dump list");
    ck_assert_str_eq ("8:0:~2:42#]", w_buf_str (&b));
    ck_assert_int_eq (11, w_buf_size (&b));
    w_buf_clear (&b);

    w_obj_unref (list);
}
END_TEST


START_TEST (test_wtnetstr_dump_dict)
{
    w_variant_t *variant;
    w_dict_t *dict = w_dict_new (true);
    w_buf_t b = W_BUF;

    /* empty dict */
    fail_if (w_io_failed (w_tnetstr_dump_dict (&b, dict)),
             "could not dump dict");
    ck_assert_str_eq ("0:}", w_buf_str (&b));
    ck_assert_int_eq (3, w_buf_size (&b));
    w_buf_clear (&b);

    /* Dictionary with one item */
    w_dict_set (dict, "Null", (variant = w_variant_new (W_VARIANT_TYPE_NULL)));
    w_obj_unref (variant);
    fail_if (w_io_failed (w_tnetstr_dump_dict (&b, dict)),
             "could not dump dict");
    ck_assert_str_eq ("10:4:Null,0:~}", w_buf_str (&b));
    ck_assert_int_eq (14, w_buf_size (&b));
    w_buf_clear (&b);

    w_obj_unref (dict);
}
END_TEST


START_TEST (test_wtnetstr_parse_null)
{
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, "0:~");
    fail_if (w_tnetstr_parse_null (&b),
             "Could not parse valid null");

    w_buf_set_str (&b, "1:~");
    fail_unless (w_tnetstr_parse_null (&b),
                 "Parsed invalid null as valid");

    w_buf_set_str (&b, " 0:~");
    fail_unless (w_tnetstr_parse_null (&b),
                 "Parsed invalid null as valid");

    w_buf_set_str (&b, "0:d~ ");
    fail_unless (w_tnetstr_parse_null (&b),
                 "Parsed invalid null as valid");

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wtnetstr_parse_bool)
{
    bool value = false;
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, "4:true!");
    fail_if (w_tnetstr_parse_bool (&b, &value),
             "Could not parse valid boolean");
    ck_assert_int_eq (true, value);

    w_buf_set_str (&b, "5:false!");
    fail_if (w_tnetstr_parse_bool (&b, &value),
             "Could not parse valid boolean");
    ck_assert_int_eq (false, value);

    w_buf_set_str (&b, "4:burp!");
    fail_unless (w_tnetstr_parse_bool (&b, &value),
                 "Parsed invalid boolean as valid");

    w_buf_set_str (&b, "4:true,");
    fail_unless (w_tnetstr_parse_bool (&b, &value),
                 "Parsed invalid boolean as valid");

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wtnetstr_parse_string)
{
    w_buf_t b = W_BUF;
    w_buf_t r = W_BUF;

    w_buf_set_str (&b, "0:,");
    fail_if (w_tnetstr_parse_string (&b, &r),
             "Could not parse empty string");
    ck_assert_int_eq (0, w_buf_size (&r));

    w_buf_set_str (&b, "1:X,");
    fail_if (w_tnetstr_parse_string (&b, &r),
             "Could not parse 1-char string");
    ck_assert_int_eq (1, w_buf_size (&r));
    ck_assert_str_eq ("X", w_buf_str (&r));
    w_buf_clear (&r);

    w_buf_set_str (&b, "10:a\020,:.1b3d5,");
    fail_if (w_tnetstr_parse_string (&b, &r),
             "Could not parse 10-char string");
    ck_assert_int_eq (10, w_buf_size (&r));
    ck_assert_str_eq ("a\020,:.1b3d5", w_buf_str (&r));
    w_buf_clear (&r);

    w_buf_set_str (&b, "5:12345#");
    fail_unless (w_tnetstr_parse_string (&b, &r),
                 "Parsed an invalid input as valid");
    w_buf_clear (&r);

    w_buf_set_str (&b, "0:12345,");
    fail_unless (w_tnetstr_parse_string (&b, &r),
                 "Parsed an invalid input as valid");
    w_buf_clear (&r);

    w_buf_set_str (&b, "10:abc,");
    fail_unless (w_tnetstr_parse_string (&b, &r),
                 "Parsed an invalid input as valid");
    w_buf_clear (&r);

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wtnetstr_parse_number)
{
    long value = 0;
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, "2:42#");
    fail_if (w_tnetstr_parse_number (&b, &value),
             "Could not parse numeric value");
    ck_assert_int_eq (42, value);

    w_buf_set_str (&b, "1:0#");
    fail_if (w_tnetstr_parse_number (&b, &value),
             "Could not parse numeric value");
    ck_assert_int_eq (0, value);

    w_buf_set_str (&b, "4:-456#");
    fail_if (w_tnetstr_parse_number (&b, &value),
             "Could not parse numeric value");
    ck_assert_int_eq (-456, value);

    w_buf_set_str (&b, "3:-456#");
    fail_unless (w_tnetstr_parse_number (&b, &value),
                 "Parsed invalid number as valid");

    w_buf_set_str (&b, "3:abc#");
    fail_unless (w_tnetstr_parse_number (&b, &value),
                 "Parsed invalid number as valid");
}
END_TEST


START_TEST (test_wtnetstr_parse_float)
{
    double value = 0;
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, "2:42^");
    fail_if (w_tnetstr_parse_float (&b, &value),
             "Could not parse numeric value");
    fail_if (42.0 != value, "Could not parse 42.0");

    w_buf_set_str (&b, "1:0^");
    fail_if (w_tnetstr_parse_float (&b, &value),
             "Could not parse float numeric value");
    fail_if (0.0 != value, "Could not parse 0.0");

    w_buf_set_str (&b, "4:-456^");
    fail_if (w_tnetstr_parse_float (&b, &value),
             "Could not parse float numeric value");
    fail_if (-456.0 != value, "Could not parse -456.0");

    w_buf_set_str (&b, "5:-3e-5^");
    fail_if (w_tnetstr_parse_float (&b, &value),
             "Could not parse float numeric value");
    fail_if (-3e-5 != value, "Could not parse -3e-5");

    w_buf_set_str (&b, "3:-456^");
    fail_unless (w_tnetstr_parse_float (&b, &value),
                 "Parsed invalid float number as valid");

    w_buf_set_str (&b, "3:abc^");
    fail_unless (w_tnetstr_parse_float (&b, &value),
                 "Parsed invalid float number as valid");

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wtnetstr_parse_list)
{
    w_list_t *list = w_list_new (true);
    w_buf_t b = W_BUF;

    /* empty list */
    w_buf_set_str (&b, "0:]");
    fail_if (w_tnetstr_parse_list (&b, list),
             "Could not parse empty list");
    ck_assert_int_eq (0, w_list_size (list));

    /* one item */
    w_buf_set_str (&b, "3:0:~]");
    fail_if (w_tnetstr_parse_list (&b, list),
             "Coult not parse 1-item list");
    ck_assert_int_eq (1, w_list_size (list));
    fail_unless (w_variant_is_null ((w_variant_t*) w_list_at (list, 0)),
                 "Item in list is not a null");
    w_list_clear (list);

    /* two items */
    w_buf_set_str (&b, "6:0:~0:~]");
    fail_if (w_tnetstr_parse_list (&b, list),
             "Coult not parse 2-item list");
    ck_assert_int_eq (2, w_list_size (list));
    fail_unless (w_variant_is_null ((w_variant_t*) w_list_at (list, 0)),
                 "Item[0] in list is not a null");
    fail_unless (w_variant_is_null ((w_variant_t*) w_list_at (list, 1)),
                 "Item[1] in list is not a null");
    w_list_clear (list);

    /* nested lists */
    w_buf_set_str (&b, "9:0:~3:0:~]]"); /* [null, [null]] */
    fail_if (w_tnetstr_parse_list (&b, list),
             "Coult not parse 2-item list");
    ck_assert_int_eq (2, w_list_size (list));
    fail_unless (w_variant_is_null ((w_variant_t*) w_list_at (list, 0)),
                 "Item[0] in list is not a null");
    fail_unless (w_variant_is_list ((w_variant_t*) w_list_at (list, 1)),
                 "Item[1] in list is not a list");
    w_list_clear (list);

    w_obj_unref (list);
    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wtnetstr_parse_dict)
{
    w_variant_t *variant;
    w_dict_t *dict = w_dict_new (true);
    w_buf_t b = W_BUF;

    /* empty dict */
    w_buf_set_str (&b, "0:}");
    fail_if (w_tnetstr_parse_dict (&b, dict),
             "Coult not parse empty dict");
    ck_assert_int_eq (0, w_dict_size (dict));

    /* one item */
    w_buf_set_str (&b, "7:1:a,0:~}");
    fail_if (w_tnetstr_parse_dict (&b, dict),
             "Coult not parse 1-item dict");
    ck_assert_int_eq (1, w_dict_size (dict));
    variant = w_dict_get (dict, "a");
    fail_unless (variant != NULL, "Key 'a' is not set in dict");
    fail_unless (w_variant_is_null (variant), "Item 'a' is not null");
    w_dict_clear (dict);

    /* two items */
    w_buf_set_str (&b, "16:1:a,1:1#1:b,1:2#}");
    fail_if (w_tnetstr_parse_dict (&b, dict),
             "Coult not parse 2-item dict");
    ck_assert_int_eq (2, w_dict_size (dict));

    variant = w_dict_get (dict, "a");
    fail_unless (variant != NULL, "Key 'a' is not set in dict");
    fail_unless (w_variant_is_number (variant), "Item 'a' is not number");
    ck_assert_int_eq (1, w_variant_number (variant));

    variant = w_dict_get (dict, "b");
    fail_unless (variant != NULL, "Key 'a' is not set in dict");
    fail_unless (w_variant_is_number (variant), "Item 'b' is not number");
    ck_assert_int_eq (2, w_variant_number (variant));

    w_dict_clear (dict);

    w_obj_unref (dict);
    w_buf_clear (&b);
}
END_TEST
