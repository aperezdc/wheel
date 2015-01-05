
.. _wio-stdio:

Input/Output on ``FILE*`` streams
=================================

Provides support for using the :ref:`stream functions <wio-functions>` to
read and write to and from ``FILE*`` streams as provided by the C standard
library.

Types
-----

.. c:type:: w_io_stdio_t

   Performs input/output on a ``FILE*`` stream.


Functions
---------

.. c:function:: void w_io_stdio_init (w_io_stdio_t *stream, FILE *stdio_stream)

   Initializes a `stream` object (possibly allocated in the stack) to be used
   with a given `stdio_stream`.

.. c:function:: w_io_t* w_io_stdio_open (FILE *stdio_stream)

   Creates a stream object to be used with a given `stdio_stream`.

