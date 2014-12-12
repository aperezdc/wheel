/*
 * wio-cat.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <unistd.h>
#define BUFFER_SIZE 512


int
main (int argc, char **argv)
{
    char buf[BUFFER_SIZE];
    ssize_t ret;

    w_unused (argc);
    w_unused (argv);

    while ((ret = w_io_read (w_stdin, buf, BUFFER_SIZE)) > 0) {
        w_io_write (w_stdout, buf, ret);
    }

    return EXIT_SUCCESS;
}


