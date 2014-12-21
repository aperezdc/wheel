/*
 * check-wioreaduntil.c
 * Copyright (C) 2011-2014 Adrian Perez <aperez@igalia.com>
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
    w_buf_t b = W_BUF;
    w_buf_t line = W_BUF;
    w_buf_t over = W_BUF;

    w_buf_set_str (&b, msg);
    w_io_t *io = w_io_buf_open (&b);

    /* Read first line */
    w_io_result_t r = w_io_read_line (io, &line, &over, 0);
    fail_if (w_io_failed (r), "Reading from buffer should succeed");
    fail_if (w_io_eof (r), "EOF marker cannot be reached");

    ck_assert_int_eq (8, w_buf_size (&line));
    ck_assert_str_eq ("Line one", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* Read next line */
    r = w_io_read_line (io, &line, &over, 0);
    fail_if (w_io_failed (r), "Reading from buffer should succeed");
    fail_if (w_io_eof (r), "EOF marker cannot be reached");

    ck_assert_str_eq ("Another line, number two", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* And now for the last one */
    r = w_io_read_line (io, &line, &over, 0);
    fail_if (w_io_failed (r), "Reading from buffer should succeed");
    fail_unless (w_io_eof (r), "EOF marker should be reached");

    ck_assert_str_eq ("And one more line, making it the third.", w_buf_str (&over));
    fail_if (w_buf_size (&line), "Line should be empty");
    w_buf_clear (&line);
    w_buf_clear (&over);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wio_read_line_smallbuf)
{
    w_buf_t b = W_BUF;
    w_buf_t line = W_BUF;
    w_buf_t over = W_BUF;

    w_buf_set_str (&b, msg);
    w_io_t *io = w_io_buf_open (&b);

    /* Read first line */
    w_io_result_t r = w_io_read_line (io, &line, &over, 10);
    fail_if (w_io_failed (r), "Reading from buffer should succeed");
    fail_if (w_io_eof (r), "EOF marker cannot be reached");

    ck_assert_int_eq (8, w_buf_size (&line));
    ck_assert_str_eq ("Line one", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* Read next line */
    r = w_io_read_line (io, &line, &over, 10);
    fail_if (w_io_failed (r), "Reading from buffer should succeed");
    fail_if (w_io_eof (r), "EOF marker cannot be reached");

    ck_assert_str_eq ("Another line, number two", w_buf_str (&line));
    fail_if (w_buf_size (&over) <= 0, "No overflow?");
    w_buf_clear (&line);

    /* And now for the last one */
    r = w_io_read_line (io, &line, &over, 10);
    fail_if (w_io_failed (r), "Reading from buffer should succeed");
    fail_unless (w_io_eof (r), "EOF marker should be reached");

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
    w_buf_t b = W_BUF;
    w_buf_t line = W_BUF;
    w_buf_t over = W_BUF;

    w_buf_set_str (&b, msg_empty_line);
    w_io_t *io = w_io_buf_open (&b);

    /* Read empty line */
    w_io_result_t r = w_io_read_line (io, &line, &over, 0);
    fail_if (w_io_failed (r), "Reading from buffer should succeed");
    fail_if (w_io_eof (r), "EOF marker cannot be reached");

    ck_assert_int_eq (0, w_io_result_bytes (r));
    ck_assert_int_eq (0, w_buf_size (&line));
    ck_assert_int_eq (0, w_buf_size (&over));

    /* Try to read past EOF, should return EOF */
    r = w_io_read_line (io, &line, &over, 0);
    fail_unless (w_io_eof (r), "EOF marker should be reached");

    w_buf_clear (&line);
    w_buf_clear (&over);
    w_obj_unref (io);

}
END_TEST
