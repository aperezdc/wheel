/*
 * wio-sock-echo-server.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
#include <errno.h>


static char *socketpath = NULL;
static const w_opt_t options[] = { W_OPT_END };

static void
set_socketpath (void *path, void *ctx)
{
    w_unused (ctx);
    socketpath = (char*) path;
}


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif /* !BUFFER_SIZE */


int
main (int argc, char **argv)
{
    w_io_t *ioo, *ioi, *ios;
    char buf[BUFFER_SIZE];
    ssize_t ret;

    w_opt_parse (options, set_socketpath, NULL, argc, argv);

    if (!socketpath) {
        socketpath = "/tmp/w-echo";
    }

    if (!(ios = w_io_socket_open (W_IO_SOCKET_UNIX, socketpath))) {
        w_die ("Problem creating server socket: %s\n", strerror (errno));
    }

    w_io_socket_connect ((w_io_socket_t*) ios);

    ioi = w_io_unix_open (STDIN_FILENO);
    ioo = w_io_unix_open (STDOUT_FILENO);

    while ((ret = w_io_read (ioi, buf, BUFFER_SIZE)) > 0) {
        w_io_write (ios, buf, ret);
    }
    w_io_socket_send_eof ((w_io_socket_t*) ios);

    while ((ret = w_io_read (ios, buf, BUFFER_SIZE)) > 0) {
        w_io_write (ioo, buf, ret);
    }

    w_obj_unref (ioi);
    w_obj_unref (ioo);
    w_obj_unref (ios);

    return EXIT_SUCCESS;
}
