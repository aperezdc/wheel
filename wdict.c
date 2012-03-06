/*
 * wdict.c
 * Copyright (C) 2010-2012 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <string.h>

#ifndef W_DICT_DEFAULT_ALLOC
#define W_DICT_DEFAULT_ALLOC 128
#endif /* !W_DICT_DEFAULT_ALLOC */

#ifndef W_DICT_REALLOC_FACTOR
#define W_DICT_REALLOC_FACTOR 10
#endif /* !W_DICT_REALLOC_FACTOR */

#ifndef W_DICT_SIZE_TO_ALLOC_RATIO
#define W_DICT_SIZE_TO_ALLOC_RATIO 1.2
#endif /* !W_DICT_SIZE_TO_ALLOC_RATIO */

#ifndef W_DICT_HASH
#define W_DICT_HASH(_k, _s) (w_str_hash (_k) % ((_s) - 1))
#endif /* !W_DICT_HASH */

#ifndef W_DICT_HASHN
#define W_DICT_HASHN(_k, _s, _n) (w_str_hashl (_k, _n) % ((_s) - 1))
#endif /* !W_DICT_HASHN */

#ifndef W_DICT_KEY_EQ
#define W_DICT_KEY_EQ(_a, _b) (!strcmp((_a), (_b)))
#endif /* !W_DICT_KEY_EQ */

#ifndef W_DICT_KEY_EQN
#define W_DICT_KEY_EQN(_a, _b, _blen) (!strncmp((_a), (_b), (_blen)))
#endif /* !W_DICT_KEY_EQN */


/*
 * XXX  Never, NEVER, change the layout of this struct. If  XXX
 * XXX  you want to add new fields, ADD FIELDS AT THE END.  XXX
 */

struct w_dict_node
{
	void *val;
	char *key;
	w_dict_node_t *next;
	w_dict_node_t *nextNode;
	w_dict_node_t *prevNode;
};


static inline w_dict_node_t*
w_dict_node_new (const char *key, void *val)
{
	w_dict_node_t *node;
	w_assert (key != NULL);

	node = w_new (w_dict_node_t);
	node->key = w_str_dup (key);
	node->val = val;
	return node;
}


static inline w_dict_node_t*
w_dict_node_newn (const char *key, size_t len, void *val)
{
	w_dict_node_t *node;
	w_assert (key != NULL);
	w_assert (len > 0);

	node = w_new(w_dict_node_t);
	node->key = w_str_dupl (key, len);
	node->val = val;
	return node;
}


static inline void
w_dict_node_free (w_dict_node_t *node)
{
	w_assert (node != NULL);
	w_assert (node->key != NULL);
	w_free (node->key);
	w_free (node);
}


static inline void
w_dict_free_nodes(w_dict_t *d)
{
	w_dict_node_t *node = d->first;
	w_dict_node_t *next;

	while (node) {
		next = node->nextNode;
		if (d->refs)
		    w_obj_unref (node->val);
		w_dict_node_free (node);
		node = next;
	}
}


static void
_w_dict_dtor (void *obj)
{
	w_dict_t *d = (w_dict_t*) obj;
	w_assert (d != NULL);
	w_dict_free_nodes (d);
	w_free (d->nodes);
}


w_dict_t*
w_dict_new (w_bool_t refs)
{
	w_dict_t *d = w_obj_new (w_dict_t);
	d->alloc = W_DICT_DEFAULT_ALLOC;
	d->nodes = w_alloc (w_dict_node_t*, d->alloc);
	d->refs  = refs;
	d->size  = 0;
	return w_obj_dtor (d, _w_dict_dtor);
}


void
w_dict_clear (w_dict_t *d)
{
	w_assert (d != NULL);
	w_dict_free_nodes (d);
	memset (d->nodes, 0x00, d->size * sizeof (w_dict_node_t*));

	d->first = NULL;
	d->size  = 0;
}


void*
w_dict_get (const w_dict_t *d, const char *key)
{
	return w_dict_getn (d, key, strlen (key));
}


void*
w_dict_getn (const w_dict_t *d, const char *key, size_t len)
{
	w_dict_node_t *node;
	unsigned hval;

	w_assert (d != NULL);
	w_assert (key != NULL);

	hval = W_DICT_HASHN (key, d->alloc, len);
	node = d->nodes[hval];

	if (node) {
		if (W_DICT_KEY_EQN (node->key, key, len)) {
			return node->val;
		}
		else {
			w_dict_node_t *lastNode = node;
			node = node->next;
			while (node) {
				if (W_DICT_KEY_EQN (node->key, key, len)) {
					lastNode->next = node->next;
					node->next = d->nodes[hval];
					d->nodes[hval] = node;
					return node->val;
				}
				lastNode = node;
				node = node->next;
			}
		}
	}
	return NULL;
}


static inline void
w_dict_rehash (w_dict_t *d)
{
	w_dict_node_t *node = d->first;

	while (node) {
		node->next = NULL;
		node = node->nextNode;
	}

	d->alloc *= W_DICT_REALLOC_FACTOR;
	d->nodes = w_resize (d->nodes, w_dict_node_t*, ++d->alloc);
	memset (d->nodes, 0x00, d->alloc * sizeof (w_dict_node_t*));

	for (node = d->first; node; node = node->nextNode) {
		unsigned hval  = W_DICT_HASH (node->key, d->alloc);
		w_dict_node_t *n = d->nodes[hval];
		if (!n) d->nodes[hval] = node;
		else {
			for (;; n = n->next) {
				if (!n->next) {
					n->next = node;
					break;
				}
			}
		}
	}
}


void
w_dict_set(w_dict_t *d, const char *key, void *val)
{
	w_dict_setn (d, key, strlen (key), val);
}


void
w_dict_setn (w_dict_t *d, const char *key, size_t len, void *val)
{
	unsigned hval;
	w_dict_node_t *node;
	w_assert (d != NULL);
	w_assert (key != NULL);

	hval = W_DICT_HASHN (key, d->alloc, len);
	node = d->nodes[hval];

	while (node) {
		if (W_DICT_KEY_EQN (node->key, key, len)) {
		    if (d->refs) {
		        w_obj_unref (node->val);
		        node->val = w_obj_ref (val);
            }
            else {
                node->val = val;
            }
			return;
		}
		node = node->next;
	}

	node = w_dict_node_newn(key, len, d->refs ? w_obj_ref (val) : val);

	if (d->nodes[hval]) node->next = d->nodes[hval];
	d->nodes[hval] = node;

	node->nextNode = d->first;
	if (d->first) d->first->prevNode = node;
	d->first = node;

	d->size++;
	if (d->size > (d->alloc * W_DICT_SIZE_TO_ALLOC_RATIO))
		w_dict_rehash (d);
}


void
w_dict_del (w_dict_t *d, const char *key)
{
    w_dict_deln (d, key, strlen (key));
}


void
w_dict_deln (w_dict_t *d, const char *key, size_t keylen)
{
	unsigned hval;
	w_dict_node_t *node;
	w_dict_node_t *lastNode = NULL;
	w_assert (d != NULL);
	w_assert (key != NULL);

	hval = W_DICT_HASHN (key, d->alloc, keylen);

	for (node = d->nodes[hval]; node; node = node->next) {
		if (W_DICT_KEY_EQN (node->key, key, keylen)) {
			w_dict_node_t *prevNode = node->prevNode;
			w_dict_node_t *nextNode = node->nextNode;

			if (prevNode) prevNode->nextNode = nextNode;
			else d->first = nextNode;
			if (nextNode) nextNode->prevNode = prevNode;

			if (lastNode) lastNode->next = node->next;
			else d->nodes[hval] = node->next;

			if (d->refs)
				w_obj_unref (node->val);

			w_dict_node_free (node);
			d->size--;
			return;
		}
	}
}


w_iterator_t
w_dict_first (const w_dict_t *d)
{
	w_assert (d != NULL);
	return (w_iterator_t) d->first;
}


w_iterator_t
w_dict_next (const w_dict_t *d, w_iterator_t i)
{
	w_dict_node_t *node = (w_dict_node_t*) i;
	w_assert (d != NULL);
	w_assert (i != NULL);
	w_unused (d); /* Avoid compiler warnings. */
	return (w_iterator_t) node->nextNode;
}


const char*
w_dict_iterator_get_key (w_iterator_t i)
{
	w_dict_node_t *node = (w_dict_node_t*) i;
	w_assert (i != NULL);
	return node->key;
}


void
w_dict_traverse (w_dict_t *d, w_traverse_fun_t f, void *ctx)
{
	w_iterator_t i;

	w_assert (d != NULL);
	w_assert (f != NULL);

	w_dict_foreach (d, i) {
		*i = (*f) (*i, ctx);
    }
}


void
w_dict_traverse_keys (w_dict_t *d, w_traverse_fun_t f, void *ctx)
{
	w_iterator_t i;

	w_assert (d != NULL);
	w_assert (f != NULL);

	for (i = w_dict_first (d); i; i = w_dict_next (d, i))
		(*f) ((void*) w_dict_iterator_get_key (i), ctx);
}


void
w_dict_traverse_values (w_dict_t *d, w_traverse_fun_t f, void *ctx)
{
	w_iterator_t i;

	w_assert (d != NULL);
	w_assert (f != NULL);

	for (i = w_dict_first (d); i; i = w_dict_next (d, i))
		*i = (*f) (*i, ctx);
}


void
w_dict_update (w_dict_t *dst, const w_dict_t *src)
{
    w_iterator_t i;

    w_assert (dst != NULL);
    w_assert (src != NULL);

    for (i = w_dict_first (src); i; i = w_dict_next (src, i))
        w_dict_set (dst, w_dict_iterator_get_key (i), *i);
}

