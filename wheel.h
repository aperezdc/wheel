/*
 * Main include file for libwheel.
 *
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wheel_h__
#define __wheel_h__

/*!
 * \defgroup misc Miscelaneous utilities
 * \addtogroup misc
 * \{
 */

/*!
 * Obtain the number of elements in a statically allocated array.
 */
#define w_lengthof(_v)  (sizeof(_v) / sizeof(0[_v]))

/*!
 * Mark a variable as unused, to avoid compiler warnings.
 */
#define w_unused(_id)   ((void)(_id))

#ifdef __GNUC__
# define w_max(_n, _m) ({      \
		__typeof__(_n) __n = (_n); \
		__typeof__(_m) __m = (_m); \
		(__n > __m) ? __n : __m; })
#else /* !__GNUC__ */
/*!
 * Obtain the highest of two numbers.
 */
# define w_max(_n, _m) \
	(((_n) > (_m)) ? (_n) : (_m))
#endif /* __GNUC__ */


#ifdef __GNUC__
# define w_min(_n, _m) ({      \
		__typeof__(_n) __n = (_n); \
		__typeof__(_m) __m = (_m); \
		(__n < __m) ? __n : __m; })
#else /* !__GNUC__ */
/*!
 * Obtain the lowest of two numbers.
 */
# define w_min(_n, _m) \
	(((_n) < (_m)) ? (_n) : (_m))
#endif /* __GNUC__ */


#ifdef __GNUC__
# define w_likely(_expr)   __builtin_expect ((_expr), 1)
# define w_unlikely(_expr) __builtin_expect ((_expr), 0)
#else /* !__GNUC__ */
# define w_likely
# define w_unlikely
#endif /* __GNUC__ */

#define _W_MAKE_STRING(__s) #__s
#define W_STRINGIZE(_s) _W_MAKE_STRING(_s)


typedef void** w_iterator_t;
typedef void* (*w_traverse_fun_t)(void *data, void *context);
typedef void  (*w_action_fun_t)(void *object, void *context);

typedef enum wbool wbool;

/*!
 * Boolean data type.
 */
enum wbool
{
    W_NO = 0, /*!< False value. */
    W_YES     /*!< True value. */
};

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

/*\}*/

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

#ifdef W_CONF_STDIO
#include <stdio.h>
#endif /* W_CONF_STDIO */


/*------------------------------------------------[ memory handling ]-----*/

/*!
 * \defgroup wmem Memory handling
 * \addtogroup wmem
 * \{
 */

/*!
 * Wrapper around malloc(). This will abort and print a message when the
 * pointer returned by malloc() is NULL.
 */
W_EXPORT void* w_malloc(size_t sz);

/*!
 * Wrapper around realloc(). This will abort and print a message when the
 * pointer returned by realloc() is NULL. Plus, it is possible to pass
 * a NULL pointer and/or a zero size, and it will behave consistently:
 *
 *  - <tt>w_realloc(NULL, 42)</tt> is equivalent to <tt>malloc(42)</tt>
 *  - <tt>w_relloac(ptr, 0)</tt> is equivalent to <tt>free(ptr)</tt>
 */
W_EXPORT void* w_realloc(void *ptr, size_t sz);

/*!
 * Frees memory and sets the pointer to \c NULL.
 * \param _x A pointer.
 */
#define w_free(_x) \
	(free(_x), (_x) = NULL)

/*!
 * Allocates a new chunk of memory suitable for a value of some type.
 * \param _t Type name for which the chunk will be allocated.
 * \sa w_new0(), w_alloc(), w_alloc0(), w_free()
 */
#define w_new(_t) \
	((_t *) w_malloc(sizeof(_t)))

/*!
 * Allocates a zero-filled chunk of memory suitable for a value of some type.
 * \param _t Type name for which the chunk will be allocated.
 * \sa w_new(), w_alloc(), w_alloc0(), w_free()
 */
#define w_new0(_t) \
    ((_t *) memset (w_new (_t), 0x00, sizeof (_t)))

/*!
 * Allocate an array of memory for items of some type.
 * \param _t Type name of the elements.
 * \param _n Number of elements to allocate memory for.
 * \sa w_new(), w_new0(), w_alloc0(), w_free(), w_resize()
 */
#define w_alloc(_t, _n) \
	((_t *) w_malloc(sizeof(_t) * (_n)))

/*!
 * Allocate a zero-filled array of memory for items of some type.
 * \param _t Type name of the elements.
 * \param _n Number of elements to allocate memory for.
 * \sa w_new(), w_new0(), w_alloc(), w_free(), w_resize()
 */
#define w_alloc0(_t, _n) \
    ((_t *) memset (w_alloc ((_t), (_n)), 0x00, sizeof (_t) * (_n)))

/*!
 * Resize a chunk of memory containing items of some type.
 * \param _p Valid pointer to a memory area.
 * \param _t Type name of the elements.
 * \param _n Number of elements to allocate memory for.
 * \sa w_new(), w_new0(), w_alloc(), w_alloc0(), w_free()
 */
#define w_resize(_p, _t, _n) \
	((_t *) w_realloc(_p, sizeof(_t) * (_n)))

/*\}*/


/*------------------------------------------------[ simple objects ]------*/

/*!
 * \defgroup wobj Poor man's object orientation
 * \addtogroup wobj
 * \{
 */

#define W_OBJ_DECL(_c) \
    typedef struct _c ## __class _c

#define W_OBJ_DEF(_c) \
    struct _c ## __class

#define W_OBJ(_c) \
    W_OBJ_DECL (_c); \
    W_OBJ_DEF  (_c)

#define W_OBJ_STATIC(_dtor) \
    { (size_t) -1, (void (*)(void*)) (_dtor) }


W_OBJ (w_obj_t)
{
    size_t __refs;
    void (*__dtor) (void*);
};


/*!
 * Initializes an object.
 * Subclasses should do not need to call this base initializer. It will be
 * automatically called when using \ref w_obj_new().
 */
static inline void*
w_obj_init (w_obj_t *obj)
{
    w_assert (obj);
    obj->__refs = 1;
    return obj;
}

/*!
 * Instantiates an object, with space allocated for a private data area.
 */
#define w_obj_new_with_priv_sized(_t, _s) \
    ((_t *) w_obj_init (w_malloc (sizeof (_t) + (_s))))

/*!
 * Instantiates an object, with space allocated for a private data area.
 * The size of the private area will be that of a type names after the
 * object type, with a \c _p suffix added to it.
 */
#define w_obj_new_with_priv(_t) \
    w_obj_new_with_priv_sized (_t, sizeof (_t ## _p))

/*!
 * Instantiates an object.
 */
#define w_obj_new(_t) \
    ((_t *) w_obj_init (w_malloc (sizeof (_t))))

/*!
 * Obtains a pointer to the private data area of an object.
 */
#define w_obj_priv(_p, _t) \
    ((void*) (((char*) (_p)) + sizeof (_t)))

/*!
 * Increases the reference counter of an object, returns the object itself.
 */
void* w_obj_ref (void *obj);

/*!
 * Decreases the reference counter of an object, returns the object itself.
 * Once the reference count for an object reaches zero, it is destroyed by
 * calling \ref w_obj_destroy().
 */
void* w_obj_unref (void *obj);

/*!
 * Destroys an object. If a destructor function was set for the object
 * (using \ref w_obj_dtor()), then it will be run prior to the memory
 * used by the object being freed.
 */
void w_obj_destroy (void *obj);

/*!
 * Registers a destructor to be called when an object is destroyed.
 */
void* w_obj_dtor (void *obj, void (*fini) (void*));

/*\}*/


/*------------------------------------------[ forward declarations ]------*/

W_OBJ_DECL (w_io_t);


/*---------------------------------------------------[ errors/debug ]-----*/

/*!
 * \defgroup debug Debugging support
 * \addtogroup debug
 * \{
 */

/*!
 * Prints a message to stderr and aborts execution.
 * \param fmt Format string for the message. If \c NULL is passed, then no
 *            message is written to stderr.
 */
W_EXPORT void w_die (const char *fmt, ...);

/*!
 * Prints a message to stderr and aborts execution.
 *
 * \param fmt Format string for the message. If \c NULL is passed, then no
 *            message is written to stderr.
 * \param al  List of arguments to be consumed as specified in the format
 *            string.
 */
W_EXPORT void w_diev (const char *fmt, va_list al);


#ifdef _DEBUG_PRINT
# define w_debug(_x) __w_debug _x
W_EXPORT void __w_debug (const char *fmt, ...);
#else
# define w_debug(_x) ((void)0)
#endif

/*\}*/

/*-----------------------------------------------[ string functions ]-----*/

/*!
 * \defgroup wstr String functions
 * \addtogroup wstr
 * \{
 */

W_EXPORT char* w_strfmtv(const char *fmt, va_list argl);
W_EXPORT char* w_strfmt (const char *fmt, ...);

/*!
 * Hashes the start of a string.
 * \param str String to get the hash of.
 * \param len Number of characters to hash.
 * \return Hash value.
 */
W_EXPORT unsigned w_str_hashl (const char *str, size_t len);

/*!
 * Hashes a string.
 * \param str String to get the hash of.
 * \return Hash value.
 */
W_EXPORT unsigned w_str_hash (const char *str);

/*!
 * Converts a string into a boolean.
 */
W_EXPORT wbool w_str_bool  (const char *str, wbool *val);

/*!
 * Converts a string into an integer.
 */
W_EXPORT wbool w_str_int   (const char *str, int *val);

/*!
 * Converts a string inyo an unsigned integer.
 */
W_EXPORT wbool w_str_uint  (const char *str, unsigned *val);

/*!
 * Converts a string into a long integer.
 */
W_EXPORT wbool w_str_long  (const char *str, long *val);

/*!
 * Converts a string into a long unsigned integer.
 */
W_EXPORT wbool w_str_ulong (const char *str, unsigned long *val);

/*!
 * Converts a string into a float number.
 */
W_EXPORT wbool w_str_float (const char *str, float *val);

/*!
 * Converts a string into a double-precision float number.
 */
W_EXPORT wbool w_str_double(const char *str, double *val);


static inline char*
w_str_dupl (const char *str, size_t len)
{
    char *r;
    if (str == NULL)
        return NULL;
    r = (char*) memcpy(w_alloc(char, len + 1), str, len);
    r[len] = '\0';
    return r;
}


static inline char*
w_str_dup (const char *str)
{
	if (str == NULL)
	    return NULL;
	return w_str_dupl (str, strlen (str));
}



#ifdef __GLIBC__
# define w_str_casecmp strcasecmp
#else  /* __GLIBC__ */
#include <ctype.h>
static inline int
w_str_casecmp(const char *s1, const char *s2)
{
	register int c1 = 0;
	register int c2 = 0;
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
	dst[n] = '\0';
	return result;
}

/*\}*/

/*---------------------------------------------------[ dictionaries ]-----*/

/*!
 * \defgroup wdict Hash-based dictionaries
 * \addtogroup wdict
 * \{
 */

typedef struct w_dict_node_t w_dict_node_t;

W_OBJ (w_dict_t)
{
    w_obj_t         parent;
    w_dict_node_t **nodes;
    w_dict_node_t  *first;
    size_t          count;
    size_t          size;
};

/*!
 * Create a new dictionary.
 */
W_EXPORT w_dict_t* w_dict_new (void);

/*!
 * Clears the contents of a dictionary.
 */
W_EXPORT void w_dict_clear (w_dict_t *d);

/*!
 * Get the number of items in a dictionary.
 */
#define w_dict_count(_d) \
    (w_assert (_d), (_d)->count)

W_EXPORT void* w_dict_getn (const w_dict_t *d, const char *key, size_t keylen);
W_EXPORT void  w_dict_setn (w_dict_t *d, const char *key, size_t keylen, void *data);
W_EXPORT void  w_dict_deln (w_dict_t *d, const char *key, size_t keylen);

/*!
 * Delete an item from a dictionary given its key.
 */
W_EXPORT void  w_dict_del (w_dict_t *d, const char *key);

/*!
 * Set an item in a dictionary.
 */
W_EXPORT void  w_dict_set (w_dict_t *d, const char *key, void *data);

/*!
 * Get an item from a dictionary.
 */
W_EXPORT void* w_dict_get (const w_dict_t *d, const char *key);

/*!
 * Update a dictionary with the contents of another. For each key present in
 * the source dictionary, copy it and its value to the destination one, if
 * the key already existed in the destination, the value gets overwritten.
 * \param d Destination dictionary.
 * \param o Source dictionary.
 */
W_EXPORT void  w_dict_update (w_dict_t *d, const w_dict_t *o);

W_EXPORT void w_dict_traverse (w_dict_t *d, w_traverse_fun_t f, void *ctx);
W_EXPORT void w_dict_traverse_keys (w_dict_t *d, w_traverse_fun_t f, void *ctx);
W_EXPORT void w_dict_traverse_values (w_dict_t *d, w_traverse_fun_t f, void *ctx);

W_EXPORT w_iterator_t w_dict_first (const w_dict_t *d);
W_EXPORT w_iterator_t w_dict_next (const w_dict_t *d, w_iterator_t i);
W_EXPORT const char const* w_dict_iterator_get_key (w_iterator_t i);

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
	((w_dict_item_t*) w_dict_first (_d))

#define w_dict_item_next(_d, _i) \
	((w_dict_item_t*) w_dict_next ((_d), (w_iterator_t)(_i)))

#define w_dict_foreach(_d, _i) \
    for ((_i) = w_dict_first (_d); (_i) != NULL; (_i) = w_dict_next ((_d), (_i)))

W_EXPORT void w_dict_free (void *d);

/*\}*/

/*----------------------------------------------------[ CLI parsing ]-----*/

/*!
 * \defgroup wopt Command line parsing
 * \addtogroup wopt
 * \{
 */

enum w_opt_status_t
{
	W_OPT_OK,          /*!< All was correct. */
	W_OPT_EXIT_OK,     /*!< Exit the program with zero status. */
	W_OPT_EXIT_FAIL,   /*!< Exit the program with a nonzero status. */
	W_OPT_BAD_ARG,     /*!< Bad format or unconvertible argument. */
	W_OPT_MISSING_ARG, /*!< Required arguments not present. */
	W_OPT_FILES,       /*!< Remaining arguments are file names. */
};

typedef enum w_opt_status_t w_opt_status_t;

typedef struct w_opt_context_t w_opt_context_t;

/*!
 * Type of option parsing action callbacks.
 */
typedef w_opt_status_t (*w_opt_action_t)(const w_opt_context_t*);


/*!
 * Command line option information.
 */
struct w_opt_t
{
	unsigned       narg;   /*!< Number of arguments consumed.           */
	unsigned char  letter; /*!< Letter for short option style parsing.  */
	const char    *string; /*!< String for long option style parsing.   */
	w_opt_action_t action; /*!< Action performed when option is parsed. */
	void          *extra;  /*!< Additional argument to the action.      */
	const char    *info;   /*!< Text describing the option.             */
};

typedef struct w_opt_t w_opt_t;

#define W_OPT_CLI_ONLY  0x80

#define W_OPT_REMAINING_AS_FILES \
	{ 0, '-' | W_OPT_CLI_ONLY, "files", w_opt_files_action, NULL, \
		"Process remaining arguments as files." },                \

#define W_OPT_END \
	{ 0, 'h' | W_OPT_CLI_ONLY, "help", NULL, NULL,    \
		"Shows a summary of command line options." }, \
	{ 0, '\0', NULL, NULL, NULL, NULL }


struct w_opt_context_t
{
	const int      argc;
	char         **argv;
	const w_opt_t *option;
	void          *userdata;
	char         **argument;
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

/*!
 * Generates a long help message for a set of options.
 *
 * \param opt Array of options.
 * \param out Stream where write the message.
 * \param progname Program name (usually <tt>argv[0]</tt>).
 * \param syntax Additional command line syntax information. May be \c NULL.
 *
 * The generated output looks like this:
 * <pre>
 * Usage: <i>progname</i> [options] <i>syntax</i>
 * Command line options:
 *
 * <i>--option, -x</i> <ARG>
 *    <i>Option help text.</i>
 * </pre>
 */
W_EXPORT void w_opt_help(const w_opt_t opt[],
                         w_io_t       *out,
                         const char   *progname,
                         const char   *syntax);

/*!
 * Parses an array of command line arguments.
 *
 * \param options  Array of options.
 * \param file_cb  Callback invoked for each non-option argument found (most
 *                 likely files, thus the name).
 * \param userdata User data, this is passed to the \c file_cb callback.
 * \param syntax   Description on how to pass extra command line argument.
 *                 This is passed directly to \ref w_opt_help.
 * \param argc     Number of command line arguments.
 * \param argv     Array of command line arguments.
 *
 * \return         Number of consumed arguments.
 */
W_EXPORT unsigned w_opt_parse(const w_opt_t  *options,
                              w_action_fun_t file_cb,
                              void          *userdata,
                              const char    *syntax,
                              int            argc,
                              char         **argv);

/*!
 * Uses long option information to parse simple config-like files.
 * \param opt   Array of options.
 * \param input Input file stream.
 * \param msg   Pointer to a place where to store an error message,
 *              if needed. You need to call \ref w_free on it if
 *              it is non-NULL upon return.
 * \return Whether parsing was successfull. If not successful, most likely
 *         there was some parsing error and the error string will be
 *         non-NULL.
 */
W_EXPORT wbool w_opt_parse_io (const w_opt_t *opt,
                               w_io_t        *input,
                               char         **msg);

/*!
 * Standard error output stream. By default, this points to a \ref
 * w_io_unix_t output stream. It can be reassigned.
 */
W_EXPORT w_io_t *w_stderr;

/*!
 * Standard output stream. By default, this points to a \ref w_io_unix_t
 * output stream. It can be reassigned.
 */
W_EXPORT w_io_t *w_stdout;

/*!
 * Standard input stream. By default, this points to a \ref w_io_unix_t
 * input stream. It can be reassigned.
 */
W_EXPORT w_io_t *w_stdin;

/*\}*/

/*---------------------------------[ simple, piece-based LL parsers ]-----*/

/*!
 * \defgroup wparse Support utilities for building simple parsers
 * \addtogroup wparse
 * \{
 */

/*!
 * Auxiliary structure for parsers. Those do not need to be manually
 * initialized, because the \ref w_parse_run function already takes care of
 * that. Usually those structures are allocated on the stack.
 *
 * Example usage:
 *
 * \code
 * char *error = NULL;
 * w_parse_t parser;
 * w_parse_run (&parser, input, '#', parse_func, NULL, &error);
 * \endcode
 */
struct w_parse_t
{
    unsigned line;    /*!< Current line number.                      */
    unsigned lpos;    /*!< Current position in line (column number). */
    int      look;    /*!< Look-ahead character.                     */
    int      comment; /*!< Comment character.                        */
    w_io_t  *input;   /*!< Input data stream.                        */
    char    *error;   /*!< Error message.                            */
    void    *result;  /*!< Parsing result.                           */
    jmp_buf  jbuf;    /*!< Jump buffer (used when raising errors).   */
};
typedef struct w_parse_t w_parse_t;

typedef void (*w_parse_fun_t) (w_parse_t*, void*);


/*!
 * Parses an input file.
 *
 * \param input     Input file stream.
 * \param comment   Character used as comment delimiter. When the character
 *                  is found, the remainder of the line will be ignored. To
 *                  disable this behavior, pass \c 0.
 * \param parse_fun Function used for parsing.
 * \param context   Context passed as user data to the parsing function.
 * \param msg       If not \c NULL, where to store an error string in case
 *                  an error is generated.
 * \return          Whether parsing was successful.
 */
W_EXPORT void* w_parse_run     (w_parse_t    *p,
                                w_io_t       *input,
                                int           comment,
                                w_parse_fun_t parse_fun,
                                void         *context,
                                char         **msg);

/*!
 * Formats an error string and raises an error.
 *
 * \sa w_parse_ferror(), w_parse_rerror()
 * \param fmt Format string (as passed to \ref w_io_format)
 * \param ... Arguments for the format string.
 */
W_EXPORT void  w_parse_error   (w_parse_t *p, const char *fmt, ...);

/*!
 * Format an error string. The formatted string will be saved in the
 * \ref w_parse_t::error field.
 *
 * \param fmt Format string (as passed to \ref w_io_format).
 * \param ... Arguments for the format string.
 */
W_EXPORT void  w_parse_ferror  (w_parse_t *p, const char *fmt, ...);

/*!
 * Raise a parsing error. Make sure you free intermediate strucutures and
 * data parsing may have created before calling this function, otherwise
 * memory will leak.
 *
 * \sa w_parse_ferror(), w_parse_error()
 */
W_EXPORT void  w_parse_rerror  (w_parse_t *p);

/*!
 * Gets the next character in the input, skipping over comments. If comments
 * are enabled i.e. the \ref w_parse_t::comment field is different than
 * \c 0, then when a comment character is found, the entire line is skipped.
 */
W_EXPORT void  w_parse_getchar (w_parse_t *p);

/*!
 * Skips whitespace in the input. All characters for which \c isspace()
 * returns true will be ignored until the first non-blank or the end of
 * the input stream is found.
 */
W_EXPORT void  w_parse_skip_ws (w_parse_t *p);

/*!
 * Gets a string enclosed in double-quotes from the input. Escape characters
 * in the string are interpreted, same way as the C compiler does. This
 * function never raises errors, but returns \c NULL when there is some
 * error in the input. The caller is responsible for calling \ref w_free()
 * on the returned string.
 */
W_EXPORT char* w_parse_string  (w_parse_t *p);

/*!
 * Gets a C-like identifier. Identifiers are the same as in C: a sequence of
 * non-blank character, being the first one a letter or an underscore, and
 * the rest letters, numbers and underscores. This function never raises
 * errors, but returns \c NULL when there is some error in the input. The
 * caller is responsible for calling \ref w_free() on the returned string.
 */
W_EXPORT char* w_parse_ident   (w_parse_t *p);

/*!
 * Gets a single word from the input. A \e word here is any sequence of
 * non-whitespace characters. This function will never raise errors, but
 * returns \c NULL when the word cannot be read. The caller is responsible
 * for calling \ref w_free() on the returned string.
 */
W_EXPORT char* w_parse_word    (w_parse_t *p);

/*!
 * Parses a floating point value as \c double.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
W_EXPORT wbool w_parse_double  (w_parse_t *p, double *value);

/*!
 * Parses a aigned integer as an <tt>unsigned long</tt> value. Note that
 * prefix \c 0x will cause the number to be parsed as hexadecimal, and
 * prefix \c 0 as octal.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
W_EXPORT wbool w_parse_ulong   (w_parse_t *p, unsigned long *value);

/*!
 * Parses a aigned integer as a \c long value. Note that prefix \c 0x will
 * cause the number to be parsed as hexadecimal, and prefix \c 0 as octal.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
W_EXPORT wbool w_parse_long    (w_parse_t *p, long *value);

/*!
 * Checks the next character in the input, cleaning up if needed. If the
 * character is not matched, the given statement is ran, then an error is
 * generated.
 *
 * \param _p Pointer to a \c w_parse_t.
 * \param _c Character to be matched.
 * \param _statement Cleanup statement.
 */
#define w_parse_match_with_cleanup(_p, _c, _statement)             \
    do {                                                           \
        if ((_c) == (_p)->look) {                                  \
            w_parse_getchar (_p);                                  \
            w_parse_skip_ws (_p);                                  \
        } else {                                                   \
            _statement;                                            \
            w_parse_error ((_p), "Character '$c' expected", (_c)); \
        }                                                          \
    } while (0)

/*!
 * Checks the next character in the input. If the character is not matched,
 * then an error is generated.
 *
 * \param _p Pointer to a \c w_parse_t.
 * \param _c Character to be matched.
 */
#define w_parse_match(_p, _c) \
        w_parse_match_with_cleanup((_p), (_c), (void)0)


/*\}*/

/*--------------------------------------------------[ data buffers ]------*/

/*!
 * \defgroup buffers Generic data buffers
 * \addtogroup buffers
 * \{
 *
 * Buffers provide a variable-length area of memory in which data may be
 * held and manipulated. Contained data is not interpreted, and length of
 * it is tracked, so it is even possible to add null bytes to a buffer.
 * Allocating a buffer is done in the stack, so it is very fast, the macro
 * \ref W_BUF is provided to intialize them; once that is done all buffer
 * functions can be used, and once working with tha buffer has finished,
 * its contents must be deallocated using \ref w_buf_free:
 *
 * \code
 * w_buf_t b = W_BUF;
 *
 * w_buf_append_str (&b, "Too much work ");
 * w_buf_append_str (&b, "and no joy makes");
 * w_buf_append_str (&b, " Jack a dull boy");
 *
 * printf ("%s\n", w_buf_str (&b));
 *
 * w_buf_free (&b);
 * \endcode
 *
 * In the above example, a buffer is created, some strings appended to it
 * (there are convenience functions to use them as string buffers) and
 * finally the resulting string is printed and the buffer-held resources
 * freed.
 */

typedef struct w_buf_t w_buf_t;

/*!
 * \brief A variable-length buffer for arbitrary data.
 * Can hold any kind of data, including null characters, as data length is
 * tracked separately. To initialize a buffer, do something like this:
 * \code
 *   w_buf_t mybuffer = W_BUFFER;
 * \endcode
 */
struct w_buf_t
{
    char  *buf; /*!< Actual data */
    size_t len; /*!< Data length */
    size_t bsz; /*!< Buffer size */
};


/*!
 * Initializer for \ref w_buf_t
 */
#define W_BUF { NULL, 0, 0 }


/*!
 * Get the length of a buffer
 */
#define w_buf_length(_b) ((_b)->len)

/*!
 * Reset a buffer to its initial (empty) state
 */
#define w_buf_reset w_buf_free

/*!
 * Adjust the length of a buffer keeping contents.
 * This is mostly useful for trimming contents, when shrinking the buffer.
 * When a buffer grows, random data is liklely to appear at the end.
 * \param buf A \ref w_buf_t buffer
 * \param len New length of the buffer
 */
W_EXPORT void w_buf_length_set (w_buf_t *buf,
                                size_t  len);

/*!
 * Set the contents of a buffer to a C string.
 * \param buf A \ref w_buf_t buffer
 * \param str String to set the buffer to
 */
W_EXPORT void w_buf_set_str (w_buf_t    *buf,
                             const char *str);

/*!
 * Appends the contents of a memory block to a buffer.
 * \param buf A \ref w_buf_t buffer
 * \param ptr Pointer to the block of memory
 * \param len Length of the memory block
 */
W_EXPORT void w_buf_append_mem (w_buf_t    *buf,
                                const void *ptr,
                                size_t     len);

/*!
 * Appends a character to a buffer
 * \param buf A \ref w_buf_t buffer
 * \param chr A character
 */
W_EXPORT void w_buf_append_char (w_buf_t *buf,
                                 int      chr);

/*!
 * Appends a string to a buffer.
 * \param buf A \ref w_buf_t buffer
 * \param str A string
 */
W_EXPORT void w_buf_append_str (w_buf_t    *buf,
                                const char *str);

/*!
 * Appends a buffer to another buffer
 * \param buf A \ref w_buf_t buffer
 * \param src Another \ref w_buf_t which contains the data to be appended
 */
W_EXPORT void w_buf_append_buf (w_buf_t       *buf,
                                const w_buf_t *src);

/*!
 * Appends formatted text into a buffer. Available formatting options are
 * the same as for \ref w_io_format.
 * \param buf A \ref w_buf_t buffer.
 * \param fmt Format string.
 */
W_EXPORT void w_buf_format (w_buf_t    *buf,
                            const char *fmt,
                            ...);

/*!
 * Frees the contents of a buffer.
 * \param buf A \ref w_buf_t buffer
 */
W_EXPORT void w_buf_free (w_buf_t *buf);

/*!
 * Get the buffer as a C string.
 * Note that if the buffer contains embedded null characters, functions like
 * \c strlen() will not report the full length of the buffer.
 * \param buf A \ref w_buf_t buffer
 */
W_EXPORT char* w_buf_str (w_buf_t *buf);


/*\}*/

/*--------------------------------------------------[ input/output ]------*/

/*!
 * \defgroup wio Input/output
 * \addtogroup wio
 * \{
 */

enum w_io_flag
{
    __W_IO_ZERO =  0,
    W_IO_ERR    = -1, /*!< An I/O error occured. */
    W_IO_EOF    = -2, /*!< End of file reached.  */
};


/*!
 * Input/output descriptor.
 */
W_OBJ_DEF (w_io_t)
{
    w_obj_t  parent;
    int      backch;
    wbool   (*close) (w_io_t *io);
    ssize_t (*write) (w_io_t *io, const void *buf, size_t len);
    ssize_t (*read ) (w_io_t *io, void       *buf, size_t len);
    wbool   (*flush) (w_io_t *io);
};


/*!
 */
W_EXPORT void w_io_init (w_io_t *io);


/*!
 * Closes an input/output descriptor. If the \c close callback of the
 * descriptor is \c NULL, then no action is performed.
 *
 * \param io An input/output descriptor.
 */
W_EXPORT wbool w_io_close (w_io_t *io);

/*!
 * Writes data to an input/output descriptor. If the descriptor has no
 * \c write callback, then \c -1 is returned and \c errno is set to
 * \c EBADF.
 *
 * \param io  An input/output descriptor.
 * \param buf Pointer to the data to be written.
 * \param len Number of bytes to be written.
 */
W_EXPORT ssize_t w_io_write (w_io_t     *io,
                             const void *buf,
                             size_t      len);

/*!
 * Reads data from an input/output descriptor. If the descriptor has no
 * \c read callback, then \c -1 is returned and \c errno is set to
 * \c EBADF.
 */
W_EXPORT ssize_t w_io_read (w_io_t *io,
                            void   *buf,
                            size_t len);

/*!
 * Reads data, until a given character or end of file is reached.
 * \param io        An input/output object.
 * \param data      Buffer where the read data is stored.
 * \param overflow  Buffer where temporary extra data is stored.
 * \param stopchar  Character used to determine when to stop reading.
 * \param readbytes Maximum read chunk size. If zero, a default size
 *                  is used.
 * \return Number of bytes read. Negative value on error.
 */
W_EXPORT ssize_t w_io_read_until (w_io_t  *io,
                                  w_buf_t *data,
                                  w_buf_t *overflow,
                                  int      stopchar,
                                  unsigned maxbytes);

/*!
 * Reads a line from an input stream.
 * This is a convenience macro that calls \ref w_io_read_until passing \c
 * '\n' as stop character.
 */
#define w_io_read_line(_io, _data, _overflow, _maxbytes) \
       (w_io_read_until ((_io), (_data), (_overflow), '\n', (_maxbytes)))

/*!
 * Formats text and writes it to an I/O object. Implements naïve (but
 * effective) formatting, providing the most common options of the
 * \c printf() family of functions. Because behavior is different from
 * those, format specifications use <tt>$</tt> as prefix. The recognized
 * specifiers are:
 *
 *  - \b c : <tt>char</tt>, a single character.
 *  - \b l : <tt>long int</tt> number.
 *  - \b L : <tt>unsigned long int</tt> number.
 *  - \b i : <tt>int</tt> number.
 *  - \b I : <tt>unsigned int</tt> number.
 *  - \b X : <tt>unsigned long int</tt> number, formatted in hexadecimal.
 *  - \b O : <tt>unsigned long int</tt> number, formatted in octal.
 *  - \b p : <tt>void*</tt> pointer, formatted in hexadecimal.
 *  - \b f : <tt>float</tt> number.
 *  - \b F : <tt>double</tt> number.
 *  - \b s : <tt>const char*</tt>, a zero-terminated string.
 *  - \b B : <tt>w_buf_t*</tt>, a buffer.
 *  - \b S : <tt>size_t</tt> and <tt>const char*</tt>, prints a given
 *    amount of characters from a string.
 *  - \b e : Last value of <tt>errno</tt>, as an integer.
 *  - \b E : Last value of <tt>errno</tt>, as a string.
 *
 * \param io  An input/output descriptor.
 * \param fmt Format string.
 */
W_EXPORT ssize_t w_io_format (w_io_t     *io,
                              const char *fmt,
                              ...);

/*!
 * Formats text and writes it to an I/O object. This version accepts
 * a standard variable argument list. For the available formatting options,
 * read the documentation for \ref w_io_format().
 * \param io   An input/output descriptor.
 * \param fmt  Format string.
 * \param args Argument list.
 */
W_EXPORT ssize_t w_io_formatv (w_io_t     *io,
                               const char *fmt,
                               va_list     args);


W_EXPORT ssize_t w_io_format_long      (w_io_t *io, long          value);
W_EXPORT ssize_t w_io_format_ulong     (w_io_t *io, unsigned long value);
W_EXPORT ssize_t w_io_format_double    (w_io_t *io, double        value);
W_EXPORT ssize_t w_io_format_ulong_hex (w_io_t *io, unsigned long value);
W_EXPORT ssize_t w_io_format_ulong_oct (w_io_t *io, unsigned long value);

/*!
 * Reads formatted input from an I/O object.
 * \param io  An input/output descriptor.
 * \param fmt Format string.
 */
W_EXPORT ssize_t w_io_fscan (w_io_t     *io,
                             const char *fmt,
                             ...);

/*!
 * Reads formatted input from an I/O object. This version accepts a standard
 * variable argument list.
 * \param io  An input/output descriptor.
 * \param fmt Format string.
 * \param args Argument list.
 */
W_EXPORT ssize_t w_io_fscanv (w_io_t     *io,
                              const char *fmt,
                              va_list     args);


W_EXPORT wbool w_io_fscan_float     (w_io_t *io, float         *result);
W_EXPORT wbool w_io_fscan_double    (w_io_t *io, double        *result);
W_EXPORT wbool w_io_fscan_int       (w_io_t *io, int           *result);
W_EXPORT wbool w_io_fscan_uint      (w_io_t *io, unsigned int  *result);
W_EXPORT wbool w_io_fscan_long      (w_io_t *io, long          *result);
W_EXPORT wbool w_io_fscan_ulong     (w_io_t *io, unsigned long *result);
W_EXPORT wbool w_io_fscan_ulong_hex (w_io_t *io, unsigned long *result);
W_EXPORT wbool w_io_fscan_ulong_oct (w_io_t *io, unsigned long *result);


/*!
 * Reads a single character from an I/O object.
 * \param io An input/output descriptor.
 * \return   The read character, or either \ref W_IO_EOF if the end of file
 *           was reached, or \ref W_IO_ERR if there was some error.
 */
W_EXPORT int w_io_getchar (w_io_t *io);

/*!
 * Writes a single character to an I/O object.
 * \param io An input/output descriptor.
 * \param ch Character.
 * \return   Whether there was some error.
 */
W_EXPORT wbool w_io_putchar (w_io_t *io,
                             int     ch);

/*!
 */
W_EXPORT void w_io_putback (w_io_t *io,
                            char    ch);

/*!
 * Flushes pending buffered data.
 * \param io An input/output descriptor.
 * \return   Whether there was some error.
 */
W_EXPORT wbool w_io_flush (w_io_t *io);

/*!
 * Input/output object on Unix file descriptors.
 */
W_OBJ (w_io_unix_t)
{
    w_io_t parent;
    int    fd;
};

/*!
 * Obtain the Unix file descriptor used in an I/O stream.
 * \param _io Pointer to a \ref w_io_t
 */
#define W_IO_UNIX_FD(_io) \
    (((w_io_unix_t*) (_io))->fd)

/*!
 * Create an I/O object to be used with an Unix file descriptor.
 * This is a convenience function that calls \c open() and then uses
 * \ref w_io_unix_open_fd.
 * \param path File path.
 * \param mode Open mode flags (\c O_CREAT, \c O_RDWR...)
 * \param perm Permissions.
 */
W_EXPORT w_io_t* w_io_unix_open (const char *path,
                                 int         mode,
                                 unsigned    perm);

/*!
 * Create an I/O object to be used with an Unix file descriptor.
 * This function allows initialization from an existing, valid file
 * descriptor.
 * \param fd Unix file descriptor.
 * \sa W_IO_UNIX_FD
 */
W_EXPORT w_io_t* w_io_unix_open_fd (int fd);

/*!
 * Initialize an I/O object to be used with an Unix file descriptor.
 * This calls \c open() and then uses \ref w_io_unix_init_fd.
 * This function is not intended to be used directly, but it is provided as
 * a convenience for code extending \ref w_io_unix_t
 */
W_EXPORT wbool w_io_unix_init (w_io_unix_t *io,
                               const char  *path,
                               int          mode,
                               unsigned     perm);

/*!
 * Initialize an I/O object to be used with an Unix file descriptor.
 * This function is not inteded to be used directly, but it is provided as
 * a convenience for code extending \ref w_io_unix_t.
 * \sa W_IO_UNIX_FD
 */
W_EXPORT void w_io_unix_init_fd (w_io_unix_t *io, int fd);


/*!
 * Socket kinds.
 * \see w_io_socket_open
 */
enum w_io_socket_kind_t
{
    W_IO_SOCKET_UNIX, /*!< Unix named socket.    */
    W_IO_SOCKET_TCP4, /*!< IPv4 TCP socket.      */
};
typedef enum w_io_socket_kind_t w_io_socket_kind_t;

/*!
 * Modes available for server-side sockets.
 * \see w_io_socket_serve
 */
enum w_io_socket_serve_mode_t
{
    W_IO_SOCKET_SINGLE, /*!< Serve one client at a time.                   */
    W_IO_SOCKET_THREAD, /*!< Each client is serviced using a new thread.   */
    W_IO_SOCKET_FORK,   /*!< Each client is serviced by forking a process. */
};
typedef enum w_io_socket_serve_mode_t w_io_socket_serve_mode_t;


#define W_IO_SOCKET_SA_LEN 1024

/*!
 * Performs input/output on sockets.
 */
W_OBJ (w_io_socket_t)
{
    w_io_unix_t        parent;
    w_io_socket_kind_t kind;
    size_t             slen;
    wbool              bound;
    char               sa[W_IO_SOCKET_SA_LEN];
};

/*!
 * Obtain the Unix file descriptor associated to a socket.
 */
#define W_IO_SOCKET_FD W_IO_UNIX_FD

/*!
 * Obtain the kind of a socket.
 */
#define W_IO_SOCKET_KIND(_io) \
    ((_io)->kind)

/*!
 * Create a new socket. Sockets can be used both for clients and servers,
 * as this function only performs basic initialization. Calling \ref
 * w_io_socket_serve will start serving request, and for client use \ref
 * w_io_socket_connect should be called. Parameters other than the socket
 * kind are dependant on the particular kind of socket being created. For
 * the moment the following are supported:
 *
 * - \ref W_IO_SOCKET_UNIX will create an Unix domain socket. A path in
 *   the file system must be given, at which the socket will be created.
 * - \ref W_IO_SOCKET_TCP4 will create a TCP socket using IPv4, arguments
 *   are the IP address as a string for binding (in the case of calling
 *   \ref w_io_socket_serve afterwards) or connecting to (when using \ref
 *   w_io_socket_connect). The second argument is the port number.
 *
 * \param kind Socket kind.
 * \return When socket creation fails, \c NULL is returned and \c errno
 *         is set accordingly.
 * \sa w_io_socket_init.
 */
W_EXPORT w_io_t* w_io_socket_open (w_io_socket_kind_t kind, ...);

/*!
 * Perform basic socket initialization. In general applications should use
 * \ref w_io_socket_open instead of this function, but it is provided to be
 * used from code extending \ref w_io_socket_t.
 * \param io   An input/output object.
 * \param kind Socket kind.
 * \return Whether initialization was successful.
 * \sa w_io_socket_open
 */
W_EXPORT wbool w_io_socket_init (w_io_socket_t *io,
                                 w_io_socket_kind_t kind, ...);

/*!
 * Serves requests using a socket. This function will start a loop accepting
 * connections, and for each connection an open socket will be passed to the
 * given handler function. The way in which each request is served can be
 * specified:
 *
 * - \ref W_IO_SOCKET_SINGLE: Each request is served by calling directly and
 *   waiting for it to finish. This makes impossible to serve more than one
 *   request at a time.
 * - \ref W_IO_SOCKET_THREAD: Each request is served in a new thread. The
 *   handler is invoked in that thread.
 * - \ref W_IO_SOCKET_FORK: A new process is forked for each request. The
 *   handler is invoked in the child process.
 *
 * \param io      An input/output object.
 * \param mode    How to serve each request.
 * \param handler Callback invoked to serve requests.
 */
W_EXPORT wbool w_io_socket_serve (w_io_socket_t *io,
                                  w_io_socket_serve_mode_t mode,
                                  wbool (*handler) (w_io_socket_t*));

/*!
 * Connect a socket to a server. This makes a connection to the host
 * specified when creating the socket with \ref w_io_socket_open, and
 * puts it in client mode. Once the socket is successfully connected,
 * read and write operations can be performed in the socket.
 * \return Whether the connection was setup successfully.
 */
W_EXPORT wbool w_io_socket_connect (w_io_socket_t *io);

/*!
 * Perform a half-close on the write direction. This closes the socket,
 * but only in writing one direction, so other endpoint will think that
 * the end of the stream was reached (thus the operation is conceptually
 * equivalent to sending and “end of file marker”). Read operations can
 * still be performing in a socket which was half-closed using this
 * function. Note that for completely closing the socket, \ref w_io_close
 * should be used instead.
 * \return Whether performing the half-close was successful.
 */
W_EXPORT wbool w_io_socket_send_eof (w_io_socket_t *io);

/*!
 * Obtain the path in the filesystem for an Unix socket.
 */
W_EXPORT const char* w_io_socket_unix_path (w_io_socket_t *io);


#ifdef W_CONF_STDIO
/*!
 * Perform input/output in standard C file descriptors.
 */
W_OBJ (w_io_stdio_t)
{
    w_io_t parent;
    FILE  *fp;
};

/*!
 * Initialize an I/O object to be used with a C standard file descriptor.
 * \param fp Standard C file descriptor.
 * \sa W_IO_STDIO, W_IO_STDIO_FILEP
 */
W_EXPORT w_io_t* w_io_stdio_open (FILE *fp);

/*!
 * Initialize an I/O object to be used with a C standard file descriptor.
 * This function is not meant to be used directly, but is provided as
 * a convenience for other code extending \ref w_io_stdio_t.
 */
W_EXPORT void w_io_stdio_init (w_io_stdio_t *io, FILE *fp);

#endif /* W_CONF_STDIO */


/*!
 * Perform input/output on memory buffers.
 */
W_OBJ (w_io_buf_t)
{
    w_io_t  parent;
    w_buf_t buf;
    size_t  pos;
    wbool   own;
};


/*!
 * Create an I/O object to be used with a buffer.
 * \param buf Pointer to a w_buf_t. Passing NULL will initialize a new
 *            \ref w_buf_t internally which can be retrieved with
 *            \ref W_IO_BUF_BUF. If you pass a non-NULL buffer, then
 *            you will be responsible to call \ref w_buf_free on it.
 */
W_EXPORT w_io_t* w_io_buf_open (w_buf_t *buf);

/*!
 * Initialize an I/O object in the stack to be used with a buffer.
 * This function is not meant to be used directly, but is provided as
 * a convenience for other code extending \ref w_io_buf_t, or wanting to
 * quickly allocate a \ref w_io_buf_t in the stack.
 */
W_EXPORT void w_io_buf_init (w_io_buf_t *io, w_buf_t *buf);

/*!
 * Obtain a pointer to the buffer being used by a \ref w_io_buf_t.
 */
#define W_IO_BUF_BUF(_p) \
    (&((w_io_buf_t*) (_p))->buf)

/*!
 * Obtain a string representation of a \ref w_io_buf_t I/O object.
 */
#define W_IO_BUF_STR(_p) \
    (w_io_buf_str (W_IO_BUF_BUF (_p)))

/*\}*/

/*---------------------------------------------------[ config files ]-----*/

/*!
 * \defgroup wcfg Structured configuration files
 * \addtogroup wcfg
 * \{
 */

typedef w_dict_t w_cfg_t;

/*!
 * Flags and constant values for node types.
 */
enum w_cfg_type_t
{
	/* Marks en of parameter list for set/get. */
	W_CFG_END       = 0,    /*!< Marker for ending parameter lists. */

	/* Types for element nodes. */
	W_CFG_NONE      = 0,    /*!< Invalid node.                      */
	W_CFG_STRING    = 0x01, /*!< Node containing a string.          */
	W_CFG_NUMBER    = 0x02, /*!< Node containing a number.          */
	W_CFG_NODE      = 0x04, /*!< Node containing a subnode.         */

	/* Mask for the type. */
	W_CFG_TYPE_MASK =  W_CFG_STRING | W_CFG_NUMBER | W_CFG_NODE,

	/* Mask for the flags use in get/set. */
	W_CFG_FLAG_MASK = ~W_CFG_TYPE_MASK,
};

typedef enum w_cfg_type_t w_cfg_type_t;

/*!
 * Create a new configuration object.
 */
W_EXPORT w_cfg_t* w_cfg_new (void);

/*!
 * Checks whether a key exists in a configuration object.
 */
W_EXPORT wbool w_cfg_has (const w_cfg_t *cf, const char *key);

/*!
 * Deletes a key from a configuration object.
 */
W_EXPORT wbool w_cfg_del (w_cfg_t *cf, const char *key);

/*!
 * Sets a number of items in a configuration object. Prefix each item with
 * the type, and then pass the key and the contents, and finally use
 * \ref W_CFG_END to terminate the list. For example:
 *
 * \code
 * w_cfg_t *cf = w_cfg_new ();
 * w_cfg_set (cf, W_CFG_STRING, "string-key", "string-value",
 *                W_CFG_NUMBER, "number-key", 12345,
 *                W_CFG_END);
 * \endcode
 */
W_EXPORT wbool w_cfg_set (w_cfg_t *cf, ...);

/*!
 * Gets a number of items from a configuration object. The variable argument
 * list works similarly to \ref w_cfg_set, but passing pointers to suitable
 * variables where to store values. Example:
 *
 * \code
 * char  *sval;
 * double dval;
 * w_cfg_get (cf, W_CFG_STRING, "string-key", &sval,
 *                W_CFG_NUMBER, "number-key", &dval,
 *                W_CFG_END);
 * \endcode
 */
W_EXPORT wbool w_cfg_get (const w_cfg_t *cf, ...);

/*!
 * Obtain the type of a configuration object node.
 */
W_EXPORT w_cfg_type_t w_cfg_type (const w_cfg_t *cf, const char *key);

/*!
 * Dump configuration to a stream.
 * \param cf     Configuration object.
 * \param output Output stream where to write.
 * \return       Whether writing was successful.
 */
W_EXPORT wbool w_cfg_dump (const w_cfg_t *cf, w_io_t *output);

/*!
 * Load configuration from a stream.
 * \param input  Input stream
 * \param msg    Pointer to where to store an error message, if there is one.
 * \return       Valid configuration object or \c NULL if there is some error.
 */
W_EXPORT w_cfg_t* w_cfg_load (w_io_t *input, char **msg);

/*!
 * Dump configuration to a file.
 * \param cf   Configuration object.
 * \param path Path to a file.
 * \return     Whether writing was successful.
 */
W_EXPORT wbool w_cfg_dump_file (const w_cfg_t *cf, const char *path);

/*!
 * Load configuration from a file.
 * \param path Path to a file.
 * \param msg  Pointer to where to store an error message, if there is one.
 * \return     Valid configuration object or \c NULL if there is some error.
 */
W_EXPORT w_cfg_t* w_cfg_load_file (const char *path, char **msg);

#define w_cfg_set_string(cf, key, val) \
	w_cfg_set ((cf), W_CFG_STRING, key, val, W_CFG_END)

#define w_cfg_set_number(cf, key, val) \
	w_cfg_set ((cf), W_CFG_NUMBER, key, val, W_CFG_END)

#define w_cfg_set_node(cf, key, val) \
	w_cfg_set ((cf), W_CFG_NODE, key, val, W_CFG_END)

#define _W_G(func_name, conf_type, c_type)              \
	static inline c_type w_cfg_get_ ## func_name         \
	(w_cfg_t *cf, const char *key, c_type defval)         \
	{ c_type value;                                        \
		if (w_cfg_get (cf, conf_type, key, &value, W_CFG_END)) \
			return value; else return defval;	}

_W_G( number, W_CFG_NUMBER, double      )
_W_G( string, W_CFG_STRING, const char* )
_W_G( node,   W_CFG_NODE,   w_cfg_t*    )

#undef _W_G

/*! Check whether a node is invalid. */
#define w_cfg_isnone(cf, key)    (W_CFG_NONE   == w_cfg_type (cf, key))

/*! Check whether a node contains a subnode. */
#define w_cfg_isnode(cf, key)    (W_CFG_NODE   == w_cfg_type (cf, key))

/*! Check whether a node contains a number. */
#define w_cfg_isnumber(cf, key)  (W_CFG_NUMBER == w_cfg_type (cf, key))

/*! Check whether a node contains a string. */
#define w_cfg_isstring(cf, key)  (W_CFG_STRING == w_cfg_type (cf, key))

/*\}*/

/*-----------------------------------------------------------[ ttys ]-----*/

/*!
 * \defgroup wtty Terminal handling
 * \addtogroup wtty
 * \{
 */

/*!
 * Obtains the dimensions (width & height) of the controlling terminal.
 *
 * \param cols Reference to a variable for the number of columns.
 * \param rows Reference to a variable for the number of rows.
 *
 * \return Wether the terminal size was guessed properly.
 */
W_EXPORT wbool w_tty_size (unsigned *cols, unsigned *rows);

/*!
 * Obtains the width of a row of the controlling terminal.
 *
 * \return Terminal width.
 */
W_EXPORT unsigned w_tty_cols (void);

/*!
 * Obtains the height of the the controlling terminal.
 *
 * \return Terminal height.
 */
W_EXPORT unsigned w_tty_rows (void);


typedef void (*w_tty_notify_fun_t) (unsigned, unsigned, void*);

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
W_EXPORT wbool w_tty_size_notify (w_tty_notify_fun_t function, void *context);

/*\}*/

#endif /* !__wheel_h__ */

