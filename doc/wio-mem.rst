
.. _wio-mem:

Input/Output on memory
======================

Provides support for using the :ref:`stream functions <wio-functions>` to
read and write to and from regions of memory of fixed sizes.


Types
-----

.. c:type:: w_io_mem_t

   Performs input/output on a region of memory of a fixed size.


Functions
---------

.. c:function:: void w_io_mem_init (w_io_mem_t *stream, uint8_t *address, size_t size)

   Initializes a `stream` object (possibly located in the stack) to be used
   with a region of memory of a given `size` located at `address`.

.. c:function:: w_io_t* w_io_mem_open (uint8_t *address, size_t size)

   Creates a stream object to be used with a region of memory of a given
   `size` located at `address`.

.. c:function:: uint8_t* w_io_mem_data (w_io_mem_t *stream)

   Obtains the base address to the memory region on which a `stream` operates.

.. c:function:: size_t w_io_mem_size (w_io_mem_t *stream)

   Obtains the size of the memory region on which a `stream` operates.

