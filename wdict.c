/*
 * wdict.c
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wdict.h"
#include "wmem.h"
#include "wstr.h"
#include <string.h>

#ifndef W_DICT_DEFAULT_SIZE
#define W_DICT_DEFAULT_SIZE 128
#endif /* !W_DICT_DEFAULT_SIZE */

#ifndef W_DICT_RESIZE_FACTOR
#define W_DICT_RESIZE_FACTOR 10
#endif /* !W_DICT_RESIZE_FACTOR */

#ifndef W_DICT_COUNT_TO_SIZE_RATIO
#define W_DICT_COUNT_TO_SIZE_RATIO 1.2
#endif /* !W_DICT_COUNT_TO_SIZE_RATIO */

#ifndef W_DICT_HASH
#define W_DICT_HASH(_k, _s) (w_hashstr(_k) % ((_s) - 1))
#endif /* !W_DICT_HASH */

#ifndef W_DICT_HASHN
#define W_DICT_HASHN(_k, _s, _n) (w_hashstrn(_k, _n) % ((_s) - 1))
#endif /* !W_DICT_HASHN */

#ifndef W_DICT_KEY_EQ
#define W_DICT_KEY_EQ(_a, _b) (!strcmp((_a), (_b)))
#endif /* !W_DICT_KEY_EQ */

#ifndef W_DICT_KEY_EQN
#define W_DICT_KEY_EQN(_a, _b, _blen) (!strncmp((_a), (_b), (_blen)))
#endif /* !W_DICT_KEY_EQN */

typedef struct w_dict_node w_dict_node_t;


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

struct w_dict
{
	w_dict_node_t **nodes;
	w_dict_node_t *first;
	unsigned      count;
	unsigned      size;
};


static inline w_dict_node_t*
_w_dict_node_new(const char *key, void *val)
{
	w_dict_node_t *node;
	w_assert (key != NULL);

	node = w_new(w_dict_node_t);
	node->key  = w_strdup(key);
	node->val  = val;
	return node;
}


static inline w_dict_node_t*
_w_dict_node_newn(const char *key, size_t len, void *val)
{
	w_dict_node_t *node;
	w_assert (key != NULL);
	w_assert (len > 0);

	node = w_new(w_dict_node_t);
	node->key = w_strndup(key, len);
	node->val = val;
	return node;
}


static inline void
_w_dict_node_free(w_dict_node_t *node)
{
	w_assert (node != NULL);
	w_assert (node->key != NULL);
	w_free(node->key);
	w_free(node);
}


static inline void
_w_dict_free_nodes(w_dict_t *d)
{
	w_dict_node_t *node = d->first;
	w_dict_node_t *next;

	while (node) {
		next = node->nextNode;
		_w_dict_node_free(node);
		node = next;
	}
}


w_dict_t*
w_dict_new(void)
{
	w_dict_t *d = w_new(w_dict_t);
	d->size  = W_DICT_DEFAULT_SIZE;
	d->nodes = w_alloc(w_dict_node_t*, d->size);
	return d;
}


void
w_dict_free(w_dict_t *d)
{
	w_assert (d != NULL);
	_w_dict_free_nodes(d);
	w_free(d->nodes);
	w_free(d);
}


unsigned
w_dict_count(const w_dict_t *d)
{
	w_assert (d != NULL);
	return d->count;
}


void
w_dict_clear(w_dict_t *d)
{
	w_assert (d != NULL);
	_w_dict_free_nodes(d);
	memset(d->nodes, 0x00, d->size * sizeof(w_dict_node_t*));
}


void*
w_dict_get(const w_dict_t *d, const char *key)
{
	return w_dict_getn(d, key, strlen(key));
}


void*
w_dict_getn(const w_dict_t *d, const char *key, size_t len)
{
	w_dict_node_t *node;
	unsigned hval;

	w_assert (d != NULL);
	w_assert (key != NULL);

	hval = W_DICT_HASHN(key, d->size, len);
	node = d->nodes[hval];

	if (node) {
		if (W_DICT_KEY_EQN(node->key, key, len)) {
			return node->val;
		}
		else {
			w_dict_node_t *lastNode = node;
			node = node->next;
			while (node) {
				if (W_DICT_KEY_EQN(node->key, key, len)) {
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
_w_dict_rehash(w_dict_t *d)
{
	w_dict_node_t *node = d->first;

	while (node) {
		node->next = NULL;
		node = node->nextNode;
	}

	d->size *= W_DICT_RESIZE_FACTOR;
	d->nodes = w_resize(d->nodes, w_dict_node_t*, ++d->size);
	memset(d->nodes, 0x00, d->size * sizeof(w_dict_node_t*));

	for (node = d->first; node; node = node->nextNode) {
		unsigned hval  = W_DICT_HASH(node->key, d->size);
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
	w_dict_setn(d, key, strlen(key), val);
}


void
w_dict_setn(w_dict_t *d, const char *key, size_t len, void *val)
{
	unsigned hval;
	w_dict_node_t *node;
	w_assert (d != NULL);
	w_assert (key != NULL);

	hval = W_DICT_HASHN(key, d->size, len);
	node = d->nodes[hval];

	while (node) {
		if (W_DICT_KEY_EQN(node->key, key, len)) {
			node->val = val;
			return;
		}
		node = node->next;
	}

	node = _w_dict_node_newn(key, len, val);

	if (d->nodes[hval]) node->next = d->nodes[hval];
	d->nodes[hval] = node;

	node->nextNode = d->first;
	if (d->first) d->first->prevNode = node;
	d->first = node;

	d->count++;
	if (d->count > (d->size * W_DICT_COUNT_TO_SIZE_RATIO))
		_w_dict_rehash(d);
}


void
w_dict_del(w_dict_t *d, const char *key)
{
	unsigned hval;
	w_dict_node_t *node;
	w_dict_node_t *lastNode = NULL;
	w_assert (d != NULL);
	w_assert (key != NULL);

	hval = W_DICT_HASH(key, d->size);

	for (node = d->nodes[hval]; node; node = node->next) {
		if (W_DICT_KEY_EQ(node->key, key)) {
			w_dict_node_t *prevNode = node->prevNode;
			w_dict_node_t *nextNode = node->nextNode;

			if (prevNode) prevNode->nextNode = nextNode;
			else d->first = nextNode;
			if (nextNode) nextNode->prevNode = prevNode;

			if (lastNode) lastNode->next = node->next;
			else d->nodes[hval] = node->next;

			_w_dict_node_free(node);
			d->count--;
			return;
		}
	}
}


w_iterator_t
w_dict_first(const w_dict_t *d)
{
	w_assert (d != NULL);
	return (w_iterator_t) d->first;
}


w_iterator_t
w_dict_next(const w_dict_t *d, w_iterator_t i)
{
	w_dict_node_t *node = (w_dict_node_t*) i;
	w_assert (d != NULL);
	w_assert (i != NULL);
	w_unused(d); /* Avoid compiler warnings. */
	return (w_iterator_t) node->nextNode;
}


const char*
w_dict_iterator_get_key(w_iterator_t i)
{
	w_dict_node_t *node = (w_dict_node_t*) i;
	w_assert (i != NULL);
	return node->key;
}


void
w_dict_traverse(w_dict_t *d, w_traverse_fun_t f, void *ctx)
{
	w_iterator_t i;

	w_assert (d != NULL);
	w_assert (f != NULL);

	for (i = w_dict_first(d); i != NULL; i = w_dict_next(d, i))
		*i = (*f)(*i, ctx);
}


void
w_dict_traverse_keys(w_dict_t *d, w_traverse_fun_t f, void *ctx)
{
	w_dict_item_t *i;

	w_assert(d != NULL);
	w_assert(f != NULL);

	for (i = w_dict_item_first(d); i != NULL; i = w_dict_item_next(d, i))
		(*f)((void*) i->key, ctx);
}


void
w_dict_traverse_values(w_dict_t *d, w_traverse_fun_t f, void *ctx)
{
	w_dict_item_t *i;

	w_assert(d != NULL);
	w_assert(f != NULL);

	for (i = w_dict_item_first(d); i != NULL; i = w_dict_item_next(d, i))
		i->val = (*f)(i->val, ctx);
}

