/*
 * wopt.c
 * Copyright (c) 2006 Connectical Labs.
 * Adrian Perez <moebius@connectical.net>
 *
 * Distributed under terms of the MIT license.
 */

#include "wdef.h"
#include "wopt.h"
#include "werr.h"
#include "wstr.h"
#include "wtty.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
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
#define _MAKE_CONVERTER(name, type, func)                                    \
	w_opt_status_t name (const w_opt_context_t *context) {                     \
		unsigned i;                                                              \
		w_assert(context != NULL);                                               \
		w_assert(context->option != NULL);                                       \
		w_assert(context->option->narg > 0);                                     \
		w_assert(context->option->extra != NULL);                                \
		for (i = 0; i < context->option->narg; i++)                              \
			if (!func(context->argument[i], &((type*) context->option->extra)[i])) \
				return W_OPT_BAD_ARG;                                                \
		return W_OPT_OK;                                                         \
	}


_MAKE_CONVERTER( W_OPT_INT   , int          , w_str_int    )
_MAKE_CONVERTER( W_OPT_UINT  , unsigned     , w_str_uint   )
_MAKE_CONVERTER( W_OPT_LONG  , long         , w_str_long   )
_MAKE_CONVERTER( W_OPT_ULONG , unsigned long, w_str_ulong  )
_MAKE_CONVERTER( W_OPT_FLOAT , float        , w_str_float  )
_MAKE_CONVERTER( W_OPT_DOUBLE, double       , w_str_double )


w_opt_status_t
w_opt_files_action(const w_opt_context_t *ctx)
{
	w_unused(ctx);
	return W_OPT_FILES;
}


/*
 * XXX The converter for booleans is not declared using the _MAKE_CONVERTER
 * macro because it may be used without extra arguments (in that case, the
 * boolean value will be toggled when this function is applied).
 */
w_opt_status_t
W_OPT_BOOL(const w_opt_context_t *context)
{
	unsigned i;
	w_assert(context != NULL);
	w_assert(context->option != NULL);
	w_assert(context->option->extra != NULL);

	if (context->option->narg == 0) {
		*((wbool*) context->option->extra) = !*((wbool*) context->option->extra);
		return W_OPT_OK;
	}

	for (i = 0; i < context->option->narg; i++)
		if (!w_str_bool(context->argument[i], &((wbool*) context->option->extra)[i]))
			return W_OPT_BAD_ARG;

	return W_OPT_OK;
}


/*
 * This one goes for simple string assignment. Note that memory may be
 * leaked, as a copy of the arguments is made. It's up to the programmer
 * taking care of freeing that.
 */
w_opt_status_t
W_OPT_STRING(const w_opt_context_t *context)
{
	unsigned i;
	w_assert(context != NULL);
	w_assert(context->option != NULL);
	w_assert(context->option->narg > 0);
	w_assert(context->option->extra != NULL);

	for (i = 0; i < context->option->narg; i++)
		((char**) context->option->extra)[i] = w_strdup(context->argument[i]);

	return W_OPT_OK;
}


static inline const char*
_program_name (const char *str)
{
  /* FIXME Using a slash literal Unixish! */
  const char *slash = strrchr (str, '/');
  return (slash == NULL) ? str : (slash + 1);
}


static inline size_t
_longest_option_width(const w_opt_t *opt)
{
  size_t longest = 4; /* "help" has 4 characters */
  size_t cur_len;
  w_assert(opt != NULL);

  for (; opt->string != NULL; opt++) {
    cur_len = strlen(opt->string);
    if (cur_len > longest) longest = cur_len;
  }

  return longest;
}


static inline void
_print_some_chars(FILE *f, const char *s, size_t n)
{
  while (n-- && (*s != '\0')) fputc(*s++, f);
}


static inline void
_print_blanks(FILE *f, size_t n)
{
  while (n--) fputc(' ', f);
}


static inline void
_print_lspaced(FILE *f, const char *s, int l)
{
  unsigned tty_cols = w_tty_cols() - 10;
  const char *spc = s;
  unsigned col = 0;
  int lstart = 1;
  size_t len;
  if (l > 65) l = 20;

  /* Reflow words by inserting line breaks at spaces. */
  while (s != NULL) {
    spc = strchr(s, (spc == NULL) ? '\0' : ' ');
    len = (spc == NULL) ? strlen(s) : (size_t)(spc - s);
    if (lstart) {
      /* We're at line start, print always. */
      col = l + len;
      _print_some_chars(f, s, len);
      lstart = 0;
    }
    else if ((col + len) > tty_cols) {
      /* Advance to next line. */
      col = l + len;
      fputc('\n', f);
      _print_blanks(f, l);
      _print_some_chars(f, s, len);
    }
    else {
      col += len;
      _print_some_chars(f, s, len);
    }
    s = (spc == NULL) ? NULL : spc + 1;
    fputc(' ', f);
  }
  fputc('\n', f);
}



void
w_opt_help(const w_opt_t *opt, FILE *out)
{
	int width;
	w_assert(opt != NULL);
	w_assert(out != NULL);

	width = (int) _longest_option_width(opt);
	fprintf(out, "command line options:\n");

	for (; opt->string != NULL; opt++) {
		switch (opt->narg) {
			case 0:
				fprintf(out, "  -%c         --%-*s  ", opt->letter, width, opt->string);
				break;
			case 1:
				fprintf(out, "  -%c arg     --%-*s  ", opt->letter, width, opt->string);
				break;
			default:
				fprintf(out, "  -%c arg...  --%-*s  ", opt->letter, width, opt->string);
		}
		_print_lspaced(out, opt->info, 11 + width);
	}
}


static inline const w_opt_t*
_opt_lookup_shrt(const w_opt_t *opt, char chr)
{
  w_assert(opt != NULL);
  for (; opt->string != NULL; opt++)
    if (chr == opt->letter) return opt;
  return NULL;
}


static inline const w_opt_t*
_opt_lookup_fuzz(const w_opt_t *opt, const char *str, const char *prg)
{
  size_t len;
  const w_opt_t *ret;
  w_assert(opt != NULL);
  w_assert(str != NULL);
  w_assert(prg != NULL);

  len = strlen(str);
  for (; opt->string != NULL; opt++)
    if (!strncmp(opt->string, str, len))
      break;

  if (opt->string == NULL)
    return NULL;

  ret = opt++;

  /* Check wether the option string is not ambiguous. */
  for (; opt->string != NULL; opt++)
    if (!strncmp(opt->string, str, len))
      break;

  /* If we reach the end, no other prefix is equal -> ok. */
  if (opt->string == NULL)
    return ret;

  /* ...otherwise, we are in trouble. */
  fprintf(stderr, "%s: option \"%s\" is ambiguous:\n", prg, str);
  for (; ret->string != NULL; ret++)
    if (!strncmp(ret->string, str, len))
      fprintf(stderr, "    -%s\n", ret->string);
  fprintf(stderr, "Try \"%s --help\" for more information.\n", prg);

  exit(EXIT_FAILURE);
  return NULL; /* Never reached -- but keeps compiler happy =) */
}


static inline const w_opt_t*
_opt_lookup_long(const w_opt_t *opt, const char *str)
{
  w_assert(opt != NULL);
  w_assert(str != NULL);
  for (; opt->string != NULL; opt++)
    if (!strcmp(opt->string, str)) return opt;
  return NULL;
}


void
w_opt_parse(const w_opt_t *opt, w_action_fun_t file_fun,
    void *ctx, int argc, const char **argv)
{
  w_opt_status_t status = W_OPT_OK;
  w_opt_context_t context = { argc, argv, NULL, ctx, NULL };
  wbool files_only = W_NO;
  size_t i = 1;

  w_assert(opt != NULL);
  w_assert(argv != NULL);

  while (i < (unsigned) argc) {
    if (!files_only && (argv[i][0] == '-')) {
      /* option */
      context.option = ((argv[i][1] == '-') && (argv[i][2] != '\0'))
        ? _opt_lookup_long(opt, &argv[i][2])
        : ((argv[i][2] == '\0')
            ? _opt_lookup_shrt(opt,  argv[i][1])
            : _opt_lookup_fuzz(opt, &argv[i][1], _program_name(argv[0]))
          );

      if (context.option == NULL) {
				status = W_OPT_BAD_ARG;
				break;
			}
			if (context.option->letter == 'h') {
				w_opt_help(opt, stdout);
				status = W_OPT_EXIT_OK;
				break;
			}

			if (context.option->narg >= (unsigned)(argc - i)) {
				status = W_OPT_MISSING_ARG;
				break;
			}

			/* Prepare context. */
			context.argument = &argv[++i];

			/* Invoke action. */
			status = (*context.option->action)(&context);

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
			if (file_fun != NULL)
				(*file_fun)((void*) argv[i], ctx);
			i++;
		}
	}


	/* Perform final status-based actions. */
	switch (status) {
		case W_OPT_FILES: /* Nothing more to do. */
		case W_OPT_OK:
			break;
		case W_OPT_BAD_ARG: /* Handle errors. */
		case W_OPT_MISSING_ARG:
			if (context.option == NULL)
				fprintf(stderr, "%s: unknown option \"%s\"\n"
						"Try \"%s --help\" for more information.\n",
						_program_name(argv[0]), argv[i], _program_name(argv[0]));
			else
				fprintf(stderr, "%s: %s --%s\nTry \"%s --help\" for more information.\n",
						_program_name (argv[0]), (status == W_OPT_BAD_ARG)
							? "bad argument passed to"
							: "missing argument(s) to",
						context.option->string, _program_name(argv[0]));
			/* fall-through */
		case W_OPT_EXIT_FAIL: /* Exit with the given status hint. */
			exit(EXIT_FAILURE);
		case W_OPT_EXIT_OK:
			exit(EXIT_SUCCESS);
	}
}


