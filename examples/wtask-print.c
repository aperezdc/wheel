/*
 * wtask-print.c
 * Copyright (C) 2014 aperez <aperez@hikari>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"


static void
print_message (void *data)
{
    const char* message = (const char*) data;
    unsigned count = 10;

    while (count--) {
        printf ("%s - %s\n", w_task_name (), message);
        w_task_yield ();
    }
}


int
main (int argc, char *argv[])
{
    int i;

    for (i = 1; i < argc; i++) {
        w_task_prepare (print_message, argv[i], 0);
    }

    w_task_run_scheduler ();
    return 0;
}
