/*
 * wio-socket-httpget.c
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
http_get (const char *host)
{
    w_printerr ("Host: $s\n", host);

    struct hostent *resolved = gethostbyname2 (host, AF_INET);
    if (!resolved) {
        w_printerr ("Cannot resolve host name\n");
        return;
    }

    w_printerr ("Address: $s\n",
                inet_ntoa (*(struct in_addr*) resolved->h_addr));
    w_lobj w_io_t *socket = w_io_socket_open (W_IO_SOCKET_TCP4,
                                              inet_ntoa (*(struct in_addr*) resolved->h_addr),
                                              80);
    if (!socket) {
        w_printerr ("Cannot open socket: $E\n");
        return;
    }

    if (!w_io_socket_connect ((w_io_socket_t*) socket)) {
        w_printerr ("Cannot connect: $E\n");
        return;
    }

    w_io_result_t r = w_io_format (socket,
                                   "GET / HTTP/1.0\r\n"
                                   "Connection: close\r\n"
                                   "Host: $s\r\n"
                                   "\r\n",
                                   host);
    if (w_io_failed (r)) {
        w_printerr ("Write error: $R\n", r);
        return;
    }

    w_printerr ("Written $I bytes\n",
                w_io_result_bytes (r));

    w_io_socket_send_eof ((w_io_socket_t*) socket);

    char buf[512];
    w_buf_t result = W_BUF;

    for (;;) {
        w_io_result_t r = w_io_read (socket, buf, w_lengthof (buf));

        if (w_io_failed (r)) {
            w_printerr ("Read error: $R\n", r);
            return;
        }

        if (w_io_result_bytes (r) > 0) {
            w_buf_append_mem (&result, buf, w_io_result_bytes (r));
        }

        if (w_io_eof (r))
            break;
    }

    size_t to_write = w_min (50U, w_buf_size (&result));
    w_printerr ("Read $I bytes:\n$S…\n\n",
                w_buf_size (&result),
                to_write,
                w_buf_const_data (&result));
}


int
main (int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        http_get (argv[i]);

    return 0;
}
