/*
 * werr.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * Error Reporting & Debugging
 * ===========================
 *
 * The debug-print macros :macro:`W_DEBUG` and :macro:`W_DEBUGC` produce
 * messages on standard error when ``_DEBUG_PRINT`` is defined before
 * including the ``wheel.h`` header., By default their expansion are empty
 * statements. The following builds ``program.c`` with the debug-print
 * statements turned on:
 *
 * .. code-block:: sh
 *
 *      cc -D_DEBUG_PRINT -o program program.c
 *
 *
 * The :macro:`W_FATAL` and :macro:`W_BUG` macros can be used to produce fatal
 * errors which abort execution of the program.
 *
 * The :macro:`W_WARN` macro can be used to produce non-fatal warnings, which
 * can optionally be converted into fatal warnings at run-time when programs
 * are run with the ``W_FATAL_WARNIGS`` environment variable defined to a
 * non-zero value in their environment.
 *
 * The :func:`w_die()` function can be used to exit a program gracefully
 * after printing an error message to standard error.
 */

#include "wheel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/**
 * Macros
 * ------
 */

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
                                    "$s (at $s, $s:$I): ",
                                    kind,
                                    func,
                                    file,
                                    line));
    }

    W_IO_NORESULT (w_io_formatv (w_stderr, fmt, args));
    W_IO_NORESULT (w_io_flush (w_stderr));
}


/*~M W_DEBUG(const char *format, ...)
 *
 * Produces a debug-print on standard error with the given `format`. The
 * macro automatically adds the function name, file name and line number
 * to the message.
 *
 * If ``_DEBUG_PRINT`` is not defined (the default), this macro expands to
 * an empty statement, causing no overhead.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */

/*~M W_DEBUGC(const char *format, ...)
 *
 * Produces a “continuation” debug-print on standard error with the given
 * `format`. The macro **does not** add the function name, file name and
 * line number to the message (hence “continuation”).
 *
 * If ``_DEBUG_PRINT`` is not defined (the default), this macro expands to
 * an empty statement, causing no overhead.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */
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


/*~M W_FATAL(const char *format, ...)
 *
 * Writes a fatal error message to standard error with the given `format`,
 * and aborts execution of the program. The macro automatically adds the
 * function name, file name and line number where the fatal error is
 * produced.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */

/*~M W_BUG(const char *message)
 *
 * Marks the line where the macro is expanded as a bug: if control ever
 * reaches the location, a fatal error is produced using :macro:`W_FATAL`
 * instructing the user to report the issue. The date and time of the build
 * are included in the error message.
 *
 * It is possible to supply an optional `message` to be printed next to
 * the supplied text. Note that, if supplied, this should be a hint to help
 * developers of the program.
 *
 * Note that, as the `message` is optional, both these macro expansions
 * produce valid code:
 *
 * .. code-block:: c
 *
 *      W_BUG();
 *      W_BUG("This is a bug");
 */
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


/*~M W_WARN(format, ...)
 *
 * Writes a warning to standard error with the given `format`. The macro
 * automatically adds the function name, file name and line number where
 * the warning is produced.
 *
 * See :ref:`formatted-output` for the available formatting options.
 *
 * If the ``W_FATAL_WARNINGS`` environment variable is defined and its
 * value is non-zero, warnings are converted into :macro:`W_FATAL`
 * errors and execution of the program will be aboerted.
 */
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


/**
 * Functions
 * ---------
 */

/*~f void w_die(const char *format, ...)
 *
 * Prints a message to standard error with a given `format` and exits the
 * program with the ``EXIT_FAILURE`` status code.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */
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
