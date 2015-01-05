
.. _wio-socket:

Input/Output on Sockets
=======================

Provides support for using the :ref:`stream functions <wio-functions>` to
read and write to and from sockets.

The following kinds of sockets are supported:

- TCP sockets.
- Unix sockets.


Types
-----

.. c:type:: w_io_socket_t

   Performs input/output on sockets.


Functions
---------

.. c:function:: bool w_io_socket_init (w_io_socket_t *stream, w_io_socket_kind_t kind, ...)

   Initializes a socket `stream` (possibly allocated in the stack).

   For a description of the additional function arguments, please check the
   documentation for :func:`w_io_socket_open()`.

.. c:function:: w_io_t* w_io_socket_open (w_io_socket_kind_t kind, ...)

   Create a new socket of a given `kind`.

   Sockets are created in a state in which they can be used both for
   client and server sockets:

   - :func:`w_io_socket_connect()` will put the socket in client mode
     and connect it to the specified address.

   - :func:`w_io_socket_serve()` will put the socket in server mode, and
     start listening for connections at the specified address.

   The parameters that need to be passed to this function vary depending on
   the `kind` of the socket being created.

   **Unix sockets:**
        Pass ``W_IO_SOCKET_UNIX`` as `kind`, and the path in the file system
        where the socket is to be created (or connected to).

   **TCP sockets:**
        Pass ``W_IO_SOCKET_TCP4`` as `kind`, plus the IP address (as a string)
        and the port to use (or to connect to).

.. c:function:: bool w_io_socket_connect (w_io_socket_t *socket)

   Connect a `socket` to a server.

   This makes a connection to the host specified when creating the
   socket with :func:`w_io_socket_open()`, and puts it in client mode.
   Once the socket is successfully connected, read and write operations
   can be performed in the socket.

   The return value indicates whether the connection was successful.

.. c:function:: bool w_io_socket_serve (w_io_socket_t *socket, w_io_socket_serve_mode_t mode, bool (*handler) (w_io_socket_t*))

   Serves requests using a `socket`. This function will start a loop accepting
   connections, and for each connection an open socket will be passed to the
   given `handler` function. The `mode` in which each request is served can be
   specified:

   - ``W_IO_SOCKET_SINGLE``: Each request is served by calling directly and
     waiting for it to finish. This makes impossible to serve more than one
     request at a time.

   - ``W_IO_SOCKET_THREAD``: Each request is served in a new thread. The
     handler is invoked in that thread.

   - ``W_IO_SOCKET_FORK``: A new process is forked for each request. The
     handler is invoked in the child process.

.. c:function:: bool w_io_socket_send_eof (w_io_socket_t *socket)

   Half-close a `socket` on the write direction. This closes the socket,
   but only in writing one direction, so other endpoint will think that
   the end of the stream was reached (thus the operation is conceptually
   equivalent to sending and “end of file marker”). Read operations can
   still be performed in a socket which was half-closed using this
   function.

   Note that for completely closing the socket, :func:`w_io_close()`
   should be used instead.

   The return value indicates whether the half-close was successful.

.. c:function:: const char* w_io_socket_unix_path (w_io_socket_t *socket)

   Obtain the path in the filesystem for an Unix socket.

   Note that the result is undefined if the socket is of any other kind than
   an Unix socket.

