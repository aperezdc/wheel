/*
 * check-wvariant.c
 * Copyright (C) 2012-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <check.h>


START_TEST (test_wvariant_store_vals)
{
    w_list_t *list;
    w_dict_t *dict;
    w_buf_t buf = W_BUF;
    w_variant_t *var;

    var = w_variant_new (W_VARIANT_INVALID);
    ck_assert_int_eq (W_VARIANT_INVALID, w_variant_type (var));
    w_obj_unref (var);

    var = w_variant_new (W_VARIANT_BOOL, true);
    ck_assert_int_eq (W_VARIANT_BOOL, w_variant_type (var));
    ck_assert_int_eq (true, w_variant_bool (var));
    w_obj_unref (var);

    var = w_variant_new (W_VARIANT_BOOL, false);
    ck_assert_int_eq (W_VARIANT_BOOL, w_variant_type (var));
    ck_assert_int_eq (false, w_variant_bool (var));
    w_obj_unref (var);

    var = w_variant_new (W_VARIANT_STRING, "Hello, world!");
    ck_assert_int_eq (W_VARIANT_STRING, w_variant_type (var));
    ck_assert_str_eq ("Hello, world!", w_variant_string (var));
    w_obj_unref (var);

    w_buf_append_str (&buf, "Welcome Mr. Marshall");
    var = w_variant_new (W_VARIANT_BUFFER, &buf);
    w_buf_clear (&buf);
    ck_assert_int_eq (W_VARIANT_STRING, w_variant_type (var));
    ck_assert_str_eq ("Welcome Mr. Marshall", w_variant_string (var));
    w_obj_unref (var);

    var = w_variant_new (W_VARIANT_NULL);
    ck_assert_int_eq (W_VARIANT_NULL, w_variant_type (var));
    w_obj_unref (var);

    list = w_list_new (false);
    var = w_variant_new (W_VARIANT_LIST, list);
    ck_assert_int_eq (W_VARIANT_LIST, w_variant_type (var));
    fail_if (list != w_variant_list (var),
             "List stored in variant is not the same");
    /* Adding the list in the variant increases the refcount */
    ck_assert_int_eq (2, list->parent.__refs);
    w_obj_unref (list);
    w_obj_unref (var);

    dict = w_dict_new (false);
    var = w_variant_new (W_VARIANT_DICT, dict);
    ck_assert_int_eq (W_VARIANT_DICT, w_variant_type (var));
    fail_if (dict != w_variant_dict (var),
             "Dict stored in variant is not the same");
    /* adding the dict in the variant increases the refcount */
    ck_assert_int_eq (2, dict->parent.__refs);
    w_obj_unref (dict);
    w_obj_unref (var);
}
END_TEST


START_TEST (test_wvariant_mutate)
{
    w_list_t *list = w_list_new (false);
    w_dict_t *dict = w_dict_new (false);
    w_variant_t *var = w_variant_new (W_VARIANT_INVALID);

    /* mutate to a list, the list gets an extra ref */
    w_variant_set_list (var, list);
    ck_assert_int_eq (W_VARIANT_LIST, w_variant_type (var));
    ck_assert_int_eq (2, list->parent.__refs);

    /* mutate to a null, the list gets one less ref */
    w_variant_set_null (var);
    ck_assert_int_eq (W_VARIANT_NULL, w_variant_type (var));
    ck_assert_int_eq (1, list->parent.__refs);

    /* Mutate again to a list */
    w_variant_set_list (var, list);
    ck_assert_int_eq (W_VARIANT_LIST, w_variant_type (var));
    ck_assert_int_eq (2, list->parent.__refs);

    w_obj_unref (list); /* not used after this point */

    /* now to a dictionary */
    w_variant_set_dict (var, dict);
    ck_assert_int_eq (W_VARIANT_DICT, w_variant_type (var));
    ck_assert_int_eq (2, dict->parent.__refs);

    /*
     * now, if we unref the dict, it should still have one ref (held
     * by the variant), and the final w_obj_unref(var) would destroy
     * also the dict.
     */
    w_obj_unref (dict);
    ck_assert_int_eq (1, dict->parent.__refs);

    w_obj_unref (var);
}
END_TEST


