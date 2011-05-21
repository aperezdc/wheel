/*
 * wio-sock-echo-server.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif /* !BUFFER_SIZE */


static wbool
serve_request (w_io_socket_t *io)
{
    char buf[BUFFER_SIZE];
    ssize_t ret;

    w_io_format (w_stdout, "BEGIN REQUEST\n");

    while ((ret = w_io_read ((w_io_t*) io, buf, BUFFER_SIZE)) > 0) {
        w_io_write (w_stdout, buf, ret);
        w_io_write ((w_io_t*) io, buf, ret);
    }

    w_io_socket_send_eof (io);

    if (ret < 0) {
        w_io_format (w_stderr, "Error: %E\n");
    }

    w_io_format (w_stdout, "END REQUEST\n");

    return W_YES;
}


static char *socketpath = NULL;
static const w_opt_t options[] = { W_OPT_END };

static void
set_socketpath (void *path, void *ctx)
{
    w_unused (ctx);
    socketpath = (char*) path;
}

int
main (int argc, char **argv)
{
    w_io_t *io;

    w_opt_parse (options, set_socketpath, NULL, "[socket-path]", argc, argv);

    if (!socketpath) {
        socketpath = "/tmp/w-echo";
    }

    if (!(io = w_io_socket_open (W_IO_SOCKET_UNIX, socketpath))) {
        w_die ("Problem creating server socket: $E\n");
    }

    if (!w_io_socket_serve ((w_io_socket_t*) io,
                            W_IO_SOCKET_THREAD,
                            serve_request))
    {
        w_die ("Could not serve: $E\n");
    }

    w_obj_unref (io);

    return EXIT_SUCCESS;
}
