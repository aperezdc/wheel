/*
 * check-wcfg.c
 * Copyright (C) 2011-2013 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <check.h>


START_TEST (test_wcfg_load_empty)
{
    w_io_t *io = w_io_buf_open (NULL);
    char *message = NULL;

    w_cfg_t *cf = w_cfg_load (io, &message);
    fail_if (cf == NULL, "w_cfg_load returned unexpected NULL");
    fail_if (message != NULL, "w_cfg_load set an unexpected error message");

    w_obj_unref (cf);
    w_obj_unref (io);
}
END_TEST


#define S_(_x) { (uint8_t*) (_x), (w_lengthof (_x) - 1) }
static const struct {
    uint8_t *data;
    size_t   size;
} test_wcfg_invalid_input__loop__data[] = {
    // Key without value.
    S_ ("a"),
    S_ ("a "),
    // Unterminated strings.
    S_ ("a \""),
    S_ ("a \"a"),
    S_ ("a \"\\\""),
    // Invalid numbers.
    S_ ("a -"),
    S_ ("a +"),
    S_ ("a -+"),
    S_ ("a +-"),
    S_ ("a a"),
    S_ ("a ☺"),
    S_ ("a -."),
    S_ ("a ."),
    S_ ("a e"),
    S_ ("a .e"),
    S_ ("a +e"),
    S_ ("a -e"),
    S_ ("a -.e"),
    S_ ("a +.e"),
    S_ ("a e+"),
    S_ ("a e-"),
    S_ ("a .-e"),
    S_ ("a .+e"),
    S_ ("a --"),
    S_ ("a ++"),
    S_ ("a +1e3."),
    S_ ("a ..1"),
    S_ ("a 1.2."),
    S_ ("a 1..2"),
    S_ ("a foo"),
    S_ ("a 0xx00"),
    S_ ("a 0.aAeA3"),
    S_ ("a -0x3"),
    S_ ("a -032"),
    S_ ("a ee"),
    S_ ("a 1ee"),
    S_ ("a 1e1e1e"),
    S_ ("a 0.1x2"),
    S_ ("a 1x.0"),
    S_ ("a 1,2"),
    // Invalid booleans.
    S_ ("a Tr"),
    S_ ("a TRUE"),
    S_ ("a TrUE"),
    S_ ("a Fa"),
    S_ ("a FALSE"),
    S_ ("a FaLSE"),
    // Invalid arrays.
    S_ ("a ["),
    S_ ("a [1"),
    S_ ("a [1 2"),
    S_ ("a [[]"),
    S_ ("a (1)"),
    S_ ("a [:]"),
    S_ ("a [\"]"),
    S_ ("a [\"]\""),
    S_ ("a [,]"),
    S_ ("a [ ,]"),
    S_ ("a [, ]"),
    S_ ("a [ , ]"),
    S_ ("a [\v,\t]"),
    S_ ("a [,1]"),
    S_ ("a [1,,]"),
    S_ ("a ]"),
    // Invalid dictionaries.
    S_ ("a { b }"),
    S_ ("a { a::1 }"),
    S_ ("a {"),
    S_ ("a {[}]"),
    S_ ("a {:}"),
    S_ ("a }"),
};
#undef S_

START_TEST (test_wcfg_invalid_input__loop)
{
    w_io_mem_t iomem;
    uint8_t *input = test_wcfg_invalid_input__loop__data[_i].data;
    w_io_mem_init (&iomem, input, test_wcfg_invalid_input__loop__data[_i].size);

    char *message = NULL;
    w_cfg_t *cf = w_cfg_load ((w_io_t*) &iomem, &message);
    fail_unless (message != NULL, "Loading of invalid input [%i:<%s>] must fail", _i, input);
    fail_if (cf, "Returned value for invalid input [%i:<%s>] is not NULL", _i, input);
}
END_TEST
