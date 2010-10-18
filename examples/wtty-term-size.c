/*
 * wtty-term-size.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"


int main (int argc, char **argv)
{
    unsigned cols, rows;

    w_unused (argc);
    w_unused (argv);

    if (!w_tty_size (&cols, &rows))
        w_die ("Could not get terminal size");

    printf ("%u %u\n", cols, rows);

    return EXIT_SUCCESS;
}


