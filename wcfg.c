/*
 * wcfg.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>


static w_variant_t*
w_cfg_getnode (const w_cfg_t *cf, const char *key)
{
    w_variant_t *node;
    char *sep;
    w_assert (key);
    w_assert (cf);

    if (w_dict_empty (cf)) return NULL;

    if ((sep = strchr (key, '.')) != NULL) {
        /* Dotted key. */
        node = w_dict_getn (cf, key, (size_t) (sep - key));
        if ((node != NULL) && w_variant_is_dict (node))
            return w_cfg_getnode (w_variant_dict (node), ++sep);
    }
    else {
        /* Plain key. */
        return w_dict_get (cf, key);
    }

    /* Not found. */
    return NULL;
}


static w_variant_t*
w_cfg_ensurenode (w_cfg_t *cf, const char *key)
{
    char *sep;
    size_t len = 0;
    w_variant_t *node = NULL;

    w_assert (key);
    w_assert (cf);

    if ((sep = strchr (key, '.')) != NULL)
        len = (size_t) (sep - key);
    else
        len = strlen (key);

    if (w_dict_size (cf)) {
        if (sep) {
            node = w_dict_getn (cf, key, len);
            if ((node != NULL) && w_variant_is_dict (node))
                return w_cfg_ensurenode (w_variant_dict (node), ++sep);
        }
        else {
            node = w_dict_get (cf, key);
        }
    }

    if (!node) {
        /* Node not found -> create it. */
        node = w_variant_new (W_VARIANT_INVALID);
        w_dict_setn (cf, key, len, node);
    }

    if (sep) {
        w_dict_t *d = w_dict_new (true);
        w_variant_set_dict (node, d);
        w_obj_unref (d);
        return w_cfg_ensurenode (d, ++sep);
    }

    return node;
}


static bool
w_cfg_setv (w_cfg_t *cf, va_list args)
{
    bool ret = true;
    w_cfg_type_t kind;
    w_variant_t *node;
    w_assert (cf);

    while ((kind = va_arg (args, w_cfg_type_t)) != W_CFG_END) {
        const char *key = va_arg (args, const char*);
        w_assert (key);

        if (!(node = w_cfg_ensurenode (cf, key)))
            ret = false;

        switch (kind) {
            case W_CFG_NUMBER:
                w_variant_set_float (node, va_arg (args, double));
                break;
            case W_CFG_STRING:
                w_variant_set_string (node, va_arg (args, const char*));
                break;
            case W_CFG_NODE:
                w_variant_set_dict (node, va_arg (args, w_cfg_t*));
                break;
            default:
                /* XXX For now just trust the programmer. */
                ret = false;
                break;
        }
    }
    return ret;
}


static bool
w_cfg_getv (const w_cfg_t *cf, va_list args)
{
    bool ret = true;
    w_cfg_type_t kind;
    w_variant_t *node;
    double *pnumber;
    w_cfg_t **pnode;
    const char **pstring;
    w_assert (cf != NULL);

    while ((kind = va_arg (args, w_cfg_type_t)) != W_CFG_END) {
        const char *key = va_arg (args, const char*);
        w_assert (key);

        if (!(node = w_cfg_getnode (cf, key)))
            ret = false;

        switch (kind) {
            case W_CFG_NUMBER:
                pnumber = va_arg (args, double*);
                if (node && w_variant_is_float (node))
                    *pnumber = w_variant_float (node);
                else ret = false;
                break;
            case W_CFG_STRING:
                pstring = va_arg (args, const char**);
                if (node && w_variant_is_string (node))
                    *pstring = w_variant_string (node);
                else ret = false;
                break;
            case W_CFG_NODE:
                pnode = va_arg (args, w_cfg_t**);
                if (node && w_variant_is_dict (node))
                    *pnode = w_variant_dict (node);
                else ret = false;
                break;
            default:
                /* XXX For now just trust the programmer. */
                ret = false;
                break;
        }
    }
    return ret;
}


bool
w_cfg_set (w_cfg_t *cf, ...)
{
    bool ret;
    va_list args;
    w_assert (cf);
    va_start (args, cf);
    ret = w_cfg_setv (cf, args);
    va_end (args);
    return ret;
}


bool
w_cfg_get (const w_cfg_t *cf, ...)
{
    bool ret;
    va_list args;
    w_assert (cf);
    va_start (args, cf);
    ret = w_cfg_getv (cf, args);
    va_end (args);
    return ret;
}


bool
w_cfg_has (const w_cfg_t *cf, const char *key)
{
    w_assert (cf);
    w_assert (key);
    return (w_cfg_getnode (cf, key) != NULL);
}


w_cfg_type_t
w_cfg_type (const w_cfg_t *cf, const char *key)
{
    w_variant_t *node;
    w_assert (cf);
    w_assert (key);

    if ((node = w_cfg_getnode (cf, key)))
        return (w_cfg_type_t) w_variant_type (node);
    else
        return W_CFG_NONE;
}


static w_variant_t*
w_cfg_getnodelocation (w_cfg_t *cf, const char *key, w_iterator_t *j, w_cfg_t **where)
{
    w_iterator_t i;
    const char *sep;
    size_t len;

    w_assert (j);
    w_assert (cf);
    w_assert (key);

    if (w_dict_empty (cf))
        return NULL;

    sep = strchr (key, '.');
    len = sep ? (size_t)(sep - key) : (size_t)(strlen (key));

    w_dict_foreach (cf, i) {
        w_variant_t *node = (w_variant_t*) *i;
        if (!strncmp (key, w_dict_iterator_get_key (i), len)) {
            if (sep) {
                if (!w_variant_is_dict (node))
                    return NULL;
                else
                    return w_cfg_getnodelocation (w_variant_dict (node), ++sep, j, where);
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


bool
w_cfg_del (w_cfg_t *cf, const char *key)
{
    w_variant_t *node;
    w_cfg_t *where = NULL;
    w_iterator_t it = NULL;

    w_assert (cf);
    w_assert (key);

    if (!(node = w_cfg_getnodelocation (cf, key, &it, &where)))
        return false;

    w_assert (where);
    w_assert (it);

    w_dict_del (where, w_dict_iterator_get_key (it));

    return true;
}


#define DUMP_INDENT(_i, _l, _o)                        \
    do {                                               \
        unsigned __tmp = (_l) * 4;                     \
        while (__tmp--)                                \
            W_IO_CHAIN (_i, w_io_putchar ((_o), ' ')); \
    } while (0)


static w_io_result_t
dump_buffer (const w_buf_t *buf, w_io_t *out)
{
    w_assert (out);
    w_assert (buf);

    w_io_result_t r = W_IO_RESULT (0);
    W_IO_CHAIN (r, w_io_putchar (out, '"'));

    if (w_buf_size (buf)) {
        for (unsigned i = 0; i < w_buf_size (buf); i++) {
            int c = w_buf_data (buf)[i];
#define ESCAPE(_c, _e) \
            case _c : W_IO_CHAIN (r, w_io_putchar (out, '\\')); \
                      W_IO_CHAIN (r, w_io_putchar (out, (_e))); \
                      break
            switch (c) {
                ESCAPE ('\n', 'n');
                ESCAPE ('\r', 'r');
                ESCAPE ('\b', 'b');
                ESCAPE (0x1b, 'e');
                ESCAPE ('\a', 'a');
                ESCAPE ('\t', 't');
                ESCAPE ('\v', 'v');
                default:
                    W_IO_CHAIN (r, w_io_putchar (out, c));
            }
#undef ESCAPE
        }
    }

    W_IO_CHAIN (r, w_io_putchar (out, '"'));
    return r;
}


static w_io_result_t dump_value (const w_variant_t*, w_io_t*, unsigned);
static w_io_result_t dump_dict (const w_dict_t*, w_io_t*, unsigned);


static w_io_result_t
dump_list (const w_list_t *list, w_io_t* stream, unsigned indent)
{
    w_io_result_t r = W_IO_RESULT (0);
    w_iterator_t i;

    w_list_foreach (list, i) {
        DUMP_INDENT (r, indent, stream);
        W_IO_CHAIN (r, dump_value ((w_variant_t*) *i, stream, indent));
        W_IO_CHAIN (r, w_io_putchar (stream, '\n'));
    }

    return r;
}


static w_io_result_t
dump_value (const w_variant_t *value, w_io_t *stream, unsigned indent)
{
    w_io_result_t r = W_IO_RESULT (0);

    switch (w_variant_type (value)) {
        case W_VARIANT_STRING:
            W_IO_CHAIN (r, dump_buffer (w_variant_buffer (value), stream));
            break;
        case W_VARIANT_BOOL:
            W_IO_CHAIN (r, w_io_format (stream, "$s",
                                        w_variant_bool (value)
                                            ? "true" : "false"));
            break;
        case W_VARIANT_NUMBER:
            W_IO_CHAIN (r, w_io_format_long (stream,
                                             w_variant_number (value)));
            break;
        case W_VARIANT_FLOAT:
            W_IO_CHAIN (r, w_io_format_double (stream,
                                               w_variant_float (value)));
            break;
        case W_VARIANT_LIST:
            W_IO_CHAIN (r, w_io_putchar (stream, '['));
            W_IO_CHAIN (r, w_io_putchar (stream, '\n'));
            W_IO_CHAIN (r, dump_list (w_variant_list (value),
                                      stream, indent + 1));
            W_IO_CHAIN (r, w_io_putchar (stream, ']'));
            W_IO_CHAIN (r, w_io_putchar (stream, '\n'));
            break;
        case W_VARIANT_DICT:
            W_IO_CHAIN (r, w_io_putchar (stream, '{'));
            W_IO_CHAIN (r, w_io_putchar (stream, '\n'));
            W_IO_CHAIN (r, dump_dict (w_variant_dict (value), stream, indent + 1));
            W_IO_CHAIN (r, w_io_putchar (stream, '}'));
            W_IO_CHAIN (r, w_io_putchar (stream, '\n'));
            break;
        default:
            // TODO: Handle errors better than aborting.
            w_die ("%s: Invalid variant type in w_cfg_t container\n", __func__);
    }

    return r;
}


static w_io_result_t
dump_dict (const w_dict_t *dict, w_io_t *stream, unsigned indent)
{
    w_io_result_t r = W_IO_RESULT (0);
    w_iterator_t i;

    w_dict_foreach (dict, i) {
        DUMP_INDENT (r, indent, stream);
        W_IO_CHAIN (r, w_io_format (stream, "$s: ",  w_dict_iterator_get_key (i)));
        W_IO_CHAIN (r, dump_value ((w_variant_t*) *i, stream, indent));
        W_IO_CHAIN (r, w_io_putchar (stream, '\n'));
    }

    return r;
}


w_io_result_t
w_cfg_dump (const w_cfg_t *cfg, w_io_t *stream)
{
    w_assert (cfg);
    w_assert (stream);
    return dump_dict (cfg, stream, 0);
}


w_io_result_t
w_cfg_dump_file (const w_cfg_t *cf, const char *path)
{
    w_assert (cf);
    w_assert (path);

    w_io_t *io = w_io_unix_open (path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (!io) return W_IO_RESULT_ERROR (errno ? errno : -1);

    w_io_result_t r = w_cfg_dump (cf, io);
    w_obj_unref (io);

    return r;
}


static char*
parse_identifier (w_parse_t *p)
{
    w_assert(p);

    w_buf_t buf = W_BUF;
    while (!isspace(p->look) && p->look != ':') {
        w_buf_append_char (&buf, p->look);
        w_parse_getchar (p);
    }

    w_parse_skip_ws (p);
    return w_buf_str (&buf);
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
        if (!(key = parse_identifier (p))) {
            w_parse_error (p, "Identifier expected");
        }

        /* Handle optional colon. */
        if (p->look == ':') {
            w_parse_getchar (p);
            w_parse_skip_ws (p);
        }

        switch (p->look) {
            case '"':
                if (!(svalue = w_parse_string (p))) {
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

    w_assert (input);

    w_parse_run (&parser, input, '#',
                 (w_parse_fun_t) w_cfg_parse_items,
                 result, &errmsg);

    if (errmsg) {
        w_obj_unref (result);
        w_assert (errmsg);
        if (!pmsg)
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

    w_assert (path);

    if (!(io = w_io_unix_open (path, O_RDONLY, 0))) {
        if (msg) *msg = w_strfmt ("Could not open file '%s' for reading", path);
        return NULL;
    }

    ret = w_cfg_load (io, msg);
    w_obj_unref (io);
    return ret;
}
