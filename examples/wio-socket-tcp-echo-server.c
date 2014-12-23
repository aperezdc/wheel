/*
 * wio-sock-echo-server.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <unistd.h>


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif /* !BUFFER_SIZE */


static bool
serve_request (w_io_socket_t *io)
{
    W_IO_NORESULT (w_io_format (w_stdout, "BEGIN REQUEST\n"));

    char buf[BUFFER_SIZE];
    w_io_result_t r;

    while (w_io_result_bytes (r = w_io_read ((w_io_t*) io, buf, BUFFER_SIZE)) > 0) {
        write (STDOUT_FILENO, buf, w_io_result_bytes (r));
        W_IO_NORESULT (w_io_write ((w_io_t*) io, buf, w_io_result_bytes (r)));
    }

    w_io_socket_send_eof (io);

    if (w_io_failed (r)) {
        W_IO_NORESULT (w_io_format (w_stderr, "Error: $E\n"));
    }

    W_IO_NORESULT (w_io_format (w_stdout, "END REQUEST\n"));

    return true;
}


static char *serverhost = NULL;
static int   serverport = 9000;

static const w_opt_t options[] = {
    { 1, 'p', "port", W_OPT_INT, &serverport,
      "TCP port to bind server to (9000)" },

    { 1, 'H', "host", W_OPT_STRING, &serverhost,
      "IP address to bind to (0.0.0.0)"} ,

    W_OPT_END
};


int
main (int argc, char **argv)
{
    w_io_t *io;

    w_opt_parse (options, NULL, NULL, NULL, argc, argv);

    if (!(io = w_io_socket_open (W_IO_SOCKET_TCP4, serverhost, serverport))) {
        w_die ("Problem creating socket: $E\n");
    }

    if (!w_io_socket_serve ((w_io_socket_t*) io,
                            W_IO_SOCKET_SINGLE,
                            serve_request))
    {
        w_die ("Could not serve: $E\n");
    }

    w_obj_unref (io);

    return EXIT_SUCCESS;
}
