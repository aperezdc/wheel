/*
 * wio-cat.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>
#define BUFFER_SIZE 512


int
main (int argc, char **argv)
{
    w_io_t *ioi = w_io_stdio_open (stdin);
    w_io_t *ioo = w_io_stdio_open (stdout);

    char buf[BUFFER_SIZE];
    ssize_t ret;

    w_unused (argc);
    w_unused (argv);

    while ((ret = w_io_read (ioi, buf, BUFFER_SIZE)) > 0) {
        w_io_write (ioo, buf, ret);
    }

    w_obj_unref (ioi);
    w_obj_unref (ioo);

    return EXIT_SUCCESS;
}


