/*
 * check-wiofscan.c
 * Copyright (C) 2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <check.h>

#define IOSTR(_s) \
    w_buf_t __buf = W_BUF; \
    w_io_t *io; \
    w_buf_set_str (&__buf, (_s)); \
    io = w_io_buf_open (&__buf)


START_TEST (test_wiofscan_ulong_zero)
{
    unsigned long uval;
    IOSTR ("0");

    fail_if (w_io_fscan_ulong (io, &uval),
             "converting \"0\" to ulong failed");
    ck_assert_int_eq (0, uval);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_ulong_one)
{
    unsigned long uval;
    IOSTR ("1");

    fail_if (w_io_fscan_ulong (io, &uval),
             "converting \"1\" to ulong failed");
    ck_assert_int_eq (1, uval);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_ulong_leadspace)
{
    unsigned long uval;
    IOSTR ("  123");

    fail_unless (w_io_fscan_ulong (io, &uval),
                 "conversion succeeded for value with leading spaces");

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_ulong_leadgarbage)
{
    unsigned long uval;
    IOSTR ("a dMDLSKM ALKM A 231231");

    fail_unless (w_io_fscan_ulong (io, &uval),
                 "conversion succeeded for value with leading garbage");

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_ulong_trailspace)
{
    unsigned long uval;
    IOSTR ("123  ");

    fail_if (w_io_fscan_ulong (io, &uval),
             "conversion failed for value with trailing spaces");
    ck_assert_int_eq (123, uval);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_ulong_trailgarbage)
{
    unsigned long uval;
    IOSTR ("321das dasdalk ol3k2el kewjafoifuj ");

    fail_if (w_io_fscan_ulong (io, &uval),
             "conversion failed for value with trailing garbage");
    ck_assert_int_eq (321, uval);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_ulong_leadplus)
{
    unsigned long uval;
    IOSTR ("+456");

    fail_if (w_io_fscan_ulong (io, &uval),
             "conversion failed for \"+456\"");
    ck_assert_int_eq (456, uval);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_ulong_leadminus)
{
    unsigned long uval;
    IOSTR ("-654");

    fail_unless (w_io_fscan_ulong (io, &uval),
                 "conversion succeeded for \"-654\"");
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_oct_zero)
{
    unsigned long uval;
    IOSTR ("0");

    fail_if (w_io_fscan_ulong_oct (io, &uval),
             "conversion failed for \"0\"");
    ck_assert_int_eq (0, uval);

    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_oct_9)
{
    unsigned long uval;
    IOSTR ("09");

    fail_if (w_io_fscan_ulong_oct (io, &uval),
             "conversion failed for \"09\"");
    ck_assert_int_eq (0, uval);
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_oct_midinvalid)
{
    unsigned long uval;
    IOSTR ("0123ds123");

    fail_if (w_io_fscan_ulong_oct (io, &uval),
             "conversion failed for \"0123ds123\"");
    ck_assert_int_eq (0123, uval);
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_oct_valid)
{
    unsigned long uval;
    IOSTR ("023123123125");

    fail_if (w_io_fscan_ulong_oct (io, &uval),
             "conversion failed for \"023123123125\"");
    ck_assert_int_eq (023123123125, uval);
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_hex_0)
{
    unsigned long uval;
    IOSTR ("0");

    fail_if (w_io_fscan_ulong_hex (io, &uval),
             "conversion failed for \"0\"");
    ck_assert_int_eq (0, uval);
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_hex_0x0)
{
    unsigned long uval;
    IOSTR ("0x0");

    fail_if (w_io_fscan_ulong_hex (io, &uval),
             "conversion failed for \"0x0\"");
    ck_assert_int_eq (0, uval);
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_hex_valid)
{
    unsigned long uval;
    IOSTR ("0xC0FFEE");

    fail_if (w_io_fscan_ulong_hex (io, &uval),
             "conversion failed for \"0xC0FEE\"");
    ck_assert_int_eq (0xC0FFEE, uval);
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_hex_invalid)
{
    unsigned long uval;
    IOSTR ("foobar");

    fail_unless (w_io_fscan_ulong_hex (io, &uval),
                 "conversion succeeded for \"foobar\"");
    w_obj_unref (io);
}
END_TEST


START_TEST (test_wiofscan_hex_midinvalid)
{
    unsigned long uval;
    IOSTR ("0xF143GH!");

    fail_if (w_io_fscan_ulong_hex (io, &uval),
             "conversion failed for \"0xF143GH!\"");
    ck_assert_int_eq (0xF143, uval);
    w_obj_unref (io);
}
END_TEST
