/*
 * wmeta.c
 * Copyright (C) 2012-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "../wheel.h"
#include <stdint.h>


static void
format_meta (w_io_t *io, const w_meta_item_t *meta, int indent)
{
    while (w_meta_item_is_valid (meta)) {
        if (indent) {
            int i = indent << 1;
            while (i--) W_IGNORE_RESULT (w_io_putchar (io, ' '));
        }

        switch (meta->type) {
            case W_META_TYPE_I8:  W_IGNORE_RESULT (w_io_format (io, "int8_t ")); break;
            case W_META_TYPE_I16: W_IGNORE_RESULT (w_io_format (io, "int16_t ")); break;
            case W_META_TYPE_I32: W_IGNORE_RESULT (w_io_format (io, "int32_t ")); break;
            case W_META_TYPE_I64: W_IGNORE_RESULT (w_io_format (io, "int64_t ")); break;
            case W_META_TYPE_U8 : W_IGNORE_RESULT (w_io_format (io, "uint8_t ")); break;
            case W_META_TYPE_U16: W_IGNORE_RESULT (w_io_format (io, "uint16_t ")); break;
            case W_META_TYPE_U32: W_IGNORE_RESULT (w_io_format (io, "uint32_t ")); break;
            case W_META_TYPE_U64: W_IGNORE_RESULT (w_io_format (io, "uint64_t ")); break;
            case W_META_TYPE_STR: W_IGNORE_RESULT (w_io_format (io, "char *")); break;
            case W_META_TYPE_BOOL:W_IGNORE_RESULT (w_io_format (io, "bool ")); break;
            default: break;
        }

        if (meta->type == W_META_TYPE_REG) {
            if (meta->alen)
                W_IGNORE_RESULT (w_io_format (io, "$s $s[$I] {\n",
                                              w_meta_desc_name (meta->mref),
                                              meta->name,
                                              meta->alen));
            else
                W_IGNORE_RESULT (w_io_format (io, "$s $s {\n",
                                              w_meta_desc_name (meta->mref),
                                              meta->name));
            format_meta (io, w_meta_desc_items (meta->mref), indent + 1);
            if (indent) {
                int i = indent << 1;
                while (i--) W_IGNORE_RESULT (w_io_putchar (io, ' '));
            }
            W_IGNORE_RESULT (w_io_format (io, "};\n"));
        }
        else {
            W_IGNORE_RESULT (w_io_format (io, "$s", meta->name));
            if (meta->alen)
                W_IGNORE_RESULT (w_io_format (io, "[$I];\n", meta->alen));
            else
                W_IGNORE_RESULT (w_io_format (io, ";\n"));
        }

        meta = w_meta_item_next (meta);
    }
}


int
main (int argc, char **argv)
{
    struct s {
        uint32_t i;
        char *str;
    };

    w_meta_t s_meta = {
        W_META ("struct s"),
        W_META_U32 (struct s, i),
        W_META_STR (struct s, str),
        W_META_END
    };

    struct t {
        struct s s1;
        struct s s2;
        int8_t    i;
        char *vstr[10];
        struct s vs[20];
    };

    w_meta_t t_meta = {
        W_META ("struct t"),
        W_META_REG (struct t, s1, s_meta),
        W_META_REG (struct t, s2, s_meta),
        W_META_STR_V (struct t, vstr, 10),
        W_META_REG_V (struct t, vs, s_meta, 20),
        W_META_END
    };

    w_unused (argc);
    w_unused (argv);

    W_IGNORE_RESULT (w_io_format (w_stdout,
                                  "$s {\n",
                                  w_meta_desc_name (s_meta)));
    format_meta (w_stdout, w_meta_desc_items (s_meta), 1);
    W_IGNORE_RESULT (w_io_format (w_stdout, "};\n\n"));

    W_IGNORE_RESULT (w_io_format (w_stdout,
                                  "$s {\n",
                                  w_meta_desc_name (t_meta)));
    format_meta (w_stdout, w_meta_desc_items (t_meta), 1);
    W_IGNORE_RESULT (w_io_format (w_stdout, "};\n"));

    return 0;
}


