
Error Reporting & Debugging
===========================

The debug-print macros :macro:`W_DEBUG` and :macro:`W_DEBUGC` produce
messages on standard error when ``_DEBUG_PRINT`` is defined before
including the ``wheel.h`` header., By default their expansion are empty
statements. The following builds ``program.c`` with the debug-print
statements turned on:

.. code-block:: sh

     cc -D_DEBUG_PRINT -o program program.c


The :macro:`W_FATAL` and :macro:`W_BUG` macros can be used to produce fatal
errors which abort execution of the program.

The :macro:`W_WARN` macro can be used to produce non-fatal warnings, which
can optionally be converted into fatal warnings at run-time when programs
are run with the ``W_FATAL_WARNIGS`` environment variable defined to a
non-zero value in their environment.

The :func:`w_die()` function can be used to exit a program gracefully
after printing an error message to standard error.


Macros
------

.. c:macro:: W_DEBUG(const char *format, ...)

   Produces a debug-print on standard error with the given `format`. The
   macro automatically adds the function name, file name and line number
   to the message.

   If ``_DEBUG_PRINT`` is not defined (the default), this macro expands to
   an empty statement, causing no overhead.

   See :ref:`formatted-output` for the available formatting options.

.. c:macro:: W_DEBUGC(const char *format, ...)

   Produces a “continuation” debug-print on standard error with the given
   `format`. The macro **does not** add the function name, file name and
   line number to the message (hence “continuation”).

   If ``_DEBUG_PRINT`` is not defined (the default), this macro expands to
   an empty statement, causing no overhead.

   See :ref:`formatted-output` for the available formatting options.

.. c:macro:: W_FATAL(const char *format, ...)

   Writes a fatal error message to standard error with the given `format`,
   and aborts execution of the program. The macro automatically adds the
   function name, file name and line number where the fatal error is
   produced.

   See :ref:`formatted-output` for the available formatting options.

.. c:macro:: W_BUG(const char *message)

   Marks the line where the macro is expanded as a bug: if control ever
   reaches the location, a fatal error is produced using :macro:`W_FATAL`
   instructing the user to report the issue. The date and time of the build
   are included in the error message.

   It is possible to supply an optional `message` to be printed next to
   the supplied text. Note that, if supplied, this should be a hint to help
   developers of the program.

   Note that, as the `message` is optional, both these macro expansions
   produce valid code:

   .. code-block:: c

        W_BUG();
        W_BUG("This is a bug");

.. c:macro:: W_WARN(format, ...)

   Writes a warning to standard error with the given `format`. The macro
   automatically adds the function name, file name and line number where
   the warning is produced.

   See :ref:`formatted-output` for the available formatting options.

   If the ``W_FATAL_WARNINGS`` environment variable is defined and its
   value is non-zero, warnings are converted into :macro:`W_FATAL`
   errors and execution of the program will be aboerted.


Functions
---------

.. c:function:: void w_die(const char *format, ...)

   Prints a message to standard error with a given `format` and exits the
   program with the ``EXIT_FAILURE`` status code.

   See :ref:`formatted-output` for the available formatting options.

