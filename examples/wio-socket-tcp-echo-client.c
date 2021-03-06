/*
 * wio-sock-echo-server.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <unistd.h>
#include <errno.h>


static char *serverhost = "127.0.0.1";
static int   serverport = 9000;

static const w_opt_t options[] = {
    { 1, 'p', "port", W_OPT_INT, &serverport,
      "TCP port to connect to (9000)" },

    { 1, 'H', "host", W_OPT_STRING, &serverhost,
      "IP address to connect to (127.0.0.1)"} ,

    W_OPT_END
};


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif /* !BUFFER_SIZE */


int
main (int argc, char **argv)
{
    w_io_t *ios;
    char buf[BUFFER_SIZE];

    w_opt_parse (options, NULL, NULL, NULL, argc, argv);

    if (!(ios = w_io_socket_open (W_IO_SOCKET_TCP4, serverhost, serverport))) {
        w_die ("Problem creating socket: $s\n", strerror (errno));
    }

    w_io_socket_connect ((w_io_socket_t*) ios);

    w_io_result_t r;
    while (w_io_result_bytes (r = w_io_read (w_stdin, buf, BUFFER_SIZE)) > 0) {
        W_IO_CHECK_RETURN (w_io_write (ios, buf, w_io_result_bytes (r)),
                           EXIT_FAILURE);
    }
    w_io_socket_send_eof ((w_io_socket_t*) ios);

    while (w_io_result_bytes (r = w_io_read (ios, buf, BUFFER_SIZE)) > 0) {
        W_IO_NORESULT (w_io_write (w_stdout, buf, w_io_result_bytes (r)));
    }

    w_obj_unref (ios);

    return EXIT_SUCCESS;
}
