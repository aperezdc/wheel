/*
 * check-wiofscan.c
 * Copyright (C) 2011-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <check.h>
#include <math.h>

#define IOSTR(_s) \
    w_buf_t __buf = W_BUF; \
    w_buf_set_str (&__buf, (_s)); \
    w_io_t *io = w_io_buf_open (&__buf)


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


#define CHECK_DVAL(_s, _f, _n)                                 \
    START_TEST (test_wiofscan_double_ ## _s) {                 \
        double dval;                                           \
        IOSTR (#_s);                                           \
        fail_if (w_io_fscan_double (io, &dval),                \
                 "conversion failed for \"" #_s "\"");         \
        fail_unless (_f (dval), "converted value is not " _n); \
        w_obj_unref (io);                                      \
    } END_TEST

#define CHECK_DVALF(_n, _v)                            \
    START_TEST (test_wiofscan_double_val_ ## _n) {     \
        double dval;                                   \
        IOSTR (#_v);                                   \
        fail_if (w_io_fscan_double (io, &dval),        \
                 "conversion failed for \"" #_v "\""); \
        fail_unless (fabs (dval - (_v)) < 0.2e-10,     \
                     "converted value is not " #_v);   \
        w_obj_unref (io);                              \
    } END_TEST

#define CHECK_DFAILF(_n, _v)                                 \
    START_TEST (test_wiofscan_double_fail_ ## _n) {          \
        double dval;                                         \
        IOSTR (_v);                                          \
        fail_unless (w_io_fscan_double (io, &dval),          \
                     "conversion succeeded for \"" _v "\""); \
        w_obj_unref (io);                                    \
    } END_TEST

CHECK_DVAL (nan, isnan, "NaN") /* Common NaN variations */
CHECK_DVAL (NaN, isnan, "NaN")
CHECK_DVAL (NAN, isnan, "NaN")
CHECK_DVAL (Nan, isnan, "NaN") /* Exotic NaN variations */
CHECK_DVAL (NAn, isnan, "NaN")
CHECK_DVAL (nAN, isnan, "NaN")
CHECK_DVAL (naN, isnan, "NaN")
CHECK_DVAL (nAn, isnan, "NaN")

CHECK_DVAL (inf,      isinf, "INFINITY") /* Common INF variations */
CHECK_DVAL (Inf,      isinf, "INFINITY")
CHECK_DVAL (INF,      isinf, "INFINITY")
CHECK_DVAL (infinity, isinf, "INFINITY")
CHECK_DVAL (INFINITY, isinf, "INFINITY")
CHECK_DVAL (InFiNiTy, isinf, "INFINITY") /* (Some) Exotic INF variations */
CHECK_DVAL (InFinIty, isinf, "INFINITY")
CHECK_DVAL (InF,      isinf, "INFINITY")
CHECK_DVAL (inF,      isinf, "INFINITY")
CHECK_DVAL (inFiniTY, isinf, "INFINITY")

CHECK_DVALF (zero,                0)
CHECK_DVALF (pluszero,           +0)
CHECK_DVALF (minuszero,          -0)
CHECK_DVALF (dotzero,            .0)
CHECK_DVALF (plusdotzero,       +.0)
CHECK_DVALF (minusdotzero,      -.0)
CHECK_DVALF (zerozero,          0.0)
CHECK_DVALF (pluszerozero,     +0.0)
CHECK_DVALF (minuszerozero,    -0.0)
CHECK_DVALF (one,                 1)
CHECK_DVALF (plusone,            +1)
CHECK_DVALF (minusone,           -1)
CHECK_DVALF (dotone,             .1)
CHECK_DVALF (plusdotone,        +.1)
CHECK_DVALF (minusdotone,       -.1)
CHECK_DVALF (oneone,            1.1)
CHECK_DVALF (plusoneone,       +1.1)
CHECK_DVALF (minusoneone,      -1.1)
CHECK_DVALF (ezero,             0e0)
CHECK_DVALF (epluszero,        +0e0)
CHECK_DVALF (eminuszero,       -0e0)
CHECK_DVALF (ezerozero,       0.0e0)
CHECK_DVALF (epluszerozero,  +0.0e0)
CHECK_DVALF (eminuszerozero, -0.0e0)
CHECK_DVALF (eone,              1e1)
CHECK_DVALF (eplusone,         +1e1)
CHECK_DVALF (eminusone,        -1e1)
CHECK_DVALF (eoneone,         1.1e1)
CHECK_DVALF (eoneplus,       +1.1e1)
CHECK_DVALF (eoneminus,      -1.1e1)

CHECK_DFAILF (fail00, "dafda")
CHECK_DFAILF (dotonly,    ".")


START_TEST (test_wio_fscan_two_mixed)
{
    long lval;
    long xval;
    IOSTR ("foo45:bar0xbabar");

    fail_unless (w_io_fscan (io, "foo$l:bar$Xr", &lval, &xval) == 2,
                 "Could not scan two values");
    ck_assert_int_eq (45, lval);
    ck_assert_int_eq (0xbaba, xval);
    w_obj_unref (io);
}
END_TEST

START_TEST (test_wio_fscan_empty_input)
{
    IOSTR ("");
    fail_unless (w_io_fscan (io, "") == 0,
                 "Coult not scan zero values");
    w_obj_unref (io);
}
END_TEST

START_TEST (test_wio_fscan_trailing_stuff)
{
    double dval;
    int ch;
    IOSTR ("pi=3.14... and moar");

    fail_unless (w_io_fscan (io, "pi=$F", &dval) == 1,
                 "Could not scan a double value");
    fail_unless (dval == 3.14, "Read value is not 3.14");

    /* Next has to be the dots */
    ch = w_io_getchar (io); ck_assert_int_eq ('.', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('.', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('.', ch);
    ch = w_io_getchar (io); ck_assert_int_eq (' ', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('a', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('n', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('d', ch);
    ch = w_io_getchar (io); ck_assert_int_eq (' ', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('m', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('o', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('a', ch);
    ch = w_io_getchar (io); ck_assert_int_eq ('r', ch);
    ch = w_io_getchar (io); ck_assert_int_eq (W_IO_EOF, ch);

    w_obj_unref (io);
}
END_TEST

START_TEST (test_wio_fscan_badinput)
{
    double dval;
    int ch;
    IOSTR ("pi=foo!bar:baz");

    fail_if (w_io_fscan (io, "pi=$F", &dval) != 0,
             "Elements were scanned on bad input.");

    ch = w_io_getchar (io); ck_assert_int_eq ('f', ch);

    w_obj_unref (io);
}
END_TEST
