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
    w_buf_reset (&b);

    fail_if (b.buf, "buffer is not null after reset");
    fail_if (b.bsz, "buffer size is not 0 after reset");
    fail_if (b.len, "buffer length is not 0 after reset");

    /* append something and then free again */
    w_buf_set_str (&b, "This is some content");
    w_buf_reset (&b);

    fail_if (b.buf, "buffer is not null after reset");
    fail_if (b.bsz, "buffer size is not 0 after reset");
    fail_if (b.len, "buffer length is not 0 after reset");
}
END_TEST


START_TEST (test_wbuf_set_str)
{
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, "This is some value");
    fail_if (strcmp ("This is some value", w_buf_str (&b)),
             "Expected 'This is some value', got '%s'", w_buf_str (&b));

    /* This should reset contents */
    w_buf_set_str (&b, "Another value");
    fail_if (strcmp ("Another value", w_buf_str (&b)),
             "Expected 'Another value', got '%s'", w_buf_str (&b));

    w_buf_free (&b);
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

    w_buf_free (&b);
}
END_TEST


START_TEST (test_wbuf_append_char)
{
    w_buf_t b = W_BUF;
    static const char foostr[] = "Too much work and no joy makes Jack a dull boy";
    size_t i;

    for (i = 0; i < w_lengthof (foostr); i++) {
        w_buf_append_char (&b, foostr[i]);
    }

    fail_if (b.len != w_lengthof (foostr),
             "Length is %lu, expected %lu",
             (unsigned long) b.len,
             (unsigned long) w_lengthof (foostr));

    fail_if (memcmp (b.buf, foostr, w_lengthof (foostr)),
             "Expected '%s', got '%s'", foostr, w_buf_str (&b));

    w_buf_free (&b);
}
END_TEST


START_TEST (test_wbuf_append_str)
{
    w_buf_t b = W_BUF;

    w_buf_append_str (&b, "Too much work ");
    w_buf_append_str (&b, "and no joy ");
    w_buf_append_str (&b, "makes Jack ");
    w_buf_append_str (&b, "a");
    w_buf_append_str (&b, " dull boy");

    fail_if (strcmp ("Too much work and no joy makes Jack a dull boy", w_buf_str (&b)),
             "Expected 'Too much work and no joy makes Jack a dull boy', got '%s'",
             w_buf_str (&b));

    w_buf_free (&b);
}
END_TEST


START_TEST (test_wbuf_append_buf)
{
    w_buf_t b1 = W_BUF;
    w_buf_t b2 = W_BUF;
    w_buf_t b3 = W_BUF;

    w_buf_set_str (&b1, "Too much work");
    w_buf_set_str (&b2, " and no joy ");
    w_buf_set_str (&b3, "makes Jack a dull boy");

    w_buf_append_buf (&b2, &b3);
    w_buf_append_buf (&b1, &b2);

    fail_if (strcmp (" and no joy makes Jack a dull boy", w_buf_str (&b2)),
             "Expected ' and no joy makes Jack a dul boy', got '%s'",
             w_buf_str (&b2));

    fail_if (strcmp ("Too much work and no joy makes Jack a dull boy", w_buf_str (&b1)),
             "Expected 'Too much work and no joy makes Jack a dull boy', got '%s'",
             w_buf_str (&b1));

    w_buf_free (&b1);
    w_buf_free (&b2);
    w_buf_free (&b3);
}
END_TEST
