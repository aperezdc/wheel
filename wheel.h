/*!
 * \file wheel.h
 * \brief Main include file for libwheel.
 *
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
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
#include <stdio.h>


/*------------------------------------------------[ memory handling ]-----*/

/*!
 * \defgroup wmem Memory handling
 * \addtogroup wmem
 * \{
 */

W_EXPORT void* w_malloc(size_t sz);
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


/*---------------------------------------------------[ errors/debug ]-----*/

/*!
 * \defgroup debug Debugging support
 * \addtogroup debug
 * \{
 */

/*!
 * Prints a message to stderr and aborts execution.
 */
W_EXPORT void  w_die(const char *fmt, ...);

/*!
 * Prints a message to stderr and aborts execution.
 */
W_EXPORT void w_vdie(const char *fmt, va_list al);


#ifdef _DPRINTF
# define w_dprintf(_x) __w_dprintf _x
W_EXPORT void __w_dprintf(const char *fmt, ...);
#else
# define w_dprintf(_x) ((void)0)
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
W_EXPORT unsigned w_hashstrn(const char *str, size_t len);

/*!
 * Hashes a string.
 * \param str String to get the hash of.
 * \return Hash value.
 */
W_EXPORT unsigned w_hashstr (const char *str);

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
w_strndup(const char *str, size_t len)
{
    char *r;
    if (str == NULL)
        return NULL;
    r = (char*) memcpy(w_alloc(char, len + 1), str, len);
    r[len] = '\0';
    return r;
}


static inline char*
w_strdup(const char *str)
{
	if (str == NULL)
	    return NULL;
	return w_strndup(str, strlen(str));
}



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

typedef struct w_dict_t w_dict_t;

/*!
 * Create a new dictionary.
 */
W_EXPORT w_dict_t* w_dict_new(void);

/*!
 * Free a dictionary.
 */
W_EXPORT void w_dict_free(w_dict_t *d);

/*!
 * Clears the contents of a dictionary.
 */
W_EXPORT void w_dict_clear(w_dict_t *d);

/*!
 * Get the number of items in a dictionary.
 */
W_EXPORT unsigned w_dict_count(const w_dict_t *d);

W_EXPORT void* w_dict_getn(const w_dict_t *d, const char *key, size_t keylen);
W_EXPORT void  w_dict_setn(w_dict_t *d, const char *key, size_t keylen, void *data);
W_EXPORT void  w_dict_deln(w_dict_t *d, const char *key, size_t keylen);

/*!
 * Delete an item from a dictionary given its key.
 */
W_EXPORT void  w_dict_del(w_dict_t *d, const char *key);

/*!
 * Set an item in a dictionary.
 */
W_EXPORT void  w_dict_set(w_dict_t *d, const char *key, void *data);

/*!
 * Get an item from a dictionary.
 */
W_EXPORT void* w_dict_get(const w_dict_t *d, const char *key);

/*!
 * Update a dictionary with the contents of another. For each key present in
 * the source dictionary, copy it and its value to the destination one, if
 * the key already existed in the destination, the value gets overwritten.
 * \param d Destination dictionary.
 * \param o Source dictionary.
 */
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

#define w_dict_foreach(_d, _i) \
    for ((_i) = w_dict_first (_d); (_i) != NULL; (_i) = w_dict_next ((_d), (_i)))

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
 * \param opt Array of options.
 * \param out Stream where write the message.
 * \param progname Program name (usually <tt>argv[0]</tt>).
 */
W_EXPORT void w_opt_help(const w_opt_t opt[], FILE *out, const char *progname);

/*!
 * Parses an array of command line arguments.
 * \param options  Array of options.
 * \param file_cb  Callback invoked for each non-option argument found (most
 *                 likely files, thus the name).
 * \param userdata User data, this is passed to the \c file_cb callback.
 * \param argc     Number of command line arguments.
 * \param argv     Array of command line arguments.
 * \return Number of consmed arguments.
 */
W_EXPORT unsigned w_opt_parse(const w_opt_t  *options,
                              w_action_fun_t file_cb,
                              void          *userdata,
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
W_EXPORT wbool w_opt_parse_file(const w_opt_t *opt,
                                FILE          *input,
                                char         **msg);


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
    FILE    *input;   /*!< Input file stream.                        */
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
                                FILE         *input,
                                int           comment,
                                w_parse_fun_t parse_fun,
                                void         *context,
                                char         **msg);

/*!
 * Formats an error string and raises an error.
 *
 * \sa w_parse_ferror(), w_parse_rerror()
 * \param fmt Format string (printf-like)
 * \param ... Arguments for the format string.
 */
W_EXPORT void  w_parse_error   (w_parse_t *p, const char *fmt, ...);

/*!
 * Format an error string. The formatted string will be saved in the
 * \ref w_parse_t::error field. A typical formats format is the following
 * one, including the line and column numbers:
 *
 * \code
 *    w_parse_ferror (p, "%u:%u: Your error message", p->line, p->lpos);
 * \endcode
 *
 * \param fmt Format string (printf-like).
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
#define w_parse_match_with_cleanup(_p, _c, _statement)              \
    do {                                                            \
        if ((_c) == (_p)->look) {                                   \
            w_parse_getchar (_p);                                   \
            w_parse_skip_ws (_p);                                   \
        } else {                                                    \
            _statement;                                             \
            w_parse_error ((_p), "%u:%u: character '%c' expected,", \
                           (_p)->line, (_p)->lpos, (_c));           \
        }                                                           \
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
#define w_buf_length(_b)  (w_assert (_b), (_b)->len)

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
W_EXPORT const char* w_buf_str (const w_buf_t *buf);


/*\}*/

/*-----------------------------------------------[ text formatting ]------*/

/*!
 * \defgroup formatting Text formatting
 * \addtogroup formatting
 * \{
 */

/*!
 * Formats text into a buffer.
 * \param buf A \ref w_buf_t buffer
 * \param fmt Format string
 */
void w_fmt_buf  (w_buf_t *buf, const char *fmt, ...);

/*!
 * Formats text into a buffer.
 * \param buf A \ref w_buf_t buffer
 * \param fmt Format string
 * \param arg Argument list
 */
void w_fmt_bufv (w_buf_t *buf, const char *fmt, va_list arg);

/*\}*/

/*--------------------------------------------------[ input/output ]------*/

/*!
 * \defgroup wio Input/output
 * \addtogroup wio
 * \{
 */

typedef struct w_io_t w_io_t;

/*!
 * Input/output descriptor.
 */
struct w_io_t
{
    wbool   (*close) (void *udata);
    ssize_t (*write) (void *udata, const void *buf, size_t len);
    ssize_t (*read ) (void *udata, void       *buf, size_t len);
};

/*!
 * Declare a I/O object with extra spacing for holding user data.
 * This macro is intended to be used by concrete implementations, so they
 * can define a macro to declare variables that contain the fields of the
 * \ref w_io_t structure, plus additional space for their private data.
 *
 * For example, the following declares a new macro which will create \ref
 * w_io_t structures with additional space for holding a <tt>struct
 * my_io_data</tt>:
 *
 * \code
 * struct my_io_data {
 *   char buffer[512];
 *   int  flags;
 * };
 *
 * #define MY_IO(_varname) \
 *         W_IO_MAKE (_varname, struct my_io_data)
 * \endcode
 *
 * \param _varname Name of the variable to be allocated.
 * \param _udata   Type name. Additional space will be reserved to
 *                 accomodate values of this type.
 */
#define W_IO_MAKE(_varname, _udata) \
    char w_io__ ## _varname ## __raw__ [sizeof (w_io_t) + sizeof (_udata)]; \
    w_io_t * _varname = (w_io_t*) (w_io__ ## _varname ## __raw__)

/*!
 * Given an I/O object, get a pointer to the user data area.
 * The user data area is the additional space reserved for data by the \ref
 * W_IO_MAKE macro. For example, if you have space reserved for a <tt>struct
 * my_io_data</tt> then you can access it like this:
 *
 * \code
 * W_IO_MAKE (my_io, struct my_io_data);
 * struct my_io_data *data = W_IO_UDATA (my_io, struct my_io_data);
 * \endcode
 *
 * \param _ioptr Pointer to a \ref w_io_t.
 * \param _udata Type name. The pointer will be cast to this type.
 */
#define W_IO_UDATA(_ioptr, _udata) \
    ((_udata*) (((char*)(_ioptr)) + sizeof (w_io_t)))

/*!
 * Closes an input/output descriptor. If the \c close callback of the
 * descriptor is \c NULL, then no action is performed.
 *
 * \param io An input/output descriptor.
 */
W_EXPORT wbool w_io_close (const w_io_t *io);

/*!
 * Writes data to an input/output descriptor. If the descriptor has no
 * \c write callback, then \c -1 is returned and \c errno is set to
 * \c EBADF.
 *
 * \param io  An input/output descriptor.
 * \param buf Pointer to the data to be written.
 * \param len Number of bytes to be written.
 */
W_EXPORT ssize_t w_io_write (const w_io_t *io,
                             const void   *buf,
                             size_t        len);

/*!
 * Reads data from an input/output descriptor. If the descriptor has no
 * \c read callback, then \c -1 is returned and \c errno is set to
 * \c EBADF.
 */
W_EXPORT ssize_t w_io_read (const w_io_t *io,
                            void         *buf,
                            size_t        len);

/*!
 * Formats text and writes it to an I/O object.
 * \param io  An input/output descriptor.
 * \param fmt Format string.
 */
W_EXPORT ssize_t w_io_format (const w_io_t *io,
                              const char   *fmt,
                              ...);

/*!
 * Formats text and writes it to an I/O object. This version accepts
 * a standard variable argument list.
 * \param io   An input/output descriptor.
 * \param fmt  Format string.
 * \param args Argument list.
 */
W_EXPORT ssize_t w_io_formatv (const w_io_t *io,
                               const char   *fmt,
                               va_list       args);


/*!
 * Declare an I/O object for use with an Unix file descriptor.
 * \param _v Variable name.
 * \sa W_IO_UNIX_FD, w_io_unix_open
 */
#define W_IO_UNIX(_v) \
        W_IO_MAKE (_v, int)

/*!
 * Get the Unix file descriptor associated to an I/O object.
 * \warning Using this macro with a \ref w_io_t which was not declared
 *          with \ref W_IO_UNIX and/or not initialized with \ref
 *          w_io_unix_open is undefined.
 * \param _io Pointer to a \ref w_io_t.
 * \sa W_IO_UNIX, w_io_unix_open
 */
#define W_IO_UNIX_FD(_io) \
      (*W_IO_UDATA (_io, int))

/*!
 * Initialize an I/O object to be used with an Unix file descriptor.
 * \param io A pointer to a \ref w_io_t previously allocated with
 *           \ref W_IO_UNIX.
 * \param fd Unix file descriptor.
 * \sa W_IO_UNIX, W_IO_UNIX_FD
 */
W_EXPORT void w_io_unix_open (w_io_t *io,
                              int    fd);

/*!
 * Declare an I/O object for use with C standard file descriptors.
 * \param _v Variable name.
 * \sa W_IO_STDIO, W_IO_STDIO_FILEP
 */
#define W_IO_STDIO(_v) \
        W_IO_MAKE (_v, FILE*)

/*!
 * Get the C standard file descriptor associated to an I/O object.
 * \warning Using this macro with a \ref w_io_t which was not declared
 *          with \ref W_IO_STDIO and/or not initialized with \ref
 *          w_io_stdio_open is undefined.
 * \param _io Pointer to a \ref w_io_t.
 * \sa W_IO_STDIO, w_io_stdio_open
 */
#define W_IO_STDIO_FILEP(_io) \
      (*W_IO_UDATA (_io, FILE*))

/*!
 * Initialize an I/O object to be used with an C standard file descriptor.
 * \param io A pointer to a \ref w_io_t previously allocated with
 *           \ref W_IO_STDIO.
 * \param fd Unix file descriptor.
 * \sa W_IO_STDIO, W_IO_STDIO_FILEP
 */
W_EXPORT void w_io_stdio_open (w_io_t *io,
                               FILE   *filep);


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
W_EXPORT w_cfg_t* w_cfg_new(void);

/*!
 * Free resources allocated in the configuration object.
 */
W_EXPORT void w_cfg_free(w_cfg_t *cf);

/*!
 * Checks whether a key exists in a configuration object.
 */
W_EXPORT wbool w_cfg_has(const w_cfg_t *cf, const char *key);

/*!
 * Deletes a key from a configuration object.
 */
W_EXPORT wbool w_cfg_del(w_cfg_t *cf, const char *key);

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
W_EXPORT wbool w_cfg_set(w_cfg_t *cf, ...);

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
W_EXPORT wbool w_cfg_get(const w_cfg_t *cf, ...);

/*!
 * Obtain the type of a configuration object node.
 */
W_EXPORT w_cfg_type_t w_cfg_type(const w_cfg_t *cf, const char *key);

/*!
 * Dump configuration to a stream.
 * \param cf     Configuration object.
 * \param output Output stream where to write.
 * \return       Whether writing was successful.
 */
W_EXPORT wbool    w_cfg_dump(const w_cfg_t *cf, FILE *output);

/*!
 * Load configuration from a stream.
 * \param input  Input stream
 * \param msg    Pointer to where to store an error message, if there is one.
 * \return       Valid configuration object or \c NULL if there is some error.
 */
W_EXPORT w_cfg_t* w_cfg_load(FILE *input, char **msg);

/*!
 * Dump configuration to a file.
 * \param cf   Configuration object.
 * \param path Path to a file.
 * \return     Whether writing was successful.
 */
W_EXPORT wbool    w_cfg_dump_file(const w_cfg_t *cf, const char *path);

/*!
 * Load configuration from a file.
 * \param path Path to a file.
 * \param msg  Pointer to where to store an error message, if there is one.
 * \return     Valid configuration object or \c NULL if there is some error.
 */
W_EXPORT w_cfg_t* w_cfg_load_file(const char *path, char **msg);

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

/*! Check whether a node is invalid. */
#define w_cfg_isnone(cf, key)    (W_CFG_NONE   == w_cfg_type(cf, key))

/*! Check whether a node contains a subnode. */
#define w_cfg_isnode(cf, key)    (W_CFG_NODE   == w_cfg_type(cf, key))

/*! Check whether a node contains a number. */
#define w_cfg_isnumber(cf, key)  (W_CFG_NUMBER == w_cfg_type(cf, key))

/*! Check whether a node contains a string. */
#define w_cfg_isstring(cf, key)  (W_CFG_STRING == w_cfg_type(cf, key))

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

/*\}*/

#endif /* !__wheel_h__ */

