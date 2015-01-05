
.. _wlist:

List Container
==============

The implementation uses a doubly-linked list, meaning that most
operations are very efficient, and the list can be traversed both
forward and backward.

The functions which use numeric indexes to refer to elements in
the list can be slow, and should be avoided if possible:
:func:`w_list_at()`, :func:`w_list_insert_at()`,
and :func:`w_list_del_at()`. Negative numeric indexes can be
passed to functions, with the same meaning as in Python: ``-1``
refers to the last element, ``-2`` to the element before the last,
and so on.

If a list is meant to contain objects (see :ref:`wobj`), it is possible
to let the list reference-count the objects (using :func:`w_obj_ref()`
and :func:`w_obj_unref()`) by passing ``true`` when creating a list
with :func:`w_list_new()`. If enabled, whenever an item is added to the
list, its reference count will be increased, and it will be decreased
when the item is removed from the list.

Usage
-----

.. code-block:: c

     w_list_t *fruits = w_list_new (false);

     w_list_append (fruits, "apples");
     w_list_append (fruits, "bananas");

     w_list_foreach (item, fruits)  // Prints "apples bananas "
         w_print ("$s ", (const char*) *item);

     w_list_insert_after (fruits, w_list_first (fruits), "pears");
     // fruits = {"apples", "pears", "bananas"}

     w_list_del_head (fruits);
     // fruits = {"pears", "bananas"}

     w_list_foreach_reverse (item, fruits)  // Prints "bananas pears "
         w_print ("$s ", (const char*) *item);

     w_obj_unref (fruits);  // Decrease the reference counter, frees the list.


Types
-----

.. c:type:: w_list_t

   Object type of a list container.


Macros
------

.. c:macro:: w_list_foreach (iterator, w_list_t *list)

   Defines a loop over all items in a `list`.

   Typical usage:

   .. code-block:: c

        w_list_t *list = make_string_list ();
        w_list_foreach (i, list) {
            w_io_format (w_stdout, "$s\n", (const char*) *i);
        }

.. c:macro:: w_list_foreach_reverse (iterator, w_list_t *list)

   Defines a loop over all items in a `list`, in reverse order.

   Typical usage:

   .. code-block:: c

        w_list_t *list = make_string_list ();
        w_list_foreach_reverse (i, list) {
            w_io_format (w_stdout, "$s\n", (const char*) *i);
        }


Functions
---------

.. c:function:: w_list_t* w_list_new (bool reference_counted)

   Creates a new list, in which elements are optionally `reference_counted`.

.. c:function:: void w_list_clear (w_list_t *list)

   Clears a `list`, removing all of its elements.

.. c:function:: void w_list_push_tail (w_list_t *list, void *element)

   Appends an `element` to the end of a `list`.

.. c:function:: void w_list_push_head (w_list_t *list, void *element)

   Inserts an `element` at the beginning of a `list`.

.. c:function:: void* w_list_pop_head (w_list_t *list)

   emoves the element at the beginning of a `list` and returns it.

   Note that this **will not** decrease the reference counter when reference
   counting is enabled: it is assumed that the caller will use the returned
   item.

.. c:function:: void* w_list_pop_tail (w_list_t *list)

   Removes the element at the end of a `list` and returns it.

   Note that this **will not** decrease the reference counter when reference
   counting is enabled: it is assumed that the caller will use the returned
   item.

.. c:function:: void* w_list_at (const w_list_t *list, long index)

   Obtains the value stored in a `list` at a given `index`. Negative indexes
   count from the end of the list.

.. c:function:: void* w_list_head (const w_list_t *list)

   Obtains the element at the first position of a `list`.

.. c:function:: void* w_list_tail (const w_list_t *list)

   Obtains the element at the last position of a `list`.

.. c:function:: w_iterator_t w_list_first (const w_list_t *list)

   Obtains an iterator pointing to the first element of a `list`.

.. c:function:: w_iterator_t w_list_last (const w_list_t *list)

   Obtains an iterator pointing to the last element of a `list`

.. c:function:: w_iterator_t w_list_next (const w_list_t *list, w_iterator_t iterator)

   Makes an `iterator` to an element of a `list` point to the next element in
   the list, and returns the updated iterator.

.. c:function:: w_iterator_t w_list_prev (const w_list_t *list, w_iterator_t iterator)

   Makes an `iterator` to an element of a `list` point to the previous element
   in the list, and returns the updated iterator.

.. c:function:: void w_list_insert_before (w_list_t *list, w_iterator_t position, void *element)

   Inserts an `element` in a `list` before a particular `position`.

.. c:function:: void w_list_insert_after (w_list_t *list, w_iterator_t position, void *element)

   Inserts an `element` in a `list` after a particular `position`.

.. c:function:: void w_list_insert_at (w_list_t *list, long index, void *element)

   Inserts an `element` in a `list` at a given `index`..

   Note that the operation is optimized for some particular indexes like
   ``0`` (first position) and ``-1`` (last position), bu in general this
   function runs in *O(n)* time depending on the size of the list.

.. c:function:: void w_list_del (w_list_t *list, w_iterator_t position)

   Deletes the element at a given `position` in a `list`.

.. c:function:: void w_list_del_at (w_list_t *list, long index)

   Deletes the element at a given `index` in a `list`.

.. c:function:: size_t w_list_size (const w_list_t *list)

   Obtains the number of elements in a `list`.

.. c:function:: bool w_list_is_empty (const w_list_t *list)

   Checks whether a `list` is empty.

.. c:function:: void w_list_del_head (w_list_t *list)

   Deletes the element at the beginning of a `list`.

   Contrary to :func:`w_list_pop_head()`, the element is **not** returned,
   and the reference counter is decreased (if reference counting is enabled).

.. c:function:: void w_list_del_tail (w_list_t *list)

   Deletes the element at the end of a `list`.

   Contrary to :func:`w_list_pop_tail()`, the element is **not** returned,
   and the reference counter is decreased (if reference counting is enabled).

.. c:function:: void w_list_insert (w_list_t *list, w_iterator_t position, void *element)

   Alias for :func:`w_list_insert_before()`.

.. c:function:: void w_list_append (w_list_t *list, void *element)

   Alias for :func:`w_list_push_tail()`.

.. c:function:: void w_list_pop (w_list_t *list, void *element)

   Alias for :func:`w_list_pop_tail()`.

