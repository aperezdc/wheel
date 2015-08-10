/*
 * wtask-listener.c
 * Copyright (C) 2015 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"

static char *bind_spec = "tcp:9000";

static const w_opt_t options[] = {
    { 1, 'b', "bind", W_OPT_STRING, &bind_spec,
      "Bind address (default: 'tcp:9000')" },
    W_OPT_END
};


static void
conn_handler (w_task_listener_t *listener,
              w_io_t            *socket)
{
    w_unused (listener);

    w_printerr ("$s: Connection accepted\n", w_task_name ());

    for (;;) {
        char buf[100];
        w_io_result_t r = w_io_read (socket, buf, sizeof (buf));
        if (w_io_eof (r)) {
            break;
        } else if (w_io_failed (r)) {
            w_printerr ("$s: Read error ($R)\n", w_task_name (), r);
        } else {
            W_IO_NORESULT (w_io_write (w_stderr, buf, w_io_result_bytes (r)));
            W_IO_NORESULT (w_io_write (socket, buf, w_io_result_bytes (r)));
        }
    }
    W_IO_NORESULT (w_io_close (socket));
}


int
main (int argc, char *argv[])
{
    w_opt_parse (options, NULL, NULL, NULL, argc, argv);

    w_task_listener_t *listener =
            w_task_listener_new (bind_spec, conn_handler, NULL);
    w_task_t *task = w_task_prepare (w_task_listener_run, listener, 16384);
    w_task_set_name (task, "Echo");

    w_task_run_scheduler ();
    w_obj_unref (listener);

    return 0;
}
