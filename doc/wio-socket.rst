
.. _wio-socket:

Input/Output on Sockets
=======================

Provides support for using the :ref:`stream functions <wio-functions>` to
read and write to and from sockets.

The following kinds of sockets are supported:

- TCP sockets.
- Unix sockets.


Usage
-----

Client
~~~~~~

To connect a socket to a remote server (and use it as a client), the usual
procedure is as follows:

#. Create the socket with :func:`w_io_socket_open()`.

#. Connect to the remote endpoint using :func:`w_io_socket_connect()`.

#. Exchange data using :func:`w_io_write()` and :func:`w_io_read()`, or any
   other of the :ref:`stream functions <wio-functions>`.

#. *(Optional)* Once no more data needs to be written, a half-close can be
   performed using :func:`w_io_socket_send_eof()`. It will still be
   possible to read data from the stream.

#. Close the socket, using :func:`w_io_close()` or calling
   :func:`w_obj_unref()` on it and letting it be closed and destroyed when
   no more references to the socket are held.

The following code will make an HTTP request and read the response back
into a :type:`w_buf_t`:

.. code-block:: c

     w_buf_t
     http_get (const char *ip_address, unsigned port, const char *resource)
     {
         w_lobj w_io_t *stream = w_io_socket_open (W_IO_SOCKET_TCP4,
                                                   ip_address,
                                                   port);
         if (!stream || !w_io_socket_connect ((w_io_socket_t*) stream))
             w_die ("Cannot create socket and connect to $s\n", ip_address);

         W_IO_NORESULT (w_io_format (stream, "GET $s HTTP/1.0\r\n", resource));
         w_io_socket_send_eof ((w_io_socket_t*) stream);

         w_buf_t response = W_BUF;
         char buf[512];

         for (;;) {
             w_io_result_t r = w_io_read (stream, buf, w_lengthof (buf));
             if (w_io_failed (r))
                 w_die ("Error reading from $s: $R\n", ip_address, r);
             if (w_io_result_bytes (r) > 0)
                 w_buf_append_mem (&response, buf, w_io_result_bytes (r));
             else if (w_io_eof (r))
                 break;
         }

         return response;
     }


Server
~~~~~~

Using a socket to server requests is a bit more convoluted, but still
much easier than using the sockets API directly. The overall procedure
is:

#. Define a request handler function which conforms to the following
   signature:

   .. code-block:: c

         bool (*request_handler) (w_io_socket_t *socket)

#. Create the socket with :func:`w_io_socket_open()`.

#. Start serving requests with :func:`w_io_socket_serve()`. The function
   will bind to the address specified when creating the socket and start
   serving requests. For each request, the request handler function will
   be called with a socket that can be used to handle the request.

The following code implements a simple TCP echo server:

.. code-block:: c

     static bool
     handle_echo_request (w_io_socket_t *socket)
     {
         char buf[512];
         for (;;) {
             w_io_result_t r = w_io_read ((w_io_t*) socket, buf, w_lengthof (buf));
             if (w_io_failed (r))
                 w_die ("Error reading from client: $R\n", r);
             if (w_io_eof (r))
                 break;

             r = w_io_write ((w_io_t*) socket, buf, w_io_result_bytes (r));
             if (w_io_failed (r))
                 w_die ("Error writing to client: $R\n", r);
         }
         w_io_socket_send_eof (socket);

         return true;  // Keep accepting more requests.
     }

     int main (void)
     {
         w_lobj w_io_t *socket = w_io_socket_open (W_IO_SOCKET_TCP4,
                                                   "0.0.0.0",  // Address.
                                                   4242);      // Port.
         if (!socket)
             w_die ("Cannot create socket: $E\n");

         if (!w_io_socket_serve ((w_io_socket_t*) socket,
                                 W_IO_SOCKET_FORK,  // Handle each request in a child process.
                                 handle_echo_request))
             w_dir ("Cannot accept connections: $E\n");

         return 0;
     }


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

