/*
 * check-wbuf.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <check.h>


START_TEST (test_wbuf_init)
{
    w_buf_t b = W_BUF;
    fail_if (b.buf, "buffer is not null upon creation");
    fail_if (b.bsz, "buffer size is not 0 upon creation");
    fail_if (b.len, "buffer length is not 0 upon creation");
}
END_TEST


START_TEST (test_wbuf_free)
{
    w_buf_t b = W_BUF;

    fail_if (b.buf, "buffer is not null upon creation");
    fail_if (b.bsz, "buffer size is not 0 upon creation");
    fail_if (b.len, "buffer length is not 0 upon creation");

    /* freeing should get to a sane state, always */
    w_buf_clear (&b);

    fail_if (b.buf, "buffer is not null after reset");
    fail_if (b.bsz, "buffer size is not 0 after reset");
    fail_if (b.len, "buffer length is not 0 after reset");

    /* append something and then free again */
    w_buf_set_str (&b, "This is some content");
    w_buf_clear (&b);

    fail_if (b.buf, "buffer is not null after reset");
    fail_if (b.bsz, "buffer size is not 0 after reset");
    fail_if (b.len, "buffer length is not 0 after reset");
}
END_TEST


START_TEST (test_wbuf_set_str)
{
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, "This is some value");
    ck_assert_str_eq ("This is some value", w_buf_str (&b));
    ck_assert_int_eq (strlen ("This is some value"), w_buf_size (&b));

    /* This should reset contents */
    w_buf_set_str (&b, "Another value");
    ck_assert_str_eq ("Another value", w_buf_str (&b));
    ck_assert_int_eq (strlen ("Another value"), w_buf_size (&b));

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wbuf_append_mem)
{
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, "XX");
    w_buf_append_mem (&b, "YYZZ", 3);

    fail_if (memcmp (b.buf, "XXYYZZ", 5),
             "Expected 'XXYYZ', got '%s'", w_buf_str (&b));

    w_buf_append_mem (&b, "Too much work and no joy...", 10);

    fail_if (memcmp (b.buf, "XXYYZToo much w", 15),
             "Expected 'XXYYZToo much w', got '%s'", w_buf_str (&b));

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wbuf_append_char)
{
    w_buf_t b = W_BUF;
    static const char foostr[] = "Too much work and no joy makes Jack a dull boy";
    size_t i;

    for (i = 0; i < strlen (foostr); i++)
        w_buf_append_char (&b, foostr[i]);

    ck_assert_int_eq (strlen (foostr), w_buf_size (&b));
    ck_assert_str_eq (foostr, w_buf_str (&b));
    ck_assert_int_eq (strlen (foostr), w_buf_size (&b));

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wbuf_append_str)
{
    w_buf_t b = W_BUF;
    size_t len = 0;

    w_buf_append_str (&b, "Too much work ");
    len += strlen ("Too much work ");
    ck_assert_int_eq (len, w_buf_size (&b));

    w_buf_append_str (&b, "and no joy ");
    len += strlen ("and no joy ");
    ck_assert_int_eq (len, w_buf_size (&b));

    w_buf_append_str (&b, "makes Jack ");
    len += strlen ("makes Jack ");
    ck_assert_int_eq (len, w_buf_size (&b));

    w_buf_append_str (&b, "a");
    len += strlen ("a");
    ck_assert_int_eq (len, w_buf_size (&b));

    w_buf_append_str (&b, " dull boy");
    len += strlen (" dull boy");
    ck_assert_int_eq (len, w_buf_size (&b));

    ck_assert_str_eq ("Too much work and no joy makes Jack a dull boy", w_buf_str (&b));

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wbuf_append_buf)
{
    w_buf_t b1 = W_BUF;
    w_buf_t b2 = W_BUF;
    w_buf_t b3 = W_BUF;

    w_buf_set_str (&b1, "Too much work");
    ck_assert_int_eq (strlen ("Too much work"), w_buf_size (&b1));

    w_buf_set_str (&b2, " and no joy ");
    ck_assert_int_eq (strlen (" and no joy "), w_buf_size (&b2));

    w_buf_set_str (&b3, "makes Jack a dull boy");
    ck_assert_int_eq (strlen ("makes Jack a dull boy"), w_buf_size (&b3));

    w_buf_append_buf (&b2, &b3);
    w_buf_append_buf (&b1, &b2);

    fail_if (strcmp (" and no joy makes Jack a dull boy", w_buf_str (&b2)),
             "Expected ' and no joy makes Jack a dul boy', got '%s'",
             w_buf_str (&b2));

    fail_if (strcmp ("Too much work and no joy makes Jack a dull boy", w_buf_str (&b1)),
             "Expected 'Too much work and no joy makes Jack a dull boy', got '%s'",
             w_buf_str (&b1));

    w_buf_clear (&b1);
    w_buf_clear (&b2);
    w_buf_clear (&b3);
}
END_TEST


START_TEST (test_wbuf_format)
{
    w_buf_t b = W_BUF;

    w_buf_format (&b, "string: $s number: $l", "the answer", 42);
    ck_assert_str_eq ("string: the answer number: 42", w_buf_str (&b));
    ck_assert_int_eq (strlen ("string: the answer number: 42"), w_buf_size (&b));

    w_buf_clear (&b);
}
END_TEST


START_TEST (test_wbuf_format_buf)
{
    w_buf_t b1 = W_BUF;
    w_buf_t b2 = W_BUF;

    w_buf_set_str (&b1, "0:~");
    w_buf_format (&b2, "$L:$B]", w_buf_size (&b1), &b1);

    ck_assert_int_eq (6, w_buf_size (&b2));
    ck_assert_str_eq ("3:0:~]", w_buf_str (&b2));
    ck_assert_int_eq (6, w_buf_size (&b2));

    w_buf_clear (&b1);
    w_buf_clear (&b2);
}
END_TEST


START_TEST (test_wbuf_format_multiple)
{
    w_buf_t b = W_BUF;

    w_buf_format (&b, "the answer is: $L", 42);
    w_buf_format (&b, " - sure man!");

    ck_assert_str_eq ("the answer is: 42 - sure man!",
                      w_buf_str (&b));
    ck_assert_int_eq (strlen ("the answer is: 42 - sure man!"),
                      w_buf_size (&b));
    w_buf_clear (&b);
}
END_TEST
