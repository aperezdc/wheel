/*
 * wloop-cat.c
 * Copyright (C) 2012-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <unistd.h>
#include <errno.h>


static bool
input_data_ready (w_event_loop_t *loop, w_event_t *event)
{
    char buffer[32];
    ssize_t rb;

    w_unused (loop);
    w_unused (event);

    while ((rb = read (event->fd, buffer, w_lengthof (buffer))) > 0)
        write (STDOUT_FILENO, buffer, rb);

    return !(rb < 0 && errno == EAGAIN) || (size_t) rb < w_lengthof (buffer);
}


int
main (int argc, char *argv[])
{
    w_event_loop_t *loop;
    w_event_t      *event;
    bool            success;

    w_unused (argc);
    w_unused (argv);

    loop  = w_event_loop_new ();
    event = w_event_new (W_EVENT_FD,
                         input_data_ready,
                         STDIN_FILENO,
                         W_EVENT_IN);

    if (w_event_loop_add (loop, event)) {
        w_obj_unref (event);
        W_IGNORE_RESULT (w_io_format (w_stderr, "$s: $E\n", argv[0]));
        return EXIT_FAILURE;
    }

    w_obj_unref (event);

    success = w_event_loop_run (loop);
    w_obj_unref (loop);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

