/*
 * wopt.h
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wopt_h__
#define __wopt_h__

#include "wdef.h"
#include <stdio.h>


enum w_opt_status
{
	W_OPT_OK,          /*!< All was correct. */
	W_OPT_EXIT_OK,     /*!< Exit the program with zero status. */
	W_OPT_EXIT_FAIL,   /*!< Exit the program with a nonzero status. */
	W_OPT_BAD_ARG,     /*!< Bad format or unconvertible argument. */
	W_OPT_MISSING_ARG, /*!< Required arguments not present. */
	W_OPT_FILES,       /*!< Remaining arguments are file names. */
};


typedef enum w_opt_status w_opt_status_t;


typedef struct w_opt_context w_opt_context_t;


/**
 * Type of option parsing action callbacks.
 */
typedef w_opt_status_t (*w_opt_action_t)(const w_opt_context_t*);


struct w_opt
{
	unsigned       narg;
	char           letter;
	const char    *string;
	w_opt_action_t action;
	void          *extra;
	const char    *info;
};


typedef struct w_opt w_opt_t;

#define W_OPT_REMAINING_AS_FILES \
	{ 0, '-', "files", w_opt_files_action, NULL, \
		"Process remaining arguments as files." },  \


#define W_OPT_END \
	{ 0, 'h', "help", NULL, NULL,                  \
		"Shows a summary of command line options." }, \
	{ 0, '\0', NULL, NULL, NULL, NULL }


struct w_opt_context
{
	const int      argc;
	const char   **argv;
	const w_opt_t *option;
	void          *userdata;
	const char   **argument;
};


#define M_(x) W_EXPORT w_opt_status_t x(const w_opt_context_t*)
M_(W_OPT_BOOL);
M_(W_OPT_INT);
M_(W_OPT_UINT);
M_(W_OPT_LONG);
M_(W_OPT_ULONG);
M_(W_OPT_FLOAT);
M_(W_OPT_DOUBLE);
M_(W_OPT_STRING);
#undef M_


W_EXPORT w_opt_status_t w_opt_files_action(const w_opt_context_t*);
W_EXPORT void w_opt_help(const w_opt_t opt[], FILE *out, const char *progname);
W_EXPORT void w_opt_parse(const w_opt_t *opt, w_action_fun_t file_fun,
    void *context, int argc, const char **argv);

#endif /* !__wopt_h__ */

