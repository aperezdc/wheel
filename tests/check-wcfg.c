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

