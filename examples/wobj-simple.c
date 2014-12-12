/*
 * wobj-simple.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"

W_OBJ (my_obj_t)
{
    w_obj_t parent;
    int     val;
};


static my_obj_t*
my_obj_new (int val)
{
    my_obj_t *obj = w_obj_new (my_obj_t);
    obj->val = val;
    return obj;
}


int
main (int argc, char **argv)
{
    my_obj_t *obj;

    w_unused (argc);
    w_unused (argv);

    obj = my_obj_new (10);
    w_io_format (w_stdout, "$i\n", obj->val);
    w_obj_unref (obj);

    return EXIT_SUCCESS;
}


