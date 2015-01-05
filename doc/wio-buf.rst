
.. _wio-buf:

Input/Output on Buffers
=======================

Provides support for using the :ref:`stream functions <wio-functions>` to
read and write to and from :type:`w_buf_t` buffers.

Types
-----

.. c:type:: w_io_buf_t

   Performs input/output on a :type:`w_buf_t` buffer.


Functions
---------

.. c:function:: void w_io_buf_init (w_io_buf_t *stream, w_buf_t *buffer, bool append)

   Initialize a `stream` object (possibly allocated in the stack) to be
   used with a `buffer`.

   Passing a ``NULL`` `buffer` will create a new buffer owned by the stream
   object, which can be retrieved using :func:`w_io_buf_get_buffer()`. The
   memory used by this buffer will be freed automatically when the stream
   object is freed. On the contrary, when a valid buffer is supplied, the
   caller is responsible for calling :func:`w_buf_clear()` on it.

   Optionally, the stream position can be setup to `append` data to the
   contents already present in the given `buffer`, insted of overwriting
   them.

.. c:function:: w_io_t* w_io_buf_open (w_buf_t *buffer)

   Creates a stream object to be used with a `buffer`.

   Passing a ``NULL`` `buffer` will create a new buffer owned by the stream
   object, which can be retrieved using :func:`w_io_buf_get_buffer()`. The
   memory used by this buffer will be freed automatically when the stream
   object is freed. On the contrary, when a valid buffer is supplied, the
   caller is responsible for calling :func:`w_buf_clear()` on it.

.. c:function:: w_buf_t* w_io_buf_get_buffer (w_io_buf_t *stream)

   Obtain a pointer to the buffer being used by a `stream`.

.. c:function:: char* w_io_buf_str (w_io_buf_t *stream)

   Obtain a string representation of the contents of the buffer being used by
   a `stream`.

