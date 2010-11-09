/*
 * wio-cat.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
#define BUFFER_SIZE 512


int
main (int argc, char **argv)
{
    w_io_t ioi = W_IO_UNIX (STDIN_FILENO);
    w_io_t ioo = W_IO_UNIX (STDOUT_FILENO);

    char buf[BUFFER_SIZE];
    ssize_t ret;

    w_unused (argc);
    w_unused (argv);

    while ((ret = w_io_read (&ioi, buf, BUFFER_SIZE)) > 0) {
        w_io_write (&ioo, buf, ret);
    }

    return EXIT_SUCCESS;
}


