/*
 * wdoc.c
 * Copyright (C) 2014 aperez <aperez@hikari>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <ctype.h>


static bool opt_verbose = false;

#define VERBOSE(...) if (opt_verbose) w_printerr (__VA_ARGS__)


static const char*
map_objtype (int t)
{
    switch (t) {
        case 'f': return "function";
        case 'm': return "member";
        case 'M': return "macro";
        case 't': return "type";
        case 'v': return "var";
        default:
            w_die ("Invalid documentation specifier: '$c'\n", t);
            return ""; /* Keep compiler happy. */
    }
}


static void
handle_source_file (void *filenamep, void *ctx)
{
    w_unused (ctx);

    const char *filename = filenamep;
    VERBOSE ("Processing '$s'\n", filename);

    /* Prefer buffered I/O. */
#ifdef W_CONF_STDIO
    FILE *fp = fopen (filename, "rb");
    if (!fp) w_die ("Cannot open '$s': $E\n", filename);
    w_lobj w_io_t* input = w_io_stdio_open (fp);
#else
    w_lobj w_io_t* input = w_io_unix_open (filename, O_RDONLY, 0);
    if (!input) w_die ("Cannot open '$s': $E\n", filename);
#endif /* !W_CONF_STDIO */

    w_buf_t line = W_BUF;
    w_buf_t overflow = W_BUF;
    bool in_doc = false;
    bool indent = false;
    int objtype = 0;

    for (;; w_buf_clear (&line)) {
        w_io_result_t r = w_io_read_line (input, &line, &overflow, 0);

        if (w_io_failed (r))
            w_die ("Error reading '$s': $R\n", filename, r);

        if (w_buf_size (&line)) {
            const char *bdata = w_buf_const_data (&line);
            const size_t blen = w_buf_size (&line);
            size_t bpos = 0;

            /* Try to determine whether the line starts a doc-comment. */
            if (!in_doc) {
                in_doc = blen >= 3
                      && bdata[0] == '/'
                      && bdata[1] == '*'
                      && (bdata[2] == '*' || bdata[2] == '~');
                indent = bdata[2] == '~';
                bpos = 2;
                if (blen >= 4 && !isspace (bdata[3])) {
                    objtype = bdata[3];
                    bpos = 4;
                } else {
                    objtype = 0;
                }
            }

            /* Skip over code lines. */
            if (!in_doc)
                continue;

            /* Determine if the doc-comment is ending. */
            if ((blen >= 2 && bdata[0] == '*' && bdata[1] == '/') ||
                (blen >= 3 && isspace(bdata[0]) && bdata[1] == '*' && bdata[2] == '/'))
            {
                w_print ("\n");
                in_doc = false;
                continue;
            }

            /*
             * Strip leading stars (if any) and at most one space character
             * after it, but only if the star is given.
             */
            if (isspace (bdata[bpos]))
                bpos++;

            if (bpos < blen && bdata[bpos] == '*') {
                bpos++;
                /* Skip over (potential) space after star. */
                if (bpos < blen && isspace (bdata[bpos]))
                    bpos++;
            }

            if (bpos < blen) {
                if (objtype) {
                    w_printerr (" - $S\n", blen - bpos, bdata + bpos);
                    w_print (".. c:$s:: ", map_objtype (objtype));
                    objtype = 0;
                } else if (indent) {
                    w_print ("   ");
                }
                w_print ("$S\n", blen - bpos, bdata + bpos);
            } else {
                w_print ("\n");
            }
        }

        if (w_io_eof (r))
            break;
    }
}



static const w_opt_t options[] = {
    { 0, 'v', "verbose", W_OPT_BOOL, &opt_verbose,
      "Enable verbose operation" },
    W_OPT_END
};


int
main (int argc, char *argv[])
{
    w_opt_parse (options,
                 handle_source_file,
                 NULL,
                 "<filename...>",
                 argc,
                 argv);
    return EXIT_SUCCESS;
}
