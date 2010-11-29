/*
 * wio-sock-echo-server.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <errno.h>
#include <unistd.h>


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif /* !BUFFER_SIZE */


static wbool
serve_request (w_io_socket_t *io)
{
    char buf[BUFFER_SIZE];
    ssize_t ret;

    printf ("BEGIN REQUEST\n");

    while ((ret = w_io_read ((w_io_t*) io, buf, BUFFER_SIZE)) > 0) {
        write (STDOUT_FILENO, buf, ret);
        w_io_write ((w_io_t*) io, buf, ret);
    }

    w_io_socket_send_eof (io);

    if (ret < 0) {
        fprintf (stderr, "Error: %s\n", strerror (errno));
    }

    printf ("END REQUEST\n");

    return W_YES;
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

    w_opt_parse (options, NULL, NULL, argc, argv);

    if (!(io = w_io_socket_open (W_IO_SOCKET_TCP4, serverhost, serverport))) {
        w_die ("Problem creating socket: %s\n", strerror (errno));
    }

    if (!w_io_socket_serve ((w_io_socket_t*) io,
                            W_IO_SOCKET_SINGLE,
                            serve_request))
    {
        w_die ("Could not serve: %s\n", strerror (errno));
    }

    w_obj_unref (io);

    return EXIT_SUCCESS;
}
