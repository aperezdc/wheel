/*
 * wtask-httpget.c
 * Copyright (C) 2014 aperez <aperez@hikari>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static void
http_get (void *arg)
{
    const char *host = (const char*) arg;

    struct hostent *resolved = gethostbyname2 (host, AF_INET);
    if (!resolved) {
        w_printerr ("$s: Cannot resolve host name: $E\n", w_task_name ());
        return;
    }

    w_printerr ("$s: Address: $s\n",
                w_task_name (),
                inet_ntoa (*(struct in_addr*) resolved->h_addr));
    w_lobj w_io_t *socket = w_io_socket_open (W_IO_SOCKET_TCP4,
                                              inet_ntoa (*(struct in_addr*) resolved->h_addr),
                                              80);
    if (!socket) {
        w_printerr ("$s: Cannot open socket: $E\n", w_task_name ());
        return;
    }

    if (!w_io_socket_connect ((w_io_socket_t*) socket)) {
        w_printerr ("$s: Cannot connect: $E\n", w_task_name ());
        return;
    }

    w_lobj w_io_t *tio = w_io_task_open (socket);
    if (!tio) {
        w_printerr ("$s: Cannot open task I/O: $E\n", w_task_name ());
        return;
    }

    w_io_result_t r = w_io_format (tio,
                                   "GET / HTTP/1.0\r\n"
                                   "Connection: close\r\n"
                                   "Host: $s\r\n"
                                   "\r\n",
                                   host);
    if (w_io_failed (r)) {
        w_printerr ("$s: Write error: $R\n", w_task_name (), r);
        return;
    }

    w_printerr ("$s: Written $I bytes\n",
                w_task_name (),
                w_io_result_bytes (r));

    w_io_socket_send_eof ((w_io_socket_t*) socket);

    char buf[512];
    w_buf_t result = W_BUF;

    for (;;) {
        w_io_result_t r = w_io_read (tio, buf, w_lengthof (buf));

        if (w_io_failed (r)) {
            w_printerr ("$s: Read error: $R\n", w_task_name (), r);
            return;
        }

        if (w_io_result_bytes (r) > 0) {
            w_buf_append_mem (&result, buf, w_io_result_bytes (r));
        }

        if (w_io_eof (r))
            break;
    }

    size_t to_write = w_min (50U, w_buf_size (&result));
    w_printerr ("$s: Read $I bytes:\n$S…\n\n",
                w_task_name (),
                w_buf_size (&result),
                to_write,
                w_buf_const_data (&result));
    w_buf_clear (&result);
}


int
main (int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        w_task_t *task = w_task_prepare (http_get, argv[i], 16384);
        w_task_set_name (task, argv[i]);
    }
    w_task_run_scheduler ();
    return 0;
}
