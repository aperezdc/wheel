
.. _wio-unix:

Input/Output on Unix file descriptors
=====================================

Once a Unix stream object has been initialized, they can be operated used
the common :ref:`stream functions <wio-functions>`.


Types
-----

.. c:type:: w_io_unix_t

   Performs input/output using Unix file descriptors.


Functions
---------

.. c:function:: w_io_t* w_io_unix_open (const char *path, int mode, unsigned permissions)

   Creates a stream object to be used with an Unix file descriptor by opening
   the file at `path` with the given `mode` and `permissions`.

   This is a convenience function that calls ``open()`` and then uses
   :func:`w_io_unix_open_fd()`.

   If opening the file fails, ``NULL`` is returned.

.. c:function:: w_io_t* w_io_unix_open_fd (int fd)

   Creates a stream object to be used with an Unix file descriptor.

.. c:function:: bool w_io_unix_init (w_io_unix_t *stream, const char *path, int mode, unsigned permissions)

   Initializes a stream object (possibly allocated in the stack) to be used
   with an Unix file descriptor by opening the file at `path` with the given
   `mode` and `permissions`.

   This is a convenience function that calls ``open()`` and then uses
   :func:`w_io_unix_init_fd()`.

   The return value indicates whether the file was opened successfully.

.. c:function:: void w_io_unix_init_fd (w_io_unix_t *stream, int fd)

   Initializes a stream object (possibly allocated in the stack) to be used
   with an Unix file descriptor.

