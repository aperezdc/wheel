/*
 * werr.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


void
w_die (const char *fmt, ...)
{
	va_list al;
    va_start (al, fmt);

	if (fmt)
        W_IO_NORESULT (w_io_formatv (w_stderr, fmt, al));
    va_end (al);

    W_IO_NORESULT (w_io_flush (w_stderr));

    exit (EXIT_FAILURE);
}


static void
print_message (const char *kind,
               const char *func,
               const char *file,
               unsigned    line,
               const char *fmt,
               va_list     args)
{
    if (func) {
        W_IO_NORESULT (w_io_format (w_stderr,
                                    "%s (at %s, %s:%u): ",
                                    kind,
                                    func,
                                    file,
                                    line));
    }

    W_IO_NORESULT (w_io_formatv (w_stderr, fmt, args));
    W_IO_NORESULT (w_io_flush (w_stderr));
}


void
w__debug (const char *func,
          const char *file,
          unsigned    line,
          const char *fmt,
          ...)
{
    va_list al;
    va_start(al, fmt);
    print_message ("DEBUG", func, file, line, fmt, al);
    va_end(al);
}


void
w__fatal (const char *func,
          const char *file,
          unsigned    line,
          const char *fmt,
          ...)
{
    va_list al;
    va_start(al, fmt);
    print_message ("FATAL", func, file, line, fmt, al);
    va_end(al);
    abort ();
}


void
w__warning (const char *func,
            const char *file,
            unsigned    line,
            const char *fmt,
            ...)
{
    va_list al;
    va_start(al, fmt);
    print_message ("WARNING", func, file, line, fmt, al);
    va_end(al);

    const char *envvar = getenv ("W_FATAL_WARNINGS");
    if (envvar && *envvar && *envvar != '0')
        abort ();
}
