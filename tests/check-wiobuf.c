/*
 * check-wiobuf.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <check.h>
#include <string.h>


static const char *msg = "Too much work and no joy makes Jack a dull boy.\n"
                         "Too much work and no joy makes Jack a dull boy.\n"
                         "Too much work and no joy makes Jack a dull boy.\n"
                         "Too much work and no joy makes Jack a dull boy.\n"
                         "Too much work and no joy makes Jack a dull boy.\n";


START_TEST (test_wio_buf_open_empty)
{
    w_io_t *io = w_io_buf_open (NULL);
    w_buf_t *buf = W_IO_BUF_BUF ((w_io_buf_t*) io);

    fail_unless (buf != NULL,
                 "I/O has no autocreated buffer");
    fail_unless (w_buf_size (buf) == 0,
                 "I/O autocreated buffer does not have zero length");

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wio_buf_open_nonempty)
{
    w_io_t *io;
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, msg);
    io = w_io_buf_open (&b);

    fail_unless (W_IO_BUF_BUF ((w_io_buf_t*) io) != NULL,
                "I/O has no buffer");
    fail_unless (W_IO_BUF_BUF ((w_io_buf_t*) io) == &b,
                 "I/O buffer is not the same as the specified one");
    fail_unless (w_buf_size (W_IO_BUF_BUF ((w_io_buf_t*) io)) == strlen (msg),
                 "I/O buffer has different length than message, "
                 "buffer = %lu, message = %lu",
                 w_buf_size (W_IO_BUF_BUF ((w_io_buf_t*) io)),
                 strlen (msg));

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wio_buf_read)
{
    char buf[64];
    ssize_t ret;
    w_io_t *io;
    w_buf_t b = W_BUF;

    w_buf_set_str (&b, msg);
    io = w_io_buf_open (&b);

    ret = w_io_read (io, buf, 64);
    fail_unless (ret == 64, "read %ld bytes, expected 64", (long) ret);
    fail_if (memcmp (msg, buf, 64), "strings are not equal");

    ret = w_io_read (io, buf, 10);
    fail_unless (ret == 10, "read %ld bytes, expected 10", (long) ret);
    fail_if (memcmp (msg + 64, buf, 10), "strings are not equal");

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wio_buf_getchar)
{
    w_buf_t buf = W_BUF;
    w_io_t *io;
    int ch;

    w_buf_set_str (&buf, "abcde");
    io = w_io_buf_open (&buf);

    ch = w_io_getchar (io); ck_assert_int_eq ('a', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('b', ch);
    w_io_putback (io, 'X');
    ch = w_io_getchar (io); ck_assert_int_eq ('X', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('c', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('d', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('e', ch);
    w_io_putback (io, 'Y');
    ch = w_io_getchar (io); ck_assert_int_eq ('Y', ch);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wio_buf_write)
{
    w_io_t *io;
    w_buf_t *b;

    io = w_io_buf_open (NULL);
    w_io_write (io, msg, 10);
    w_io_write (io, msg, 10);
    w_io_write (io, msg, 10);

    b = W_IO_BUF_BUF ((w_io_buf_t*) io);
    fail_if (b == NULL, "null autocreated buffer");

    fail_unless (w_buf_size (b) == 30,
                 "buffer length %lu, expected 30",
                 (unsigned long) w_buf_size (b));

    fail_if (memcmp (w_buf_data (b) +  0, msg, 10), "buffers do not match");
    fail_if (memcmp (w_buf_data (b) + 10, msg, 10), "buffers do not match");
    fail_if (memcmp (w_buf_data (b) + 20, msg, 10), "buffers do not match");

    w_obj_unref (io);
}
END_TEST

