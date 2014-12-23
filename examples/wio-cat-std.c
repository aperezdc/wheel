/*
 * wio-cat.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <stdio.h>
#define BUFFER_SIZE 512


int
main (int argc, char **argv)
{
    w_unused (argc);
    w_unused (argv);

    w_io_t *ioi = w_io_stdio_open (stdin);
    w_io_t *ioo = w_io_stdio_open (stdout);

    char buf[BUFFER_SIZE];
    w_io_result_t r;

    while (!w_io_failed (r = w_io_read (ioi, buf, BUFFER_SIZE)) &&
           w_io_result_bytes (r) > 0) {
        W_IO_CHECK_RETURN (w_io_write (ioo, buf, w_io_result_bytes (r)),
                           EXIT_FAILURE);
    }

    w_obj_unref (ioi);
    w_obj_unref (ioo);

    return EXIT_SUCCESS;
}


