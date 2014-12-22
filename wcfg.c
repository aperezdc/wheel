/*
 * wcfg.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <fcntl.h>


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
        w_dict_t *d = w_dict_new (W_YES);
        w_variant_set_dict (node, d);
        w_obj_unref (d);
        return w_cfg_ensurenode (d, ++sep);
    }

    return node;
}


static w_bool_t
w_cfg_setv (w_cfg_t *cf, va_list args)
{
    w_bool_t ret = W_YES;
    w_cfg_type_t kind;
    w_variant_t *node;
    w_assert (cf);

    while ((kind = va_arg (args, w_cfg_type_t)) != W_CFG_END) {
        const char *key = va_arg (args, const char*);
        w_assert (key);

        if (!(node = w_cfg_ensurenode (cf, key)))
            ret = W_NO;

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
                ret = W_NO;
                break;
        }
    }
    return ret;
}


static w_bool_t
w_cfg_getv (const w_cfg_t *cf, va_list args)
{
    w_bool_t ret = W_YES;
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
            ret = W_NO;

        switch (kind) {
            case W_CFG_NUMBER:
                pnumber = va_arg (args, double*);
                if (node && w_variant_is_float (node))
                    *pnumber = w_variant_float (node);
                else ret = W_NO;
                break;
            case W_CFG_STRING:
                pstring = va_arg (args, const char**);
                if (node && w_variant_is_string (node))
                    *pstring = w_variant_string (node);
                else ret = W_NO;
                break;
            case W_CFG_NODE:
                pnode = va_arg (args, w_cfg_t**);
                if (node && w_variant_is_dict (node))
                    *pnode = w_variant_dict (node);
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


w_bool_t
w_cfg_set (w_cfg_t *cf, ...)
{
    w_bool_t ret;
    va_list args;
    w_assert (cf);
    va_start (args, cf);
    ret = w_cfg_setv (cf, args);
    va_end (args);
    return ret;
}


w_bool_t
w_cfg_get (const w_cfg_t *cf, ...)
{
    w_bool_t ret;
    va_list args;
    w_assert (cf);
    va_start (args, cf);
    ret = w_cfg_getv (cf, args);
    va_end (args);
    return ret;
}


w_bool_t
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


w_bool_t
w_cfg_del (w_cfg_t *cf, const char *key)
{
    w_variant_t *node;
    w_cfg_t *where = NULL;
    w_iterator_t it = NULL;

    w_assert (cf);
    w_assert (key);

    if (!(node = w_cfg_getnodelocation (cf, key, &it, &where)))
        return W_NO;

    w_assert (where);
    w_assert (it);

    w_dict_del (where, w_dict_iterator_get_key (it));

    return W_YES;
}



#define W_CFG_DUMP_INDENT(_l, _o)                       \
    do {                                                \
        unsigned __tmp = (_l) * 4;                      \
        while (__tmp--)                                 \
            if (w_io_failed (w_io_putchar ((_o), ' '))) \
                return W_NO;                            \
    } while (0)


static w_bool_t
w_cfg_dump_string (w_io_t *out, const char *str)
{
    w_assert (out);
    w_assert (str);

    if (w_io_failed (w_io_putchar (out, '"')))
        return W_NO;

    for (; *str; str++) {
#define ESCAPE(_c, _e) \
        case _c : if (w_io_failed (w_io_putchar (out, '\\')) || \
                      w_io_failed (w_io_putchar (out, (_e))))   \
                          return W_NO;                          \
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
                if (w_io_failed (w_io_putchar (out, *str)))
                    return W_NO;
        }
#undef ESCAPE
    }
    if (w_io_failed (w_io_putchar (out, '"')))
        return W_NO;
    return W_YES;
}


static w_bool_t
w_cfg_dump_cfg (const w_cfg_t *cf, w_io_t *out, unsigned indent)
{
    w_iterator_t i;
    w_variant_t *node;

    w_assert (cf);
    w_assert (out);

    w_dict_foreach (cf, i) {
        W_CFG_DUMP_INDENT (indent, out);

        node = (w_variant_t*) *i;

        if (w_io_failed (w_io_format (out, "$s ", w_dict_iterator_get_key (i))))
            return W_NO;

        switch (w_variant_type (node)) {
            case W_CFG_NODE:
                if (w_io_failed (w_io_putchar (out, '{')) ||
                    w_io_failed (w_io_putchar (out, '\n')) ||
                    !w_cfg_dump_cfg (w_variant_dict (node), out, indent + 1))
                    return W_NO;
                W_CFG_DUMP_INDENT (indent, out);
                if (w_io_failed (w_io_putchar (out, '}')))
                    return W_NO;
                break;
            case W_CFG_STRING:
                if (!w_cfg_dump_string (out, w_variant_string (node)))
                    return W_NO;
                break;
            case W_CFG_NUMBER:
                if (w_io_failed (w_io_format_double (out, w_variant_float (node))))
                    return W_NO;
                break;
            default:
                break;
        }
        if (w_io_failed (w_io_putchar (out, '\n')))
            return W_NO;
    }

    return W_YES;
}


/*
 * TODO Better error checking of IO functions.
 */
w_bool_t
w_cfg_dump (const w_cfg_t *cf, w_io_t *output)
{
    w_assert (cf);
    w_assert (output);
    return w_cfg_dump_cfg (cf, output, 0);
}


w_bool_t
w_cfg_dump_file (const w_cfg_t *cf, const char *path)
{
    w_io_t *io;
    w_bool_t ret;

    w_assert (cf);
    w_assert (path);

    if (!(io = w_io_unix_open (path, O_WRONLY | O_CREAT | O_TRUNC, 0666)))
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
