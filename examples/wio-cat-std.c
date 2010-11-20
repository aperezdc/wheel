/*
 * wio-cat.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>
#define BUFFER_SIZE 512


int
main (int argc, char **argv)
{
    W_IO_STDIO (ioi);
    W_IO_STDIO (ioo);
    char buf[BUFFER_SIZE];
    ssize_t ret;

    w_unused (argc);
    w_unused (argv);

    w_io_stdio_open (ioi, stdin);
    w_io_stdio_open (ioo, stdout);

    while ((ret = w_io_read (ioi, buf, BUFFER_SIZE)) > 0) {
        w_io_write (ioo, buf, ret);
    }

    return EXIT_SUCCESS;
}


