/*
 * check-wioreaduntil.c
 * Copyright (C) 2011-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <check.h>

static const char *msg = "Line one\n"
                         "Another line, number two\n"
                         "And one more line, making it the third.";


START_TEST (test_wio_read_line)
{
    w_io_t *io;
    w_buf_t b = W_BUF;
    w_buf_t line = W_BUF;
    w_buf_t over = W_BUF;
    ssize_t ret;

    w_buf_set_str (&b, msg);
    io = w_io_buf_open (&b);

    /* Read first line */
    ret = w_io_read_line (io, &line, &over, 0);
    ck_assert_int_ne (W_IO_EOF, ret);
    ck_assert_int_ne (W_IO_ERR, ret);
    ck_assert_int_eq (8, w_buf_size (&line));
    ck_assert_str_eq ("Line one", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* Read next line */
    ret = w_io_read_line (io, &line, &over, 0);
    ck_assert_int_ne (W_IO_EOF, ret);
    ck_assert_int_ne (W_IO_ERR, ret);
    ck_assert_str_eq ("Another line, number two", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* And now for the last one */
    ret = w_io_read_line (io, &line, &over, 0);
    ck_assert_int_eq (W_IO_EOF, ret);
    ck_assert_str_eq ("And one more line, making it the third.", w_buf_str (&over));
    fail_if (w_buf_size (&line), "Line should be empty");
    w_buf_clear (&line);
    w_buf_clear (&over);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wio_read_line_smallbuf)
{
    w_io_t *io;
    w_buf_t b = W_BUF;
    w_buf_t line = W_BUF;
    w_buf_t over = W_BUF;
    ssize_t ret;

    w_buf_set_str (&b, msg);
    io = w_io_buf_open (&b);

    /* Read first line */
    ret = w_io_read_line (io, &line, &over, 10);
    ck_assert_int_ne (W_IO_EOF, ret);
    ck_assert_int_ne (W_IO_ERR, ret);
    ck_assert_int_eq (8, w_buf_size (&line));
    ck_assert_str_eq ("Line one", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* Read next line */
    ret = w_io_read_line (io, &line, &over, 10);
    ck_assert_int_ne (W_IO_EOF, ret);
    ck_assert_int_ne (W_IO_ERR, ret);
    ck_assert_str_eq ("Another line, number two", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* And now for the last one */
    ret = w_io_read_line (io, &line, &over, 10);
    ck_assert_int_eq (W_IO_EOF, ret);
    ck_assert_str_eq ("And one more line, making it the third.", w_buf_str (&over));
    fail_if (w_buf_size (&line), "Line should be empty");
    w_buf_clear (&line);
    w_buf_clear (&over);

    w_obj_unref (io);
}
END_TEST



static const char *msg_empty_line = "\n";


START_TEST (test_wio_read_empty_line)
{
    w_io_t *io;
    w_buf_t b = W_BUF;
    w_buf_t line = W_BUF;
    w_buf_t over = W_BUF;
    ssize_t ret;

    w_buf_set_str (&b, msg_empty_line);
    io = w_io_buf_open (&b);

    /* Read empty line */
    ret = w_io_read_line (io, &line, &over, 0);
    ck_assert_int_eq (0, ret);
    ck_assert_int_eq (0, w_buf_size (&line));
    ck_assert_int_eq (0, w_buf_size (&over));
    ck_assert_int_ne (W_IO_ERR, ret);
    ck_assert_int_ne (W_IO_EOF, ret);

    /* Try to read past EOF, should return EOF */
    ret = w_io_read_line (io, &line, &over, 0);
    ck_assert_int_eq (W_IO_EOF, ret);

    w_buf_clear (&line);
    w_buf_clear (&over);
    w_obj_unref (io);

}
END_TEST

