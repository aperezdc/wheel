/*
 * wmem.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * Memory Utilities
 * ================
 *
 * The functions :func:`w_malloc()`, :func:`w_realloc()` and the function-like
 * macros :func:`w_new()`, :func:`w_new0()`, and  :func:`w_free()` can be used
 * to allocate and release chunks of memory from the heap.
 *
 * The function-like macros :func:`w_alloc()`, :func:`w_alloc0()`, and
 * :func:`w_resize()` can be used to allocate and resize chunks of memory
 * which contain arrays of elements of the same type.
 *
 * It is highly encourages to use these instead of any other dynamic allocation
 * facilities, as they do additional checks and will take care of aborting the
 * running program with a meaningful message in case of out-of-memory
 * conditions.
 *
 *
 * Macros
 * ------
 */

/*~M w_lmem
 *
 * Marks a variable as being pointer to a heap-allocated chunk of memory which
 * must be freed by calling :func:`w_free()` on it when the variable goes out
 * of scope.
 *
 * For example:
 *
 * .. code-block:: c
 *
 *      if (show_frob ()) {
 *          // frob_message() allocates memory for the message using w_malloc()
 *          w_lmem char *frob_msg = get_frob_message ();
 *
 *          w_print ("Frob: $s\n", frob_msg);
 *
 *          // After this point, "frob_msg" goes out of scope, so w_free()
 *          // is called automatically on it to free the memory.
 *      }
 *
 * Note that this macro uses variable attributes supported by GCC and Clang to
 * implement this behavior. When building your program with any other compiler
 * a compilation error will be generated if this macro is used.
 */

/*~M w_lobj
 *
 * Marks a variable as being a pointer which holds a reference to to a
 * heap-allocated object, and that :func:`w_obj_unref()` will be called on it
 * when the variable goes out of scope.
 *
 * Usage example:
 *
 * .. code-block:: c
 *
 *      if (do_frob ()) {
 *          // Keep a reference to a frob_t, increasing the reference counter.
 *          w_lobj frob_t *frob_obj = w_obj_ref (get_frob_object ());
 *
 *          handle_frob (frob_obj);
 *
 *          // After this point, "frob_obj" goes out of scope, so w_obj_unref()
 *          // is called automatically on it to decrease the reference counter.
 *      }
 *
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*~f void* w_malloc(size_t size)
 *
 * Allocates a chunk of memory from the heap of a given `size` and returns a
 * pointer to it. The returned pointer is guaranteed to be valid.
 *
 * If it was not possible to allocate memory, a fatal error is printed to
 * standard error and execution aborted.
 */
void*
w_malloc (size_t sz)
{
    void *p = malloc(sz);
    if (w_unlikely (p == NULL)) {
        W_FATAL ("virtual memory exhausted (tried to allocate $L bytes)\n",
                 (unsigned long) sz);
    }

    /* FIXME This should be removed */
    memset(p, 0x00, sz);
    return p;
}


/*~f void* w_realloc(void *address, size_t new_size)
 *
 * Resizes a chunk of memory from the heap at `address` from its
 * current size to a `new_size`.
 *
 * Note that:
 *
 * - Passing ``NULL`` is valid, and it will allocate memory if the
 *   `new_size` is non-zero. This means that the following two
 *   expressions are equivalent:
 *
 *   .. code-block:: c
 *
 *          void *addr = w_realloc (NULL, 42);
 *          void *addr = w_malloc (42);
 *
 * - Requesting a `new_size` of zero causes memory to be deallocated.
 *   This means that the following two expressions are equivalent:
 *
 *   .. code-block:: c
 *
 *          addr = w_realloc (addr, 0);
 *          w_free (addr);
 */
void*
w_realloc (void *ptr, size_t sz)
{
    if (w_likely (sz)) {
        if (w_unlikely (ptr == NULL)) {
            return w_malloc(sz);
        }
        else {
            ptr = realloc(ptr, sz);
            if (w_unlikely (ptr == NULL)) {
                W_FATAL ("virtual memory exhausted (tried to allocate $L bytes)\n",
                         (unsigned long) sz);
            }
            return ptr;
        }
    }
    else {
        if (w_unlikely (ptr == NULL)) {
            return NULL;
        }
        else {
            free(ptr);
            return NULL;
        }
    }
}


void
w__lmem_cleanup (void *ptr)
{
    void **location = ptr;
    if (location) {
        /* w_free already sets the address to NULL */
        if (*location) {
            w_free (*location);
        }
    }
}


void
w__lobj_cleanup (void *ptr)
{
    void **location = ptr;
    if (location) {
        w_obj_t *obj = *location;
        if (obj) {
            w_obj_unref (obj);
            *location = NULL;
        }
    }
}


/*~f void w_free(void *address)
 *
 * Frees the chunk of heap memory at `address` and sets the pointer
 * to ``NULL``.
 */

/*~f type* w_new(type)
 *
 * Allocates a new chunk of memory from the heap suitable to hold a value
 * of a certain `type`. Note that the pointer is returned casted to the
 * appropriate `type` pointer, and no additional casts are needed:
 *
 * .. code-block:: c
 *
 *      int *value = w_new (int);
 */

/*~f type* w_new0(type)
 *
 * Allocates a zero-filled chunk of memory from the heap suitable to hold
 * a value of a certain `type`. This is equivalent to using :func:`w_new()`,
 * clearing the memory with ``memset()``, and then casting to the appropriate
 * type:
 *
 * .. code-block:: c
 *
 *      int *value = w_new0 (int);
 *      w_print ("$i\n", *value);  // Prints "0"
 */

/*~f type* w_alloc(type, size_t size)
 *
 * Allocates a chunk of memory suitable to hold an array of a given `size` of
 * values of a `type`. Note that the returned pointer is casted to the
 * appropriate `type`, and no additional casts are needed:
 *
 * .. code-block:: c
 *
 *      int *point = w_alloc (int, 2);
 *      point[0] = 42;
 *      point[1] = 14;
 */

/*~f type* w_alloc0(type, size_t size)
 *
 * Allocates a zero-filled chunk of memory suitable to hold an array of a
 * given `size` of values of a `type`. This is equivalent to using
 * :func:`w_alloc()`, clearing the memory with ``memset()``, and then casting
 * to the appropriate type:
 *
 * .. code-block:: c
 *
 *      int *point = w_alloc0 (int, 2);
 *      w_print ("($i, $i)\n", point[0], point[1]); // Prints "(0, 0)"
 */

/*~f type* w_resize(type* address, type, size_t new_size)
 *
 * Resizes a chunk of memory at `address` which contains an array of elements
 * of a given `type` to a `new_size`.
 *
 * .. code-block:: c
 *
 *      int *values = w_alloc (int, 10);
 *      if (need_more_values ())
 *          values = w_resize (values, int, 20);
 */
