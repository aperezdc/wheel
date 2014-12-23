/*
 * wio-cat.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
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

    w_unused (argc);
    w_unused (argv);

    w_io_result_t r;
    while (w_io_result_bytes (r = w_io_read (w_stdin, buf, BUFFER_SIZE)) > 0) {
        W_IO_NORESULT (w_io_write (w_stdout, buf, w_io_result_bytes (r)));
    }

    return EXIT_SUCCESS;
}


