/*
 * check-wlist.c
 * Copyright (C) 2012-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <check.h>

START_TEST (test_wlist_itemcount)
{
    w_list_t *l = w_list_new (false);
    ck_assert_int_eq (0, w_list_size (l));

    w_list_append (l, (void*) 0xcafebabe);
    ck_assert_int_eq (1, w_list_size (l));

    w_list_append (l, (void*) 0xdeadface);
    ck_assert_int_eq (2, w_list_size (l));

    w_obj_unref (l);
}
END_TEST


START_TEST (test_wlist_firstlast)
{
    w_list_t *l = w_list_new (false);

    fail_if (w_list_first (l), "Empty list should not have first element");
    fail_if (w_list_last  (l), "Empty list should not have last element");

    w_list_append (l, (void*) 0xcafebabe);
    fail_unless (w_list_first (l) == w_list_last (l),
                 "List with one item is expected that first == last");

    fail_unless (*w_list_first (l) == (void*) 0xcafebabe,
                 "First element is not 0xcafebabe");

    w_list_append (l, (void*) 0xdeadface);
    fail_unless (w_list_first (l) != w_list_last (l),
                 "List with two items is expected that first != last");

    fail_unless (*w_list_first (l) == (void*) 0xcafebabe,
                 "First element is not 0xcafebabe");

    fail_unless (*w_list_last (l) == (void*) 0xdeadface,
                 "Last element is not 0xcafebabe");

    w_list_push_head (l, (void*) 0xb00fb00f);
    fail_unless (w_list_first (l) != w_list_last (l),
                 "List with three items is expected that first != last");

    fail_unless (*w_list_first (l) == (void*) 0xb00fb00f,
                 "First element is not 0xb00fb00f");

    fail_unless (*w_list_last (l) == (void*) 0xdeadface,
                 "Last element is not 0xcafebabe");

    w_obj_unref (l);
}
END_TEST


START_TEST (test_wlist_iterate)
{
    void *items[] = {
        (void*) 0xcafebabe,
        (void*) 0xb00fb00f,
        (void*) 0xdeadface,
        (void*) 0x00feca11,
        (void*) 0xf0caf0ca,
    };
    unsigned i;
    w_iterator_t iter;

    w_list_t *l = w_list_new (false);

    for (i = 0; i < w_lengthof (items); i++)
        w_list_append (l, items[i]);

    ck_assert_int_eq (w_lengthof (items), w_list_size (l));

    i = 0;
    w_list_foreach (l, iter)
        fail_unless (items[i++] == *iter, "items do not match");

    i = w_lengthof (items);
    w_list_foreach_rev (l, iter)
        fail_unless (items[--i] == *iter, "items do not match");

    w_obj_unref (l);
}
END_TEST
