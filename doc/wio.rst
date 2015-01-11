
.. _wio:

Input/Output Streams
====================

The following kinds of streams are provided:

- :ref:`wio-buf`
- :ref:`wio-mem`
- :ref:`wio-stdio`
- :ref:`wio-unix`
- :ref:`wio-socket`


.. _formatted-output:

Formatted output
----------------

A number of functions support specifying a *format string*, and will
accept a variable amount of additional function arguments, depending
on the *format specifiers* present in the *format string*. All those
functions use the same formatting mechanism, as described here.

*Format specifiers* are sequences of characters started with a dollar
symbol (``$``), followed by at a character, which determines the type
and amount of the additional function arguments being consumed.

The recognized *format specifiers* are:

========= ===================== =================
Specifier Type(s)               Output format.
========= ===================== =================
 ``$c``   int                   Character.
 ``$l``   long int              Decimal number.
 ``$L``   unsigned long int     Decimal number.
 ``$i``   int                   Decimal number.
 ``$I``   unsigned int          Decimal number.
 ``$X``   unsigned long int     Hexadecimal number.
 ``$O``   unsigned long int     Octal number.
 ``$p``   void*                 Pointer, as hexadecimal number.
 ``$f``   float                 Floating point number.
 ``$F``   double                Floating point number.
 ``$s``   const char*           A ``\0``-terminated string.
 ``$B``   w_buf_t*              Arbitrary data (usually a string).
 ``$S``   size_t, const char*   String of a particular length.
 ``$e``                         Last value of ``errno``, as an integer.
 ``$E``                         Last value of ``errno``, as a string.
 ``$R``   w_io_result_t         String representing the return value of an
                                input/output operation (see
                                :ref:`wio-result-format` below).
========= ===================== =================


.. _wio-result-format:

Output for ``w_io_result_t``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``$R`` *format specifier* will consume a :type:`w_io_result_t` value,
and format it as a string, in one of the following ways:

``IO<EOF>``
     End-of-file marker. This is printed when :func:`w_io_eof()` would
     return ``true`` for the value.

``IO<string>``
     This format is used for errors, the *string* describes the error.
     Most of the time the error strings correspond to the values obtained
     by using ``strerror (w_io_result_error (value))`` on the value.

``IO<number>``
     This format is used for successful operations, the *number* is the
     amount of bytes that were handled during the input/output operation,
     and it can be zero, e.g. for the result of :func:`w_io_close()`.


Reusing
~~~~~~~

The main entry point of the formatting mechanism is the
:func:`w_io_format()` function, which works for any kind of output stream.
To allow for easier integration with other kinds of output, an alternate
version of the function, :func:`w_io_formatv()`, which accepts a
``va_list``, is provided as well.

By providing a custom output stream implementation, it is possible to reuse
the formatting mechanism for your own purposes. The :func:`w_buf_format()`
function, which writes formatted data to a buffer, is implemented using this
technique.

The following example implements a function similar to ``asprintf()``, which
allocates memory as needed to fit the formatted output, using
:func:`w_io_formatv()` in combination with a :type:`w_io_buf_t` stream:

.. code-block:: c

     char*
     str_format (const char *format, ...)
     {
         // Using NULL uses a buffer internal to the w_io_buf_t.
         w_io_buf_t buffer_io;
         w_io_buf_init (&buffer_io, NULL, false);

         // Writing to a buffer always succeeds, the result can be ignored.
         va_list args;
         va_start (args, format);
         W_IO_NORESULT (w_io_formatv ((w_io_t*) &buffer_io, format, args));
         va_end (args);

         // The data area of the buffer is heap allocated, so it is safe to
         // return a pointer to it even when the w_io_buf_t is in the stack.
         return w_io_buf_str (&buffer_io);
     }


Types
-----

.. c:type:: w_io_result_t

   Represents the result of an input/output operation, which is one of:

   - An error, which signals the failure of the operation. The
     :func:`w_io_failed()` can be used to check whether an input/output
     operation failed. If case of failure, the error code can be obtained
     using :func:`w_io_result_error()`; otherwise the operation succeeded
     and it can have one of the other values.

   - An indication that the end-of-file marker has been reached — typically
     used when reading data from a stream. The :func:`w_io_eof()` function
     can be used to check for the end-of-file marker.

   - A successful operation, indicating the amount of data involved. This
     is the case when an operation neither failed, neither it is the
     end-of-file marker. The amount of bytes handled can be retrieved using
     :func:`w_io_result_bytes()`.

   It is possible to obtain a textual representation of values of this type
   by using the ``$R`` *format specifier* with any of the functions that
   use the :ref:`formatted-output` system.

.. c:type:: w_io_t

   Represents an input/output stream.


Macros
------

.. c:macro:: W_IO_RESULT(bytes)

   Makes a :type:`w_io_result_t` value which indicates a successful operation
   which handled the given amount of `bytes`.

.. c:macro:: W_IO_RESULT_ERROR(error)

   Makes a :type:`w_io_result_t` value which indicates a failure due to given
   `error`.

.. c:macro:: W_IO_RESULT_EOF

   Makes a :type:`w_io_result_t` value which indicates a successful operation
   that reached the end-of-file marker.

.. c:macro:: W_IO_RESULT_SUCCESS

   Makes a :type:`w_io_result_t` value which indicates a successful operation.

.. c:macro:: W_IO_CHAIN (result, expression)

   This macro expands to code that evaluates an `expression`, which must
   return a :type:`w_io_result_t` value. If the operation failed, it
   will cause the current function to return the error, otherwise the
   amount of bytes returned by :func:`w_io_result_bytes()` are added
   to the amount in the variable of type :func:`w_io_result_t` passed
   as the `result` parameter.

   Typical usage involves using the macro is inside a function that
   performs a number of reads (or writes), and the overall status of a
   “chain” of input/output operations is to be reported back as a result.

   The expanded macro is roughly equivalent to:

   .. code-block:: c

        w_io_result_t expr_result = expression;
        if (w_io_failed (expr_result))
            return expr_result;
        result.bytes += w_io_result_bytes (expr_result);

   As an example, consider a data stream which contains “messages” of the
   form ``SIZE:DATA``, with the *SIZE* of the message encoded as a plain
   text integer, followed by a colon, and *SIZE* bytes of arbitrary data.
   The following function reads such messages, one at a time, returning
   the result of reading from the `input` as a :type:`w_io_result_t` value
   —which can be checked for end-of-file or errors—, and returning the
   *DATA* in a `buffer` and its expected `size`:

   .. code-block:: c

        w_io_result_t
        read_message (w_io_t *input, w_buf_t *message, unsigned long *size)
        {
            w_io_result_t bytes = W_IO_RESULT (0);

            // Read the length of the record. If reading fails, the macro
            // causes the function to return early with an error result.
            // The amount of bytes read is added to "bytes".
            W_IO_CHAIN (bytes, w_io_fscan_ulong (input, size));

            // Read the separator. Again: the macro causes an early error
            // return on failure, or incrementing "bytes" on success.
            char separator = '\0';
            W_IO_CHAIN (bytes, w_io_read (input, &separator, 1));
            if (separator != ':')
                return W_IO_RESULT_ERROR (EBADMSG);

            // Read the message contents. Again: the macro causes an early
            // error on failure, or incrementing "bytes" on success.
            w_buf_resize (message, record_size);
            W_IO_CHAIN (bytes, w_io_read (input, w_buf_data (message), *size);

            return bytes;  // Returns the total amount of bytes read.
        }


.. c:macro:: W_IO_CHECK (expression)

   This macro expands to code that evaluates an `expression`, which must
   return a :type:`w_io_result_t` value. If the operation failed, it will
   cause the current function to return the error. It is roughly equivalent
   to:

   .. code-block:: c

        w_io_result_t expr_result = expression;
        if (w_io_failed (expr_result))
            return expr_result;

.. c:macro:: W_IO_CHECK_RETURN (expression, value)

   Similar to :macro:`W_IO_CHECK`, but instead of returning a
   :type:`w_io_result_t` if the `expression` fails to perform input/output,
   the given `value` is returned instead. It is roughly equivalent to:

   .. code-block:: c

        if (w_io_failed (expression))
            return value;

.. c:macro:: W_IO_NORESULT (expression)

   This macro expands to code that evaluates an `expression`, which must
   return a :type:`w_io_result_t` value, and ignoring that result value.
   This is typically used when a an input/output operation is known to never
   fail, and the result status will not be checked, or when the operation
   might fail, but it does not matter whether it did.


.. _wio-functions:

Functions
---------

.. c:function:: void w_io_init (w_io_t *stream)

   Initializes a base input/output `stream`.

.. c:function:: w_io_result_t w_io_close (w_io_t *stream)

   Closes an input/output `stream`.

.. c:function:: w_io_result_t w_io_read (w_io_t *stream, void *buffer, size_t count)

   Reads up to `count` bytes from the an input `stream`, placing the data in
   in memory starting at `buffer`.

   Passing a `count` of zero always succeeds and has no side effects.

   If reading succeeds, the amount of bytes read may be smaller than the
   requested `count`. The reason may be that the end-of-file marker has been
   reached (and it will be notified at the next attempt of reading data), or
   because no more data is available for reading at the moment.

.. c:function:: w_io_result_t w_io_write (w_io_t *stream, const void *buffer, size_t count)

   Writes up to `count` bytes from the data in memory starting at `buffer` to
   an output `stream`.

   Passing a `count` of zero always succeeds and has no side effects.

.. c:function:: int w_io_getchar (w_io_t *stream)

   Reads the next character from a input `stream`.

   If the enf-of-file marker is reached, :data:`W_IO_EOF` is returned.
   On errors, negative values are returned.

.. c:function:: w_io_result_t w_io_putchar (w_io_t *stream, int character)

   Writes a `character` to an output `stream`.

.. c:function:: void w_io_putback (w_io_t *stream, int character)

   Pushes a `character` back into an input `stream`, making it available
   during the next read operation.

   .. warning:: Pushing more than one character is not supported, and only
      the last pushed one will be saved.

.. c:function:: w_io_result_t w_io_flush (w_io_stream *stream)

   For an output `stream`, forces writing buffered data to the stream.

   For in input `stream`, discards data that may have been fetched from the
   stream but still not consumed by the application.

.. c:function:: int w_io_get_fd (w_io_t *stream)

   Obtains the underlying file descriptor used by a `stream`.

   .. warning:: Not all types of input/output streams have an associated file
      descriptor, and a negative value will be returned for those.

.. c:function:: w_io_result_t w_io_format (w_io_t *stream, const char *format, ...)

   Writes data with a given `format` to an output `stream`.
   The amount of consumed arguments depends on the `format` string.

   See :ref:`formatted-output` for more information.

.. c:function:: w_io_result_t w_io_formatv (w_io_t *stream, const char *format, va_list arguments)

   Writes data with a given `format` to an output `stream`, consuming the
   needed additional `arguments` from the supplied ``va_list``.
   The amount of consumed arguments depends on the `format` string.

   See :ref:`formatted-output` for more information.

.. c:function:: w_io_result_t w_print (format, ...)

   Writes data in the given `format` to the standard output stream
   ``w_stdout``. The amount of consumed arguments depends on the
   `format` string.

   See :ref:`formatted-output` for more information.

.. c:function:: w_io_result_t w_printerr (format, ...)

   Writes data in the given `format` to the standard error stream
   ``w_stderr``. The amount of consumed arguments depends on the
   `format` string.

   See :ref:`formatted-output` for more information.

.. c:function:: w_io_result_t w_io_read_until (w_io_t *stream, w_buf_t *buffer, w_buf_t *overflow, int character, unsigned read_bytes)

   Reads data from a `stream` until a certain `character` is read. Data is
   read in chunks of `read_bytes` size, and placed in a `buffer`, with the
   excess characters placed in an `overflow` buffer, which can be passed
   back to continue scanning for the stop `character` in subsequent calls.

   Passing zero for the `read_bytes` size will make the function use a default
   chunk size — usually the size of a memory page.

   This function is intended to read records of data of variable size which
   are separated using a certain character as a delimiter. For example,
   usually `fortune <https://en.wikipedia.org/wiki/Fortune_%28Unix%29>`__
   data files have items separated by ``%`` characters. The following
   function would read such a file:

   .. code-block:: c

        void read_fortunes (w_io_t *input) {
            w_buf_t fortune = W_BUF;
            w_buf_t overflow = W_BUF;

            for (;;) {
                w_io_result_t r = w_io_read_line (input, &fortune, &overflow, 0);

                if (w_io_failed (r)) {
                    w_printerr ("Error reading fortunes: $R\n" r);
                    break;
                }
                if (w_io_eof (r))
                    break;

                handle_fortune (&fortune);
            }

            w_buf_clear (&overflow);
            w_buf_clear (&fortune);
        }

.. c:function:: bool w_io_failed (w_io_result_t result)

   Checks whether the `result` of an input/output operation was a failure.

   If the `result` happens to be a failure, :func:`w_io_result_error()` can be
   used to retrieve the error code.

.. c:function:: bool w_io_eof (w_io_result_t result)

   Checks whether the `result` of an input/output operation was the
   end-of-file marker.

.. c:function:: int w_io_result_error (w_io_result_t result)

   Obtains the code of the error that caused an input/output operation to
   return a failure `result`.

   This function only returns meaningful values when `result` indicates a
   failed operation. This condition can be checked using
   :func:`w_io_failed()`.

.. c:function:: size_t w_io_result_bytes (w_io_result_t result)

   Obtains the amount of bytes which were involved as the `result` of an
   input/output operation.

   When the `result` signals a failed operation, or the end-of-file marker,
   the returned value is always zero.

.. c:function:: w_io_result_t w_io_read_line (w_io_t *stream, w_buf_t *line, w_buf_t *overflow, unsigned read_bytes)

   Reads a `line` of text from an input `stream`.

   This is a convenience function that calls :func:`w_io_read_until()` passing
   ``'\n'`` as delimiter character.

