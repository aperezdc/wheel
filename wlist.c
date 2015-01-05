/*
 * wlist.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include "queue.h"
#include <stdarg.h>


TAILQ_HEAD (w_list_head, w_list_entry);

struct w_list_entry
{
    void                      *value;
    TAILQ_ENTRY (w_list_entry) tailq;
};


#define _W_LIST_HE        \
    struct w_list_head *h; \
    struct w_list_entry *e; \
    w_assert (list);         \
    h = w_obj_priv (list, w_list_t)


w_list_t*
w_list_new (bool refs)
{
    w_list_t *list = w_obj_new_with_priv_sized (w_list_t,
                                                sizeof (struct w_list_head));
    struct w_list_head *h = w_obj_priv (list, w_list_t);
    list->refs = refs;
    list->size = 0;
    TAILQ_INIT (h);
    return w_obj_dtor (list, (void (*)(void*)) w_list_clear);
}


void
w_list_clear (w_list_t *list)
{
    _W_LIST_HE;

    while (!TAILQ_EMPTY (h)) {
        e = TAILQ_FIRST (h);
        TAILQ_REMOVE (h, e, tailq);
        if (list->refs)
            w_obj_unref (e->value);
        w_free (e);
    }
    list->size = 0;
}


void
w_list_push_tail (w_list_t *list, void *item)
{
    _W_LIST_HE;

    e = w_new (struct w_list_entry);
    e->value = list->refs ? w_obj_ref (item) : item;

    TAILQ_INSERT_TAIL (h, e, tailq);
    list->size++;
}


void
w_list_push_head (w_list_t *list, void *item)
{
    _W_LIST_HE;

    e = w_new (struct w_list_entry);
    e->value = list->refs ? w_obj_ref (item) : item;

    TAILQ_INSERT_HEAD (h, e, tailq);
    list->size++;
}


void*
w_list_pop_head (w_list_t *list)
{
    void *r;
    _W_LIST_HE;

    w_assert (list->size > 0);

    e = TAILQ_FIRST (h);
    TAILQ_REMOVE (h, e, tailq);
    list->size--;
    r = e->value;
    w_free (e);
    return r;
}


void*
w_list_pop_tail (w_list_t *list)
{
    void *r;
    _W_LIST_HE;

    w_assert (list->size > 0);

    e = TAILQ_LAST (h, w_list_head);
    TAILQ_REMOVE (h, e, tailq);
    list->size--;
    r = e->value;
    w_free (e);
    return r;
}


void*
w_list_at (const w_list_t *list, long index)
{
    size_t pos;
    _W_LIST_HE;

    pos = (index < 0) ? list->size + index : (size_t) index;
    w_assert (pos < list->size);

    e = TAILQ_FIRST (h);
    while (pos--)
        e = TAILQ_NEXT (e, tailq);

    return e->value;
}


void*
w_list_head (const w_list_t *list)
{
    _W_LIST_HE;
    e = TAILQ_FIRST (h);
    return e->value;
}


void*
w_list_tail (const w_list_t *list)
{
    _W_LIST_HE;
    e = TAILQ_LAST (h, w_list_head);
    return e->value;
}


w_iterator_t
w_list_first (const w_list_t *list)
{
    _W_LIST_HE;
    e = TAILQ_FIRST (h);
    return (w_iterator_t) e;
}


w_iterator_t
w_list_last (const w_list_t *list)
{
    _W_LIST_HE;
    e = TAILQ_LAST (h, w_list_head);
    return (w_iterator_t) e;
}


w_iterator_t
w_list_next (const w_list_t *list, w_iterator_t i)
{
    _W_LIST_HE;
    e = (struct w_list_entry*) i;

    if (!i || e == TAILQ_LAST (h, w_list_head))
        return NULL;

    return (w_iterator_t) TAILQ_NEXT (e, tailq);
}


w_iterator_t
w_list_prev (const w_list_t *list, w_iterator_t i)
{
    _W_LIST_HE;
    e = (struct w_list_entry*) i;

    if (!i || e == TAILQ_FIRST (h))
        return NULL;

    return (w_iterator_t) TAILQ_PREV (e, w_list_head, tailq);
}


void
w_list_traverse (const w_list_t *list, w_traverse_fun_t f, void *ctx)
{
    _W_LIST_HE;
    w_assert (f);
    TAILQ_FOREACH (e, h, tailq)
        (*f) (e->value, ctx);
}


void
w_list_traverse_rev (const w_list_t *list, w_traverse_fun_t f, void *ctx)
{
    _W_LIST_HE;
    w_assert (f);
    TAILQ_FOREACH_REVERSE (e, h, w_list_head, tailq)
        (*f) (e->value, ctx);
}


void
w_list_insert_before (w_list_t *list, w_iterator_t i, void *item)
{
    _W_LIST_HE;
    w_unused (h);

    e = w_new (struct w_list_entry);
    e->value = list->refs ? w_obj_ref (item) : item;

    TAILQ_INSERT_BEFORE ((struct w_list_entry*) i, e, tailq);
    list->size++;
}


void
w_list_insert_after (w_list_t *list, w_iterator_t i, void *item)
{
    _W_LIST_HE;

    e = w_new (struct w_list_entry);
    e->value = list->refs ? w_obj_ref (item) : item;

    TAILQ_INSERT_AFTER (h, (struct w_list_entry*) i, e, tailq);
    list->size++;
}


void
w_list_insert_at (w_list_t *list, long index, void *item)
{
    _W_LIST_HE;

    e = w_new (struct w_list_entry);
    e->value = list->refs ? w_obj_ref (item) : item;

    if (index == -1) {
        TAILQ_INSERT_TAIL (h, e, tailq);
    }
    else {
        struct w_list_entry *ref;
        size_t pos = (index < 0) ? list->size + index : (size_t) index;

        w_assert (pos <= list->size);

        ref = TAILQ_FIRST (h);
        while (pos--)
            ref = TAILQ_NEXT (ref, tailq);

        TAILQ_INSERT_BEFORE (ref, e, tailq);
    }
    list->size++;
}


void
w_list_del (w_list_t *list, w_iterator_t i)
{
    _W_LIST_HE;
    e = (struct w_list_entry*) i;
    TAILQ_REMOVE (h, e, tailq);
    if (list->refs)
        w_obj_unref (e->value);
    list->size--;
}


void
w_list_del_at (w_list_t *list, long index)
{
    size_t pos;
    _W_LIST_HE;

    pos = (index < 0) ? list->size + index : (size_t) index;
    w_assert (pos < list->size);

    e = TAILQ_FIRST (h);
    while (pos--)
        e = TAILQ_NEXT (e, tailq);
    TAILQ_REMOVE (h, e, tailq);
    if (list->refs)
        w_obj_unref (e->value);
    list->size--;
}
