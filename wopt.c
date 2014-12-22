/*
 * wopt.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>


/*
 * XXX I'm way tooo lazy, so let the preprocessor make some functions for us.
 * This macro allows for easy creation of multi-argument conversion
 * functions for the command line option parser. Arguments to the macro are:
 *
 *   - name   Name of the created converter function. Choose a name which
 *            doesn't clash with other ones.
 *   - type   Type of the converted arguments.
 *   - func   Name of the primitive function used to convert a single item.
 *            For examples take a look at "wheel/wstr.h".
 */
#define _MAKE_CONVERTER(name, type, func)                                       \
    w_opt_status_t name (const w_opt_context_t *context) {                      \
        unsigned i;                                                             \
        w_assert (context != NULL);                                             \
        w_assert (context->option != NULL);                                     \
        w_assert (context->option->narg > 0);                                   \
        w_assert (context->option->extra != NULL);                              \
        for (i = 0; i < context->option->narg; i++)                             \
        if (!func (context->argument[i], &((type*) context->option->extra)[i])) \
            return W_OPT_BAD_ARG;                                               \
        return W_OPT_OK;                                                        \
    }


_MAKE_CONVERTER( W_OPT_INT   , int          , w_str_int    )
_MAKE_CONVERTER( W_OPT_UINT  , unsigned     , w_str_uint   )
_MAKE_CONVERTER( W_OPT_LONG  , long         , w_str_long   )
_MAKE_CONVERTER( W_OPT_ULONG , unsigned long, w_str_ulong  )
_MAKE_CONVERTER( W_OPT_FLOAT , float        , w_str_float  )
_MAKE_CONVERTER( W_OPT_DOUBLE, double       , w_str_double )


#define OPT_LETTER(_c) \
        ((_c) & ~W_OPT_CLI_ONLY)

#define CLI_LETTER(_c) \
        (((_c) & W_OPT_CLI_ONLY) == W_OPT_CLI_ONLY)


w_opt_status_t
w_opt_files_action (const w_opt_context_t *ctx)
{
    w_unused (ctx);
    return W_OPT_FILES;
}


/*
 * XXX The converter for booleans is not declared using the _MAKE_CONVERTER
 * macro because it may be used without extra arguments (in that case, the
 * boolean value will be toggled when this function is applied).
 */
w_opt_status_t
W_OPT_BOOL (const w_opt_context_t *context)
{
    unsigned i;
    w_assert (context != NULL);
    w_assert (context->option != NULL);
    w_assert (context->option->extra != NULL);

    if (context->option->narg == 0) {
        *((w_bool_t*) context->option->extra) = W_YES;
        return W_OPT_OK;
	}

    for (i = 0; i < context->option->narg; i++)
        if (!w_str_bool (context->argument[i], &((w_bool_t*) context->option->extra)[i]))
            return W_OPT_BAD_ARG;

    return W_OPT_OK;
}


/*
 * This one goes for simple string assignment. Note that memory may be
 * leaked, as a copy of the arguments is made. It's up to the programmer
 * taking care of freeing that.
 */
w_opt_status_t
W_OPT_STRING (const w_opt_context_t *context)
{
    unsigned i;
    w_assert (context != NULL);
    w_assert (context->option != NULL);
    w_assert (context->option->narg > 0);
    w_assert (context->option->extra != NULL);

    for (i = 0; i < context->option->narg; i++)
        ((char**) context->option->extra)[i] = w_str_dup (context->argument[i]);

    return W_OPT_OK;
}


w_opt_status_t
W_OPT_DATA_SIZE (const w_opt_context_t *ctx)
{
    w_assert (ctx);
    w_assert (ctx->option);
    w_assert (ctx->option->extra);
    w_assert (ctx->option->narg == 1);

    return (!w_str_size_bytes (ctx->argument[0], ctx->option->extra))
            ? W_OPT_BAD_ARG
            : W_OPT_OK;
}


w_opt_status_t
W_OPT_TIME_PERIOD (const w_opt_context_t *ctx)
{
    w_assert (ctx);
    w_assert (ctx->option);
    w_assert (ctx->option->extra);
    w_assert (ctx->option->narg == 1);

    return (!w_str_time_period (ctx->argument[0], ctx->option->extra))
            ? W_OPT_BAD_ARG
            : W_OPT_OK;
}


static inline const char*
_program_name (const char *str)
{
    /* FIXME Using a slash literal Unixish! */
    const char *slash = strrchr (str, '/');
    return (slash == NULL) ? str : (slash + 1);
}


static inline void
_print_some_chars (w_io_t *f, const char *s, size_t n)
{
    while (n-- && (*s != '\0')) {
        /* TODO: Maybe hanle I/O errors. */
        (void) w_io_putchar (f, *s++);
    }
}


static inline void
_print_blanks (w_io_t *f, size_t n)
{
    while (n--) {
        /* TODO: Maybe hanle I/O errors. */
        (void) w_io_putchar (f, ' ');
    }
}


static inline void
_print_lspaced (w_io_t *f, const char *s, int l)
{
    unsigned tty_cols = w_tty_cols () - l - 22;
    const char *spc = s;
    unsigned col = 0;
    int lstart = 1;
    size_t len;
    if (l > 65) l = 20;

    if (tty_cols > 60) {
        tty_cols = 60;
    }

    /* Reflow words by inserting line breaks at spaces. */
    while (s != NULL) {
        spc = strchr (s, (spc == NULL) ? '\0' : ' ');
        len = (spc == NULL) ? strlen (s) : (size_t) (spc - s);
        if (lstart) {
            /* We're at line start, print always. */
            col = l + len;
            _print_some_chars (f, s, len);
            lstart = 0;
        }
        else if ((col + len) > tty_cols) {
            /* Advance to next line. */
            col = l + len;
            /* TODO: Maybe hanle I/O errors. */
            (void) w_io_putchar (f, '\n');
            _print_blanks (f, l);
            _print_some_chars (f, s, len);
        }
        else {
            col += len;
            _print_some_chars (f, s, len);
        }
        s = (spc == NULL) ? NULL : spc + 1;
        /* TODO: Maybe hanle I/O errors. */
        (void) w_io_putchar (f, ' ');
    }
    /* TODO: Maybe hanle I/O errors. */
    (void) w_io_putchar (f, '\n');
}


void
w_opt_help (const w_opt_t *opt,
            w_io_t        *out,
            const char    *progname,
            const char    *syntax)
{
    w_assert (opt != NULL);
    w_assert (out != NULL);

    /* TODO: Handle I/O errors. */
    (void) w_io_format (out, "Usage: $s [options] $s\n"
                        "Command line options:\n\n", progname,
                        (syntax != NULL) ? syntax : "");

    for (; opt->string != NULL; opt++) {
        if (OPT_LETTER (opt->letter) && opt->string) {
            (void) w_io_format (out, "-$c, --$s ",
                                OPT_LETTER (opt->letter),
                                opt->string);
        }
        else if (opt->string) {
            (void) w_io_format (out, "--$s ", opt->string);
        }
        else {
            (void) w_io_format (out, "-$c ", OPT_LETTER (opt->letter));
        }

        switch (opt->narg) {
            case  0: (void) w_io_format (out, "\n   "); break;
            case  1: (void) w_io_format (out, "<ARG>\n   "); break;
            default: (void) w_io_format (out, "<ARG...>\n   ");
        }
        _print_lspaced (out, opt->info, 3);
        (void) w_io_putchar (out, '\n');
    }
}


static inline const w_opt_t*
_opt_lookup_shrt (const w_opt_t *opt, char chr)
{
    w_assert (opt != NULL);
    for (; opt->string != NULL; opt++)
        if (chr == OPT_LETTER (opt->letter))
            return opt;

    return NULL;
}


static inline const w_opt_t*
_opt_lookup_fuzz (const w_opt_t *opt, const char *str, const char *prg)
{
    size_t len;
    const w_opt_t *ret;

    w_assert (opt != NULL);
    w_assert (str != NULL);
    w_assert (prg != NULL);

    len = strlen (str);
    for (; opt->string != NULL; opt++)
        if (!strncmp (opt->string, str, len))
            break;

    if (opt->string == NULL)
        return NULL;

    ret = opt++;

    /* Check wether the option string is not ambiguous. */
    for (; opt->string != NULL; opt++)
        if (!strncmp (opt->string, str, len))
            break;

    /* If we reach the end, no other prefix is equal -> ok. */
    if (opt->string == NULL)
        return ret;

    /* ...otherwise, we are in trouble. */
    (void) w_io_format (w_stderr,
                        "$s: option '$s' is ambiguous, possibilities:\n",
                        prg, str);
    for (; ret->string != NULL; ret++) {
        if (!strncmp (ret->string, str, len)) {
            (void) w_io_format (w_stderr, "    --$s\n", ret->string);
        }
    }
    (void) w_io_format (w_stderr, "Hint: try '$s --help'\n", prg);

    exit (EXIT_FAILURE);
    return NULL; /* Never reached -- but keeps compiler happy =) */
}


static inline const w_opt_t*
_opt_lookup_long (const w_opt_t *opt, const char *str)
{
    w_assert (opt != NULL);
    w_assert (str != NULL);
    for (; opt->string != NULL; opt++)
        if (!strcmp (opt->string, str)) return opt;
    return NULL;
}


unsigned
w_opt_parse (const w_opt_t *options,
             w_action_fun_t file_cb,
             void          *userdata,
             const char    *syntax,
             int            argc,
             char         **argv)
{
    w_opt_status_t status = W_OPT_OK;
    w_opt_context_t context = { argc, argv, NULL, userdata, NULL };
    w_bool_t files_only = W_NO;
    size_t i = 1;

    w_assert (options != NULL);
    w_assert (argv != NULL);

    while (i < (unsigned) argc) {
        if (!files_only && (argv[i][0] == '-')) {
            /* option */
            context.option = ((argv[i][1] == '-') && (argv[i][2] != '\0'))
                ? _opt_lookup_long (options, &argv[i][2])
                : ((argv[i][2] == '\0')
                   ? _opt_lookup_shrt (options,  argv[i][1])
                   : _opt_lookup_fuzz (options, &argv[i][1], _program_name (argv[0]))
                  );

            if (context.option == NULL) {
                status = W_OPT_BAD_ARG;
                break;
            }
            if (OPT_LETTER (context.option->letter) == 'h') {
                w_opt_help (options, w_stdout, _program_name (argv[0]), syntax);
                status = W_OPT_EXIT_OK;
                break;
			}

            if (context.option->narg >= (unsigned) (argc - i)) {
                status = W_OPT_MISSING_ARG;
                break;
            }

            /* Prepare context. */
            context.argument = &argv[++i];

            /* Invoke action. */
            status = (*context.option->action) (&context);

            /* If --files (or similar) was given change flag and continue */
            if (status == W_OPT_FILES) {
                files_only = 1;
                continue;
            }

            /* Bail out of this is not a good status */
            if (status != W_OPT_OK)
                break;

            /* Advance in arguments. */
            i += context.option->narg;
        }
        else {
            if (file_cb != NULL)
                (*file_cb) ((void*) argv[i++], userdata);
            else
                return i;
        }
    }

    /* Perform final status-based actions. */
    switch (status) {
        case W_OPT_FILES: /* Nothing more to do. */
        case W_OPT_OK:
            break;
        case W_OPT_BAD_ARG: /* Handle errors. */
        case W_OPT_MISSING_ARG:
            if (context.option == NULL) {
                (void) w_io_format (w_stderr,
                                    "$s: unknown option '$s'\nHint: try '$s --help'\n",
                                    _program_name (argv[0]), argv[i],
                                    _program_name (argv[0]));
            }
            else {
                (void) w_io_format (w_stderr,
                                    "$s: $s --$s\nTry \"$s --help\" for more information.\n",
                                    _program_name (argv[0]), (status == W_OPT_BAD_ARG)
                                                               ? "bad argument passed to"
                                                               : "missing argument(s) to",
                                                   context.option->string,
                                                   _program_name (argv[0]));
            }
            /* fall-through */
        case W_OPT_EXIT_FAIL: /* Exit with the given status hint. */
            exit (EXIT_FAILURE);
        case W_OPT_EXIT_OK:
            exit (EXIT_SUCCESS);
    }

    return i;
}


#ifndef W_OPT_PARSE_ARG_CHUNK
#define W_OPT_PARSE_ARG_CHUNK 5
#endif /* W_OPT_PARSE_ARG_CHUNK */


static void
_w_opt_parse_file (w_parse_t *p, void *ctx)
{
    w_opt_status_t status;
    const w_opt_t *options = (w_opt_t*) ctx;
    const w_opt_t *opt;
    char *token;
    char **args;
    unsigned maxarg = 0;
    unsigned narg = 0;

    w_assert (p != NULL);
    w_assert (options != NULL);

    while (w_likely (p->look != W_IO_EOF)) {
        if ((token = w_parse_word (p)) == NULL) {
            w_parse_error (p, "Identifier expected");
        }

        opt = _opt_lookup_long (options, token);

        if (opt == NULL || CLI_LETTER (opt->letter)) {
            w_parse_ferror (p, "No such option '$s'", token);
            w_free (token);
            w_parse_rerror (p);
        }

        if (opt->narg > 0) {
            narg   = 0;
            maxarg = W_OPT_PARSE_ARG_CHUNK;
            args   = w_alloc (char*, maxarg);

            while (narg < opt->narg && p->look != W_IO_EOF) {
                if (narg >= maxarg) {
                    maxarg += W_OPT_PARSE_ARG_CHUNK;
                    args    = w_resize (args, char*, maxarg);
                }
                args[narg++] = (p->look == '"')
                    ? w_parse_string (p)
                    : w_parse_word (p);
            }

            /* Not enough arguments given */
            if (p->look != W_IO_EOF && narg < opt->narg) {
                unsigned i;
                for (i = 0; i < narg; i++)
                    w_free (args[i]);
                w_free (args);

                w_parse_ferror (p, "Insufficient arguments to '$s'", token);
                w_free (token);
                w_parse_rerror (p);
            }

            {
                /* Ok, arguments gathered, now let's try to invoke the action */
                w_opt_context_t oc = { narg, args, opt, NULL, args };
                status = (*opt->action) (&oc);
            }

            /* Clean up after ourselves */
            for (narg = 0; narg < opt->narg; narg++)
                w_free (args[narg]);
            w_free (args);

        }
        else {
            w_opt_context_t oc = { 0, NULL, opt, NULL, NULL };
            status = (*opt->action) (&oc);
        }

        /* Did something go wrong? Notify error */
        if (status != W_OPT_OK) {
            w_parse_ferror (p, "Arguments to '$s' are invalid", token);
            w_free (token);
            w_parse_rerror (p);
        }
    }
}


w_bool_t
w_opt_parse_io (const w_opt_t *opt,
                w_io_t        *input,
                char         **msg)
{
    char *errmsg;
    w_bool_t ret;

    w_parse_t parser;

    w_assert (opt != NULL);
    w_assert (input != NULL);

    w_parse_run (&parser, input, '#',
                 _w_opt_parse_file,
                 (void*) opt, &errmsg);


    ret = (errmsg == NULL);

    if (msg == NULL)
        w_free (errmsg);
    else
        *msg = errmsg;

    return ret;
}
