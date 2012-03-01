/*
 * wcfg.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <fcntl.h>


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


static void*
w_cfg_free_item (void *i, void *ctx)
{
	w_cfg_node_t *node = (w_cfg_node_t*) i;
	w_assert (node != NULL);
	w_unused (ctx);

	switch (node->kind & W_CFG_TYPE_MASK) {
		case W_CFG_NODE: w_obj_unref (node->node); break;
		case W_CFG_STRING: w_free (node->string); break;
		case W_CFG_NUMBER:
			/* Fall through. */
		default: /* Do nothing. */
			break;
	}
	w_free (node);
	return NULL;
}


static void
w_cfg_free (void *cf)
{
	w_assert (cf != NULL);
	w_dict_traverse ((w_dict_t*) cf, w_cfg_free_item, NULL);
	w_dict_free (cf);
}


w_cfg_t*
w_cfg_new (void)
{
	return w_obj_dtor (w_dict_new (W_NO), w_dict_free);
}


static w_cfg_node_t*
w_cfg_getnode (const w_cfg_t *cf, const char *key)
{
	w_cfg_node_t *node;
	char *sep;
	w_assert (key != NULL);
	w_assert (cf != NULL);

	if (!w_dict_count (cf)) return NULL;

	if ((sep = strchr (key, '.')) != NULL) {
		/* Dotted key. */
		node = w_dict_getn (cf, key, (size_t) (sep - key));
		if ((node != NULL) && (node->kind == W_CFG_NODE))
			return w_cfg_getnode (node->node, ++sep);
	}
	else {
		/* Plain key. */
		return w_dict_get (cf, key);
	}

	/* Not found. */
	return NULL;
}


static w_cfg_node_t*
w_cfg_ensurenode (w_cfg_t *cf, const char *key)
{
	char *sep;
	size_t len = 0;
	w_cfg_node_t *node = NULL;

	w_assert (key != NULL);
	w_assert (cf != NULL);

	if ((sep = strchr (key, '.')) != NULL)
		len = (size_t) (sep - key);
    else
        len = strlen (key);

	if (w_dict_count (cf)) {
		if (sep != NULL) {
			node = w_dict_getn (cf, key, len);
			if ((node != NULL) && (node->kind == W_CFG_NODE))
				return w_cfg_ensurenode (node->node, ++sep);
		}
		else {
			node = w_dict_get (cf, key);
		}
	}

	if (node == NULL) {
		/* Node not found -> create it. */
		node = w_new (w_cfg_node_t);
		if (node != NULL)
			w_dict_setn (cf, key, len, node);
		else
			w_dict_set (cf, key, node);
	}

	if (sep != NULL) {
		node->kind = W_CFG_NODE;
		node->node = w_cfg_new ();
		return w_cfg_ensurenode (node->node, ++sep);
	}

	return node;
}


static wbool
w_cfg_setv (w_cfg_t *cf, va_list args)
{
	wbool ret = W_YES;
	w_cfg_type_t kind;
	w_cfg_node_t *node;
	w_assert (cf != NULL);

	while ((kind = va_arg (args, int)) != W_CFG_END) {
		const char *key = va_arg (args, const char*);
		w_assert (key != NULL);

		if ((node = w_cfg_ensurenode (cf, key)) == NULL)
			ret = W_NO;
		node->kind = kind & W_CFG_TYPE_MASK;

		switch (node->kind) {
			case W_CFG_NUMBER:
				node->number = va_arg (args, double);
				break;
			case W_CFG_STRING:
				node->string = w_str_dup (va_arg (args, const char*));
				break;
			case W_CFG_NODE:
				if (node->node) {
				    w_obj_unref (node->node);
				}
				node->node = w_obj_ref (va_arg (args, w_cfg_t*));
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
w_cfg_getv (const w_cfg_t *cf, va_list args)
{
	wbool ret = W_YES;
	w_cfg_type_t kind;
	w_cfg_node_t *node;
	double *pnumber;
	w_cfg_t **pnode;
	const char **pstring;
	w_assert (cf != NULL);

	while ((kind = va_arg (args, int)) != W_CFG_END) {
		const char *key = va_arg (args, const char*);
		w_assert (key != NULL);

		if ((node = w_cfg_getnode (cf, key)) == NULL)
			ret = W_NO;

		switch (kind & W_CFG_TYPE_MASK) {
			case W_CFG_NUMBER:
				pnumber = va_arg (args, double*);
				if ((node != NULL) && (node->kind == W_CFG_NUMBER))
					*pnumber = node->number;
				else ret = W_NO;
				break;
			case W_CFG_STRING:
				pstring = va_arg (args, const char**);
				if ((node != NULL) && (node->kind == W_CFG_STRING))
					*pstring = node->string;
				else ret = W_NO;
				break;
			case W_CFG_NODE:
				pnode = va_arg (args, w_cfg_t**);
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
w_cfg_set (w_cfg_t *cf, ...)
{
	wbool ret;
	va_list args;

	w_assert (cf != NULL);
	va_start (args, cf);
	ret = w_cfg_setv (cf, args);
	va_end (args);
	return ret;
}


wbool
w_cfg_get (const w_cfg_t *cf, ...)
{
	wbool ret;
	va_list args;

	w_assert (cf != NULL);
	va_start (args, cf);
	ret = w_cfg_getv (cf, args);
	va_end (args);
	return ret;
}


wbool
w_cfg_has (const w_cfg_t *cf, const char *key)
{
	w_assert (cf != NULL);
	w_assert (key != NULL);

	return (w_cfg_getnode (cf, key) != NULL);
}


w_cfg_type_t
w_cfg_type (const w_cfg_t *cf, const char *key)
{
	w_cfg_node_t *node;
	w_assert (cf != NULL);
	w_assert (key != NULL);

	if ((node = w_cfg_getnode (cf, key)) != NULL)
		return node->kind;
	else
		return W_CFG_NONE;
}


static w_cfg_node_t*
w_cfg_getnodelocation (w_cfg_t *cf, const char *key, w_iterator_t *j, w_cfg_t **where)
{
	w_iterator_t i;
	const char *sep;
	size_t len;

	w_assert (j != NULL);
	w_assert (cf != NULL);
	w_assert (key != NULL);

	if (!w_dict_count (cf)) return NULL;

	sep = strchr (key, '.');
	len = (sep != NULL)
		? (size_t) (sep - key)
		: (size_t) strlen (key);

	for (i = w_dict_first (cf); i != NULL; i = w_dict_next (cf, i)) {
		w_cfg_node_t *node = (w_cfg_node_t*) *i;
		if (!strncmp (key, w_dict_iterator_get_key (i), len)) {
			if (sep != NULL) {
				if (node->kind != W_CFG_NODE) return NULL;
				else return w_cfg_getnodelocation (node->node, ++sep, j, where);
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
w_cfg_del (w_cfg_t *cf, const char *key)
{
	w_cfg_node_t *node;
	w_cfg_t *where = NULL;
	w_iterator_t it = NULL;

	w_assert (cf != NULL);
	w_assert (key != NULL);

	if ((node = w_cfg_getnodelocation (cf, key, &it, &where)) == NULL)
		return W_NO;

	w_assert (where != NULL);
	w_assert (it != NULL);

	if (node->kind == W_CFG_NODE)
		w_cfg_free (node->node);
	else if (node->kind == W_CFG_STRING)
		w_free (node->string);

	w_dict_del (where, w_dict_iterator_get_key (it));
	w_free (node);

	return W_YES;
}



#define W_CFG_DUMP_INDENT(_l, _o)     \
    do {                              \
        unsigned __tmp = (_l) * 4;    \
        while (__tmp--)               \
            w_io_putchar ((_o), ' '); \
    } while (0)


static wbool
w_cfg_dump_string (w_io_t *out, const char *str)
{
    w_assert (out);
    w_assert (str);

    w_io_putchar (out, '"');
    for (; *str; str++) {
#define ESCAPE(_c, _e) \
        case _c : w_io_putchar (out, '\\'); \
                  w_io_putchar (out, (_e)); \
                  break
        switch (*str) {
            ESCAPE ('\n', 'n');
            ESCAPE ('\r', 'r');
            ESCAPE ('\b', 'b');
            ESCAPE (0x1b, 'e');
            ESCAPE ('\a', 'a');
            ESCAPE ('\t', 't');
            ESCAPE ('\v', 'v');
            default:
                w_io_putchar (out, *str);
        }
#undef ESCAPE
    }
    w_io_putchar (out, '"');
    return W_YES;
}


static wbool
w_cfg_dump_cfg (const w_cfg_t *cf, w_io_t *out, unsigned indent)
{
	w_iterator_t i;
	w_cfg_node_t *node;

	w_assert (cf != NULL);
	w_assert (out != NULL);

    w_dict_foreach (cf, i) {
        W_CFG_DUMP_INDENT (indent, out);

        node = (w_cfg_node_t*) *i;

        w_io_format (out, "$s ", w_dict_iterator_get_key (i));

        switch (node->kind & W_CFG_TYPE_MASK) {
            case W_CFG_NODE:
                w_io_putchar (out, '{');
                w_io_putchar (out, '\n');
                w_cfg_dump_cfg (node->node, out, indent + 1);
                W_CFG_DUMP_INDENT (indent, out);
                w_io_putchar (out, '}');
                break;
            case W_CFG_STRING:
                w_cfg_dump_string (out, node->string);
                break;
            case W_CFG_NUMBER:
                w_io_format_double (out, node->number);
                break;
            default:
                break;
        }
        w_io_putchar (out, '\n');
    }

    return W_YES;
}


/*
 * TODO Better error checking of IO functions.
 */
wbool
w_cfg_dump (const w_cfg_t *cf, w_io_t *output)
{
    w_assert (cf != NULL);
    w_assert (output != NULL);

    return w_cfg_dump_cfg (cf, output, 0);
}


wbool
w_cfg_dump_file (const w_cfg_t *cf, const char *path)
{
    w_io_t *io;
    wbool ret;

    w_assert (cf != NULL);
    w_assert (path != NULL);

    if ((io = w_io_unix_open (path, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == NULL)
        return W_NO;

    ret = w_cfg_dump (cf, io);
    w_obj_unref (io);

    return ret;
}


static void
w_cfg_parse_items (w_parse_t *p, w_cfg_t *r)
{
    char    *key;
    char    *svalue;
    w_cfg_t *cvalue;
    double   dvalue;

    w_assert (p);
    w_assert (r);

    /*
     * Empty subnode, e.g. like the following:
     *
     *    this_is_empty { }
     *
     * This is valid syntax, so take it into account.
     */
    while (p->look != W_IO_EOF && p->look != '}') {
        if (!(key = w_parse_ident (p))) {
            w_parse_error (p, "Identifier expected");
        }
        switch (p->look) {
            case '"':
                if ((svalue = w_parse_string (p)) == NULL) {
                    w_parse_ferror (p, "Malformed string for key '$s'", key);
                    w_free (key);
                    w_parse_rerror (p);
                }
                w_cfg_set_string (r, key, svalue);
                w_free (svalue);
                break;
            case '{':
                w_parse_match (p, '{');
                cvalue = w_cfg_new ();
                w_cfg_set_node (r, key, cvalue);
                w_obj_unref (cvalue);
                w_free (key);
                w_cfg_parse_items (p, cvalue);
                w_parse_match (p, '}');
                break;
            default:
                if (!w_parse_double (p, &dvalue)) {
                    w_free (key);
                    w_parse_error (p, "Number expected");
                }
                w_cfg_set_number (r, key, dvalue);
        }
        w_free (key);
    }
}


w_cfg_t*
w_cfg_load (w_io_t *input, char **pmsg)
{
    char *errmsg;
    w_cfg_t *result = w_cfg_new ();

    w_parse_t parser;

    w_assert (input != NULL);

    w_parse_run (&parser, input, '#',
                 (w_parse_fun_t) w_cfg_parse_items,
                 result, &errmsg);

    if (errmsg != NULL) {
        w_obj_unref (result);
        w_assert (errmsg);
        if (pmsg == NULL)
            w_free (errmsg);
        else
            *pmsg = errmsg;
        return NULL;
    }
    else {
        return result;
    }
}



w_cfg_t*
w_cfg_load_file (const char *path, char **msg)
{
    w_io_t *io;
    w_cfg_t *ret;

    w_assert (path != NULL);

    if ((io = w_io_unix_open (path, O_RDONLY, 0)) == NULL) {
        if (msg) {
            *msg = w_strfmt ("Could not open file '%s' for reading", path);
        }
        return NULL;
    }

    ret = w_cfg_load (io, msg);
    w_obj_unref (io);

    return ret;
}

