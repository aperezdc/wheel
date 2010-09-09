/*
 * wcfg.c
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wcfg.h"
#include "wmem.h"
#include "wstr.h"
#include <string.h>
#include <stdarg.h>


typedef struct w_cfg_node w_cfg_node_t;

struct w_cfg_node
{
	w_cfg_type_t kind;
	union {
		w_cfg_t *node;
		char    *string;
		double   number;
	};
};


w_cfg_t*
w_cfg_new(void)
{
	return w_dict_new();
}


static void*
_w_cfg_free_item(void *i, void *ctx)
{
	w_cfg_node_t *node = (w_cfg_node_t*) i;
	w_assert(node != NULL);
	w_unused(ctx);

	switch (node->kind & W_CFG_TYPE_MASK) {
		case W_CFG_NODE: w_cfg_free(node->node); break;
		case W_CFG_STRING: w_free(node->string); break;
		case W_CFG_NUMBER:
			/* Fall through. */
		default: /* Do nothing. */
			break;
	}
	w_free(node);
	return NULL;
}


void
w_cfg_free(w_cfg_t *cf)
{
	w_assert(cf != NULL);
	w_dict_traverse(cf, _w_cfg_free_item, NULL);
	w_dict_free(cf);
}


static w_cfg_node_t*
_w_cfg_getnode(const w_cfg_t *cf, const char *key)
{
	w_cfg_node_t *node;
	char *sep;
	w_assert(key != NULL);
	w_assert(cf != NULL);

	if (!w_dict_count(cf)) return NULL;

	if ((sep = strchr(key, '.')) != NULL) {
		/* Dotted key. */
		node = w_dict_getn(cf, key, (size_t)(sep - key));
		if ((node != NULL) && (node->kind == W_CFG_NODE))
			return _w_cfg_getnode(node->node, ++sep);
	}
	else {
		/* Plain key. */
		return w_dict_get(cf, key);
	}

	/* Not found. */
	return NULL;
}


static w_cfg_node_t*
_w_cfg_ensurenode(w_cfg_t *cf, const char *key)
{
	char *sep;
	size_t len = 0;
	w_cfg_node_t *node = NULL;

	w_assert(key != NULL);
	w_assert(cf != NULL);

	if ((sep = strchr(key, '.')) != NULL)
		len = (size_t) (sep - key);

	if (w_dict_count(cf)) {
		if (sep != NULL) {
			node = w_dict_getn(cf, key, len);
			if ((node != NULL) && (node->kind == W_CFG_NODE))
				return _w_cfg_ensurenode(node->node, ++sep);
		}
		else {
			node = w_dict_get(cf, key);
		}
	}

	if (node == NULL) {
		/* Node not found -> create it. */
		node = w_new(w_cfg_node_t);
		if (node != NULL)
			w_dict_setn(cf, key, len, node);
		else
			w_dict_set(cf, key, node);
	}

	if (sep != NULL) {
		node->kind = W_CFG_NODE;
		node->node = w_cfg_new();
		return _w_cfg_ensurenode(node->node, ++sep);
	}

	return node;
}


static wbool
_w_cfg_setv(w_cfg_t *cf, va_list args)
{
	wbool ret = W_YES;
	w_cfg_type_t kind;
	w_cfg_node_t *node;
	w_assert(cf != NULL);

	while ((kind = va_arg(args, int)) != W_CFG_END) {
		const char *key = va_arg(args, const char*);
		w_assert(key != NULL);

		if ((node = _w_cfg_ensurenode(cf, key)) == NULL)
			ret = W_NO;
		node->kind = kind & W_CFG_TYPE_MASK;

		switch (node->kind) {
			case W_CFG_NUMBER:
				node->number = va_arg(args, double);
				break;
			case W_CFG_STRING:
				node->string = w_strdup(va_arg(args, const char*));
				break;
			case W_CFG_NODE:
				node->node = va_arg(args, w_cfg_t*);
				break;
			default:
				/* XXX For now just trust the programmer. */
				ret = W_NO;
				break;
		}
	}
	return ret;
}


static wbool
_w_cfg_getv(const w_cfg_t *cf, va_list args)
{
	wbool ret = W_YES;
	w_cfg_type_t kind;
	w_cfg_node_t *node;
	double *pnumber;
	w_cfg_t **pnode;
	const char **pstring;
	w_assert(cf != NULL);

	while ((kind = va_arg(args, int)) != W_CFG_END) {
		const char *key = va_arg(args, const char*);
		w_assert(key != NULL);

		if ((node = _w_cfg_getnode(cf, key)) == NULL)
			ret = W_NO;

		switch (kind & W_CFG_TYPE_MASK) {
			case W_CFG_NUMBER:
				pnumber = va_arg(args, double*);
				if ((node != NULL) && (node->kind == W_CFG_NUMBER))
					*pnumber = node->number;
				else ret = W_NO;
				break;
			case W_CFG_STRING:
				pstring = va_arg(args, const char**);
				if ((node != NULL) && (node->kind == W_CFG_STRING))
					*pstring = node->string;
				else ret = W_NO;
				break;
			case W_CFG_NODE:
				pnode = va_arg(args, w_cfg_t**);
				if ((node != NULL) && (node->kind == W_CFG_NODE))
					*pnode = node->node;
				else ret = W_NO;
				break;
			default:
				/* XXX For now just trust the programmer. */
				ret = W_NO;
				break;
		}
	}
	return ret;
}


wbool
w_cfg_set(w_cfg_t *cf, ...)
{
	wbool ret;
	va_list args;

	w_assert(cf != NULL);
	va_start(args, cf);
	ret = _w_cfg_setv(cf, args);
	va_end(args);
	return ret;
}


wbool
w_cfg_get(const w_cfg_t *cf, ...)
{
	wbool ret;
	va_list args;

	w_assert(cf != NULL);
	va_start(args, cf);
	ret = _w_cfg_getv(cf, args);
	va_end(args);
	return ret;
}


wbool
w_cfg_has(const w_cfg_t *cf, const char *key)
{
	w_assert(cf != NULL);
	w_assert(key != NULL);

	return (_w_cfg_getnode(cf, key) != NULL);
}


w_cfg_type_t
w_cfg_type(const w_cfg_t *cf, const char *key)
{
	w_cfg_node_t *node;
	w_assert(cf != NULL);
	w_assert(key != NULL);

	if ((node = _w_cfg_getnode(cf, key)) != NULL)
		return node->kind;
	else
		return W_CFG_NONE;
}


static w_cfg_node_t*
_w_cfg_getnodelocation(w_cfg_t *cf, const char *key, w_iterator_t *j, w_cfg_t **where)
{
	w_iterator_t i;
	const char *sep;
	size_t len;

	w_assert(j != NULL);
	w_assert(cf != NULL);
	w_assert(key != NULL);

	if (!w_dict_count(cf)) return NULL;

	sep = strchr(key, '.');
	len = (sep != NULL)
		? (size_t) (sep - key)
		: (size_t) strlen(key);

	for (i = w_dict_first(cf); i != NULL; i = w_dict_next(cf, i)) {
		w_cfg_node_t *node = (w_cfg_node_t*) *i;
		if (!strncmp(key, w_dict_iterator_get_key(i), len)) {
			if (sep != NULL) {
				if (node->kind != W_CFG_NODE) return NULL;
				else return _w_cfg_getnodelocation(node->node, ++sep, j, where);
			}
			else {
				*j     = i;
				*where = cf;
				return node;
			}
		}
	}
	return NULL;
}


wbool
w_cfg_del(w_cfg_t *cf, const char *key)
{
	w_cfg_node_t *node;
	w_cfg_t *where = NULL;
	w_iterator_t it = NULL;

	w_assert(cf != NULL);
	w_assert(key != NULL);

	if ((node = _w_cfg_getnodelocation(cf, key, &it, &where)) == NULL)
		return W_NO;

	w_assert(where != NULL);
	w_assert(it != NULL);

	if (node->kind == W_CFG_NODE)
		w_cfg_free(node->node);
	else if (node->kind == W_CFG_STRING)
		w_free(node->string);

	w_dict_del(where, w_dict_iterator_get_key(it));
	w_free(node);

	return W_YES;
}



#define W_CFG_DUMP_INDENT(_l, _o)  \
    do {                           \
        unsigned __tmp = (_l) * 4; \
        while (__tmp--)            \
            fputc (' ', (_o));     \
    } while (0)



static wbool
_w_cfg_dump_cfg (const w_cfg_t *cf, FILE *out, unsigned indent)
{
	w_iterator_t i;
	w_cfg_node_t *node;

	w_assert (cf != NULL);
	w_assert (out != NULL);

    for (i = w_dict_first (cf); i != NULL; i = w_dict_next (cf, i)) {
        W_CFG_DUMP_INDENT (indent, out);

        node = (w_cfg_node_t*) *i;

        fprintf (out, "%s = ", w_dict_iterator_get_key (i));

        switch (node->kind & W_CFG_TYPE_MASK) {
            case W_CFG_NODE:
                fprintf (out, "{\n");
                _w_cfg_dump_cfg (node->node, out, indent + 1);
                W_CFG_DUMP_INDENT (indent, out);
                fprintf (out, "};\n");
                break;
            case W_CFG_STRING:
                break;
                fprintf (out, "\"%s\";\n", node->string);
            case W_CFG_NUMBER:
                fprintf (out, "%lf;\n", node->number);
                break;
            default:
                break;
        }
    }

    return W_YES;
}


/*
 * TODO Better error checking of IO functions.
 */
wbool
w_cfg_dump (const w_cfg_t *cf, FILE *output)
{
    w_assert (cf != NULL);
    w_assert (output != NULL);

    return (fprintf (output, "{\n") == 1 &&
            _w_cfg_dump_cfg (cf, output, 1) &&
            fprintf (output, "};\n") == 1);
}

