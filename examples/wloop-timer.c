/*
 * wloop-cat.c
 * Copyright (C) 2012-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <unistd.h>
#include <signal.h>
#include <errno.h>


static bool
signal_arrived (w_event_loop_t *loop, w_event_t *event)
{
    w_unused (loop);
    w_unused (event);
    /* Receiving an interrupt signal stops the event loop. */
    return event->signum == SIGINT;
}


static bool
timer_triggered (w_event_loop_t *loop, w_event_t *event)
{
    w_unused (event);
    printf ("timer triggered, timestamp: %f\n", w_event_loop_now (loop));
    return false;
}


int
main (int argc, char *argv[])
{
    w_event_loop_t *loop;
    w_event_t      *event;
    bool            success;

    w_unused (argc);
    w_unused (argv);

    W_IGNORE_RESULT (w_io_format (w_stdout, "Press Ctrl-C to stop\n"));

    loop  = w_event_loop_new ();

    event = w_event_new (W_EVENT_SIGNAL,
                         signal_arrived,
                         SIGINT);

    if (w_event_loop_add (loop, event))
        w_die ("Could not register SIGINT event: $E\n");
    w_obj_unref (event);

    /* Do something every two seconds. */
    event = w_event_new (W_EVENT_TIMER,
                         timer_triggered,
                         2.0);

    if (w_event_loop_add (loop, event))
        w_die ("Could not register timer event: $E\n");
    w_obj_unref (event);

    success = w_event_loop_run (loop);
    w_obj_unref (loop);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

