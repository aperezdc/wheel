
Input/Output Streams
====================

.. _formatted-output:

Formatted output
----------------


Types
-----

.. c:type:: w_io_result_t

   Represents the result of an input/output operation, which is one of:

   - A successful operation, optionally indicating the amount of data
     involved.

   - An indication that the end-of-file marker has been reached — typically
     used when reading data from a stream.

   - An error, which signals the failure of the operation.

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

