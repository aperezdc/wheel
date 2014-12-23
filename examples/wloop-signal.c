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
    printf ("signal: %i, tstamp = %f\n",
            event->signum, w_event_loop_now (loop));
    return event->signum == SIGINT;
}


int
main (int argc, char *argv[])
{
    w_event_loop_t *loop;
    w_event_t      *event;
    bool            success;

    w_unused (argc);
    w_unused (argv);

    printf ("press Ctrl-C! or send HUP to %lu\n", (unsigned long) getpid ());

    loop  = w_event_loop_new ();

    event = w_event_new (W_EVENT_SIGNAL, signal_arrived, SIGINT);
    if (w_event_loop_add (loop, event))
        w_die ("Could not register SIGINT event: $E\n");
    w_obj_unref (event);

    event = w_event_new (W_EVENT_SIGNAL, signal_arrived, SIGHUP);
    if (w_event_loop_add (loop, event))
        w_die ("Could not register SIGHUP event: $E\n");
    w_obj_unref (event);

    success = w_event_loop_run (loop);
    w_obj_unref (loop);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

