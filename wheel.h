/*
 * wheel.h
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wheel_h__
#define __wheel_h__

#define w_lengthof(_v)  (sizeof(_v) / sizeof(0[_v]))
#define w_unused(_id)   ((void)(_id))

#ifdef __GNUC__
# define w_max(_n, _m) ({      \
		__typeof__(_n) __n = (_n); \
		__typeof__(_m) __m = (_m); \
		(__n > __m) ? __n : __m; })
#else /* !__GNUC__ */
# define w_max(_n, _m) \
	(((_n) > (_m)) ? (_n) : (_m))
#endif /* __GNUC__ */

#define _W_MAKE_STRING(__s) #__s
#define W_STRINGIZE(_s) _W_MAKE_STRING(_s)


typedef void** w_iterator_t;
typedef void* (*w_traverse_fun_t)(void *data, void *context);
typedef void  (*w_action_fun_t)(void *object, void *context);

enum w_bool { W_NO = 0, W_YES };
typedef enum w_bool wbool;

/*
 * Use this macros to control symbol visibility with recent GCC compilers
 * when target system uses the ELF binary format. This improves load times
 * and forbids access to library-private symbols. Consider also enabling
 * the "-fvisibility=hidden" flag for GCC >= 4.1.
 */
#if defined(__ELF__) && defined(__GNUC__) && \
	  (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)
# define W_EXPORT __attribute__((visibility("default")))
# define W_HIDDEN __attribute__((visibility("hidden")))
#else
# define W_EXPORT
# define W_HIDDEN
#endif


/*--------------------------------------------------[ libc includes ]-----*/

#ifdef _DEBUG
# include <assert.h>
# define w_assert      assert
#else
# define w_assert(_s)  ((void)0)
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <stdio.h>


/*------------------------------------------------[ memory handling ]-----*/

W_EXPORT void* w_malloc(size_t sz);
W_EXPORT void* w_realloc(void *ptr, size_t sz);

#define w_free(_x) \
	(free(_x), (_x) = NULL)

#define w_new(_t) \
	((_t *) w_malloc(sizeof(_t)))

#define w_alloc(_t, _n) \
	((_t *) w_malloc(sizeof(_t) * (_n)))

#define w_resize(_p, _t, _n) \
	((_t *) w_realloc(_p, sizeof(_t) * (_n)))


/*---------------------------------------------------[ errors/debug ]-----*/


W_EXPORT void  w_die(const char *fmt, ...);
W_EXPORT void w_vdie(const char *fmt, va_list al);


#ifdef _DPRINTF
# define w_dprintf(_x) __w_dprintf _x
W_EXPORT void __w_dprintf(const char *fmt, ...);
#else
# define w_dprintf(_x) ((void)0)
#endif


/*-----------------------------------------------[ string functions ]-----*/

W_EXPORT char* w_strfmtv(const char *fmt, va_list argl);
W_EXPORT char* w_strfmt (const char *fmt, ...);

W_EXPORT unsigned w_hashstrn(const char *str, size_t len);
W_EXPORT unsigned w_hashstr (const char *str);

W_EXPORT wbool w_str_bool  (const char *str, wbool *val);
W_EXPORT wbool w_str_int   (const char *str, int *val);
W_EXPORT wbool w_str_uint  (const char *str, unsigned *val);
W_EXPORT wbool w_str_long  (const char *str, long *val);
W_EXPORT wbool w_str_ulong (const char *str, unsigned long *val);
W_EXPORT wbool w_str_float (const char *str, float *val);
W_EXPORT wbool w_str_double(const char *str, double *val);


#if defined(__GNUC__)
# define w_strdup(str) ({ \
		__typeof__(str) __s = (str); \
	  w_strndup(__s, strlen(__s)); \
	})
#else
static inline char*
w_strdup(const char *str)
{
	return w_strndup(str, strlen(str));
}
#endif

#if defined(__GNUC__)
# define w_strndup(str, len) ({ __typeof__(len) __l = (len)+1; \
		(char*) memcpy(w_alloc(char, __l), (str), __l); })
#else
static inline char*
w_strndup(const char *str, size_t len)
{
	return (char*) memcpy(w_alloc(char, ++len), str, len);
}
#endif


#ifdef __GLIBC__
# define w_strcasecmp strcasecmp
#else  /* __GLIBC__ */
#include <ctype.h>
static inline int
w_strcasecmp(const char *s1, const char *s2)
{
	register int c1, c2;
	while ((*s1 != '\0') && (*s2 != '\0')) {
		c1 = tolower(*s1);
		c2 = tolower(*s2);
		if (c1 != c2) break;
		c1++;
		c2++;
	}
	return ((*s1 == '\0') && (*s2 == '\0')) ? 0 : ((c1 < c2) ? -1 : 1);
}
#endif /* __GLIBC__ */


static inline char*
w_strncpy(char *dst, const char *src, size_t n)
{
	char *result = strncpy(dst, src, n);
	dst[n-1] = '\0';
	return result;
}


/*---------------------------------------------------[ dictionaries ]-----*/

typedef struct w_dict w_dict_t;

W_EXPORT w_dict_t* w_dict_new(void);

W_EXPORT void w_dict_free(w_dict_t *d);
W_EXPORT void w_dict_clear(w_dict_t *d);

W_EXPORT unsigned w_dict_count(const w_dict_t *d);

W_EXPORT void* w_dict_getn(const w_dict_t *d, const char *key, size_t keylen);
W_EXPORT void  w_dict_setn(w_dict_t *d, const char *key, size_t keylen, void *data);

W_EXPORT void  w_dict_del(w_dict_t *d, const char *key);
W_EXPORT void  w_dict_set(w_dict_t *d, const char *key, void *data);
W_EXPORT void* w_dict_get(const w_dict_t *d, const char *key);

W_EXPORT void  w_dict_update(w_dict_t *d, const w_dict_t *o);

W_EXPORT void w_dict_traverse(w_dict_t *d, w_traverse_fun_t f, void *ctx);
W_EXPORT void w_dict_traverse_keys(w_dict_t *d, w_traverse_fun_t f, void *ctx);
W_EXPORT void w_dict_traverse_values(w_dict_t *d, w_traverse_fun_t f, void *ctx);

W_EXPORT w_iterator_t w_dict_first(const w_dict_t *d);
W_EXPORT w_iterator_t w_dict_next(const w_dict_t *d, w_iterator_t i);
W_EXPORT const char const* w_dict_iterator_get_key(w_iterator_t i);

/*
 * XXX: Never, NEVER change the layout of this structure. This **MUST**
 *      follow w_dict_node_t defined in wdict.c.
 * XXX: They key is totally unmodifiable, as it is insane to change it while
 *      traversing the dictionary.
 */
struct w_dict_item
{
	void *val;
	const char const* key;
};
typedef struct w_dict_item w_dict_item_t;


#define w_dict_item_first(_d) \
	((w_dict_item_t*) w_dict_first(_d))

#define w_dict_item_next(_d, _i) \
	((w_dict_item_t*) w_dict_next((_d), (w_iterator_t)(_i)))


/*----------------------------------------------------[ CLI parsing ]-----*/

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
	unsigned char  letter;
	const char    *string;
	w_opt_action_t action;
	void          *extra;
	const char    *info;
};

typedef struct w_opt w_opt_t;

#define W_OPT_CLI_ONLY  0x80

#define W_OPT_REMAINING_AS_FILES \
	{ 0, '-' | W_OPT_CLI_ONLY, "files", w_opt_files_action, NULL, \
		"Process remaining arguments as files." },                \

#define W_OPT_END \
	{ 0, 'h' | W_OPT_CLI_ONLY, "help", NULL, NULL,    \
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


#define _W_M_(x) W_EXPORT w_opt_status_t x(const w_opt_context_t*)
_W_M_(W_OPT_BOOL);
_W_M_(W_OPT_INT);
_W_M_(W_OPT_UINT);
_W_M_(W_OPT_LONG);
_W_M_(W_OPT_ULONG);
_W_M_(W_OPT_FLOAT);
_W_M_(W_OPT_DOUBLE);
_W_M_(W_OPT_STRING);
#undef _W_M_


W_EXPORT w_opt_status_t w_opt_files_action(const w_opt_context_t*);
W_EXPORT void w_opt_help(const w_opt_t opt[], FILE *out, const char *progname);

W_EXPORT void w_opt_parse(const w_opt_t  *opt,
                          w_action_fun_t file_fun,
                          void          *context,
                          int            argc,
                          const char   **argv);

W_EXPORT wbool w_opt_parse_file(const w_opt_t *opt,
                                FILE          *input,
                                char         **msg);


/*---------------------------------[ simple, piece-based LL parsers ]-----*/

struct w_parse_s {
    unsigned line;
    unsigned lpos;
    int      look;
    int      comment;
    FILE    *input;
    char    *error;
    void    *result;
    jmp_buf  jbuf;
};
typedef struct w_parse_s w_parse_t;

typedef void (*w_parse_fun_t) (w_parse_t*, void*);


W_EXPORT void* w_parse_run    (w_parse_t    *p,
                               FILE         *input,
                               int           comment,
                               w_parse_fun_t parse_fun,
                               void         *context,
                               char         **msg);

W_EXPORT void  w_parse_error   (w_parse_t *p, const char *fmt, ...);
W_EXPORT void  w_parse_ferror  (w_parse_t *p, const char *fmt, ...);
W_EXPORT void  w_parse_rerror  (w_parse_t *p);
W_EXPORT void  w_parse_getchar (w_parse_t *p);
W_EXPORT void  w_parse_skip_ws (w_parse_t *p);
W_EXPORT char* w_parse_string  (w_parse_t *p);
W_EXPORT char* w_parse_ident   (w_parse_t *p);
W_EXPORT char* w_parse_word    (w_parse_t *p);
W_EXPORT wbool w_parse_double  (w_parse_t *p, double *value);
W_EXPORT wbool w_parse_ulong   (w_parse_t *p, unsigned long *value);
W_EXPORT wbool w_parse_long    (w_parse_t *p, long *value);


/*---------------------------------------------------[ config files ]-----*/

typedef w_dict_t w_cfg_t;

enum w_cfg_type
{
	/* Marks en of parameter list for set/get. */
	W_CFG_END       = 0,

	/* Types for element nodes. */
	W_CFG_NONE      = 0,
	W_CFG_STRING    = 0x01,
	W_CFG_NUMBER    = 0x02,
	W_CFG_NODE      = 0x04,

	/* Flags for set/get. */
	W_CFG_SET       = 0x80,
	W_CFG_DEFAULT   = 0x40,

	/* Mask for the type. */
	W_CFG_TYPE_MASK =  W_CFG_STRING | W_CFG_NUMBER | W_CFG_NODE,

	/* Mask for the flags use in get/set. */
	W_CFG_FLAG_MASK = ~W_CFG_TYPE_MASK,
};

typedef enum w_cfg_type w_cfg_type_t;

W_EXPORT w_cfg_t* w_cfg_new(void);
W_EXPORT void w_cfg_free(w_cfg_t *cf);

W_EXPORT wbool w_cfg_has(const w_cfg_t *cf, const char *key);
W_EXPORT wbool w_cfg_del(w_cfg_t *cf, const char *key);
W_EXPORT wbool w_cfg_set(w_cfg_t *cf, ...);
W_EXPORT wbool w_cfg_get(const w_cfg_t *cf, ...);
W_EXPORT w_cfg_type_t w_cfg_type(const w_cfg_t *cf, const char *key);

W_EXPORT wbool    w_cfg_dump(const w_cfg_t *cf, FILE *output);
W_EXPORT w_cfg_t* w_cfg_load(FILE *input, char **msg);

W_EXPORT wbool    w_cfg_dump_file(const w_cfg_t *cf, const char *path);
W_EXPORT w_cfg_t *w_cfg_load_file(const char *path, char **msg);

#define w_cfg_set_string(cf, key, val) \
	w_cfg_set(cf, W_CFG_STRING, key, val, W_CFG_END)

#define w_cfg_set_number(cf, key, val) \
	w_cfg_set(cf, W_CFG_NUMBER, key, val, W_CFG_END)

#define w_cfg_set_node(cf, key, val) \
	w_cfg_set(cf, W_CFG_NODE, key, val, W_CFG_END)

#define _W_G(func_name, conf_type, c_type)              \
	static inline c_type w_cfg_get_ ## func_name         \
	(w_cfg_t *cf, const char *key, c_type defval)         \
	{ c_type value;                                        \
		if (w_cfg_get(cf, conf_type, key, &value, W_CFG_END)) \
			return value; else return defval;	}

_W_G( number, W_CFG_NUMBER, double      )
_W_G( string, W_CFG_STRING, const char* )
_W_G( node,   W_CFG_NODE,   w_cfg_t*    )

#undef _W_G

#define w_cfg_isnone(cf, key)    (W_CFG_NONE   == w_cfg_type(cf, key))
#define w_cfg_isnode(cf, key)    (W_CFG_NODE   == w_cfg_type(cf, key))
#define w_cfg_isnumber(cf, key)  (W_CFG_NUMBER == w_cfg_type(cf, key))
#define w_cfg_isstring(cf, key)  (W_CFG_STRING == w_cfg_type(cf, key))


/*-----------------------------------------------------------[ ttys ]-----*/

/*!
 * Obtains the dimensions (width & height) of the controlling terminal.
 *
 * \param cols Reference to a variable for the number of columns.
 * \param rows Reference to a variable for the number of rows.
 *
 * \return Wether the terminal size was guessed properly.
 */
W_EXPORT wbool w_tty_size(unsigned *cols, unsigned *rows);

/*!
 * Obtains the width of a row of the controlling terminal.
 *
 * \return Terminal width.
 */
W_EXPORT unsigned w_tty_cols(void);

/*!
 * Obtains the height of the the controlling terminal.
 *
 * \return Terminal height.
 */
W_EXPORT unsigned w_tty_rows(void);


typedef void (*w_tty_notify_fun_t)(unsigned, unsigned, void*);

/*!
 * Enables automatic tracking of the terminal size. Whenever the terminal
 * size changes, the supplied callback function will be called with the new
 * size of the terminal.
 *
 * \param function The function which will be notified of updates. Pass \c
 *		NULL in order to disable notifications and restore the signal handler
 *		for \c SIGWINCH.
 * \param context Context information which will be passed back to the
 *		callback function.
 * \return Wether the signal handler was correctly installed.
 *
 * \note This functionality requires the \c SIGWINCH signal to be defined.
 */
W_EXPORT wbool w_tty_size_notify(w_tty_notify_fun_t function, void *context);


#endif /* !__wheel_h__ */

