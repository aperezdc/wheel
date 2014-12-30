/*
 * Main include file for libwheel.
 *
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
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

#ifdef __GNUC__
# define w_offsetof __builtin_offsetof
#else
# define w_offsetof(st, m) \
    ((size_t) ((char*) &((st*) 0)->m - (char*) 0))
#endif /* __GNUC__ */

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

#define W_HAS_FLAG(_bitfield, _bits) \
    (((_bitfield) & (_bits)) == (_bits))

typedef void** w_iterator_t;
typedef void* (*w_traverse_fun_t)(void *data, void *context);
typedef void  (*w_action_fun_t)(void *object, void *context);


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
# define W_UNUSED __attribute__((unused))
# define W_FUNCTION_ATTR_MALLOC __attribute__((malloc))
# define W_FUNCTION_ATTR_PURE \
    __attribute__((pure))
# define W_FUNCTION_ATTR_NORETURN \
    __attribute__((noreturn))
# define W_FUNCTION_ATTR_NOT_NULL(_p) \
    __attribute__((nonnull _p))
# define W_FUNCTION_ATTR_NOT_NULL_RETURN \
    __attribute__((returns_nonnull))
# define W_FUNCTION_ATTR_WARN_UNUSED_RESULT \
    __attribute__((warn_unused_result))
#else
# define W_EXPORT
# define W_HIDDEN
# define W_UNUSED
# define W_FUNCTION_ATTR_PURE
# define W_FUNCTION_ATTR_MALLOC
# define W_FUNCTION_ATTR_NORETURN
# define W_FUNCTION_ATTR_NOT_NULL
# define W_FUNCTION_ATTR_NOT_NULL_RETURN
# define W_FUNCTION_ATTR_WARN_UNUSED_RESULT
#endif

/*\}*/

/*--------------------------------------------------[ libc includes ]-----*/

#ifdef _DEBUG
# include <assert.h>
# define w_assert      assert
#else
# define w_assert(_s)  ((void)0)
#endif

#include <stdbool.h>
#include <stdint.h>
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
W_EXPORT void* w_malloc (size_t sz)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_MALLOC;

/*!
 * Wrapper around realloc(). This will abort and print a message when the
 * pointer returned by realloc() is NULL. Plus, it is possible to pass
 * a NULL pointer and/or a zero size, and it will behave consistently:
 *
 *  - <tt>w_realloc(NULL, 42)</tt> is equivalent to <tt>malloc(42)</tt>
 *  - <tt>w_relloac(ptr, 0)</tt> is equivalent to <tt>free(ptr)</tt>
 */
W_EXPORT void* w_realloc (void *ptr, size_t sz)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT;

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
	((_t *) w_malloc (sizeof(_t)))

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
	((_t *) w_malloc (sizeof (_t) * (_n)))

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
	((_t *) w_realloc (_p, sizeof (_t) * (_n)))

#ifdef __GNUC__
# define w_lobj __attribute__((cleanup(_w_lobj_cleanup)))
# define w_lmem __attribute__((cleanup(_w_lmem_cleanup)))
W_EXPORT void _w_lobj_cleanup (void*);
W_EXPORT void _w_lmem_cleanup (void*);
#else
# define w_lobj - GCC is needed for w_lobj to work -
# define w_lobj - GCC is needed for w_lmem to work -
#endif /* __GNUC__ */

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

/*!
 * Initializes a statically-allocated \ref w_obj_t.
 */
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
static inline void* w_obj_init (w_obj_t *obj)
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

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
void w_obj_destroy (void *obj)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Type of object destructors.
 */
typedef void (*w_obj_dtor_t) (void*);

/*!
 * Registers a destructor to be called when an object is destroyed.
 */
void* w_obj_dtor (void *obj, w_obj_dtor_t fini)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Mark an object as static.
 *
 * When the last reference to an object marked as static is lost, its destructor
 * will be called, but the area of memory occupied by the object will not be
 * freed. This is the same behaviour as for objects initialized with \ref
 * W_OBJ_STATIC. The typical use-case for this function to mark objects that
 * are allocated as part of others, and the function is called during their
 * initialization, like this:
 *
 * \code
 * W_OBJ (my_type) {
 *   w_obj_t     parent;
 *   w_io_unix_t unix_io;
 * };
 *
 * void my_type_free (void *objptr) {
 *   w_obj_destroy (&self->unix_io);
 * }
 *
 * my_type* my_type_new (void) {
 *   my_type *self = w_obj_new (my_type);
 *   w_io_unix_init_fd (&self->unix_io, 0);
 *   w_obj_mark_static (&self->unix_io);
 *   return w_obj_dtor (self, _my_type_free);
 * }
 * \endcode
 */
void w_obj_mark_static (void *obj)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*\}*/


/*------------------------------------------[ forward declarations ]------*/

W_OBJ_DECL (w_io_t);
typedef struct w_io_result w_io_result_t;

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


W_EXPORT void w__warning (const char *func,
                          const char *file,
                          unsigned    line,
                          const char *fmt,
                          ...)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2, 4));

W_EXPORT void w__fatal (const char *func,
                        const char *file,
                        unsigned    line,
                        const char *fmt,
                        ...)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2, 4))
    W_FUNCTION_ATTR_NORETURN;

W_EXPORT void w__debug (const char *func,
                        const char *file,
                        unsigned    line,
                        const char *fmt,
                        ...)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2, 4));

#define W_WARN(...) \
    w__warning (__func__, __FILE__, __LINE__, __VA_ARGS__)

#define W_FATAL(...) \
    w__fatal (__func__, __FILE__, __LINE__, __VA_ARGS__)

#define W_BUG(...) \
    W_FATAL (__VA_ARGS__                                             \
             "This is a BUG. Please report this to the developer.\n" \
             "Build date: " __DATE__ " " __TIME__ "\n")

#ifdef _DEBUG_PRINT
# define W_DEBUG(...) \
    w__debug (__func__, __FILE__, __LINE__, __VA_ARGS__)
# define W_DEBUGC(...) \
    w__debug (NULL, NULL, 0, __VA_ARGS__)
#else
# define W_DEBUG(...) \
    ((void)0)
# define W_DEBUGC(...) \
    ((void)0)
#endif

/*\}*/

/*-----------------------------------------------[ string functions ]-----*/

/*!
 * \defgroup wstr String functions
 * \addtogroup wstr
 * \{
 */

W_EXPORT char* w_strfmtv (const char *fmt, va_list argl)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT char* w_strfmt (const char *fmt, ...)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Hashes the start of a string.
 * \param str String to get the hash of.
 * \param len Number of characters to hash.
 * \return Hash value.
 */
W_EXPORT unsigned w_str_hashl (const char *str, size_t len)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Hashes a string.
 * \param str String to get the hash of.
 * \return Hash value.
 */
W_EXPORT unsigned w_str_hash (const char *str)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Converts a string into a boolean.
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_bool (const char *str, bool *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into an integer.
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_int (const char *str, int *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into an unsigned integer.
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_uint (const char *str, unsigned *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into a long integer.
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_long (const char *str, long *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into a long unsigned integer.
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_ulong (const char *str, unsigned long *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into a float number.
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_float (const char *str, float *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into a double-precision float number.
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_double (const char *str, double *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into a data size value in bytes.
 * The input string is expected to contain a number, optionally terminated
 * by one of the following suffixes:
 *
 * - \b k or \b K : value is in kilobytes.
 * - \b m or \b M : value is in megabytes.
 * - \b g or \b G : value is in gigabytes.
 * - \b b or \b B : value is in bytes (default if no suffix given).
 *
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value, which is
 *            always returned in bytes.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_size_bytes (const char *str, unsigned long long *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Converts a string into a time value in seconds.
 * The input string is expected to contain a number, optionally terminated
 * by one of the following suffixes:
 *
 * - \b y : value is in years.
 * - \b M : value is in months.
 * - \b w : value is in weeks.
 * - \b d : value is in days.
 * - \b h : value is in hours.
 * - \b s : value is in seconds (default if no suffix given).
 *
 * \param str Input (null-terminated) string.
 * \param val Pointer to where to store the parsed value, which is
 *            always returned in seconds.
 * \return Whether the conversion was done successfully.
 */
W_EXPORT bool w_str_time_period (const char *str, unsigned long long *val)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));


static inline char*
w_str_dupl (const char *str, size_t len)
{
    char *r;
    if (str == NULL)
        return NULL;
    r = (char*) memcpy (w_alloc(char, len + 1), str, len);
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
# define w_str_casecmp  strcasecmp
# define w_str_ncasecmp strncasecmp
#else  /* __GLIBC__ */
#include <ctype.h>
static inline int
w_str_casecmp (const char *s1, const char *s2)
{
	w_assert (s1);
	w_assert (s2);

	register int c1 = 0;
	register int c2 = 0;
	while ((*s1 != '\0') && (*s2 != '\0')) {
		c1 = tolower (*s1);
		c2 = tolower (*s2);
		if (c1 != c2) break;
		c1++;
		c2++;
	}
	return ((*s1 == '\0') && (*s2 == '\0')) ? 0 : ((c1 < c2) ? -1 : 1);
}
static inline int
w_str_ncasecmp (const char *s1, const char *s2, size_t n)
{
	w_assert (s1);
	w_assert (s2);

    register int c1 = 0;
    register int c2 = 0;
    while ((*s1 != '\0') && (*s2 != '\0') && n-- > 0) {
        c1 = tolower (*s1);
        c2 = tolower (*s2);
        if (c1 != c2) break;
        c1++;
        c2++;
    }
    if (n == 0 && c1 == c2)
        return 0;
    if (*s1 == '\0' && *s2 == '\0' && n == 0)
        return 0;
    return (c1 < c2) ? -1 : 1;
}
#endif /* __GLIBC__ */


static inline char*
w_strncpy (char *dst, const char *src, size_t n)
{
	w_assert (dst);
	w_assert (src);

	char *result = strncpy (dst, src, n);
	dst[n] = '\0';
	return result;
}

/*\}*/

/*----------------------------------------------------------[ tasks ]-----*/

/*!
 * \defgroup wtask Tasks
 * \addtogroup wtask
 * \{
 */

typedef struct w_task w_task_t;

typedef void (*w_task_func_t) (void*);

/*!
 * Prepares a task for running. The task will start running ready when
 * the task scheduler (see \ref w_task_run_scheduler()) schedules it.
 *
 * \param func Function called to start the task. Receives the value passed
 *      as \data as its only argument.
 * \param data Pointer passed to the task function. May be \c NULL.
 * \param stack_size Size of the stack allocated for the task. The value
 *      is always rounded up to the size of a memory page, which means
 *      it is possible to pass \c 0 to get the smallest possible stack
 *      (usually 4 kB).
 * \return The newly created task.
 */
W_EXPORT w_task_t* w_task_prepare (w_task_func_t func, void *data, size_t stack_size)
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Obtains the task currently running.
 *
 * This function \m must be called from inside a task, once the task scheduler
 * has been started by using \ref w_task_run_scheduler(). Otherwise, calling
 * this function is an error and the execution of the program will be aborted.
 *
 * \return The current task.
 */
W_EXPORT w_task_t* w_task_current (void)
    W_FUNCTION_ATTR_NOT_NULL_RETURN;

/*!
 * Sets the name of a task. The name of the tast is copied as-is, and it
 * is not used. The ability of naming tasks is mainly provided to aid in
 * debugging if client code.
 *
 * \param task Task being renamed.
 * \param name New name for the task. Passing \c NULL will clear the custom
 *      task name and further calls to \ref w_task_get_name() will return
 *      the default task name.
 */
W_EXPORT void w_task_set_name (w_task_t* task, const char *name)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Obtains the name of a task. If a name has not been set using \ref
 * w_task_set_name(), an autogenerated one of the form <tt>Task<ID></tt>
 * will be returned.
 *
 * \param task Task.
 * \return Name of the task.
 */
W_EXPORT const char* w_task_get_name (w_task_t *task)
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Name of the current task. See \ref w_task_get_name() for more information.
 */
static inline const char* w_task_name (void)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN;

static inline const char*
w_task_name (void)
{
    return w_task_get_name (w_task_current ());
}

/*!
 * Marks a task as a system task. System tasks are those which must be
 * always running, and having active system tasks will not prevent \ref
 * w_task_run_scheduler() from returning.
 *
 * \param task Task.
 * \param is_system Whether the task is to be marked as a system task.
 */
W_EXPORT void w_task_set_is_system (w_task_t *task, bool is_system)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Mark the current task as a system task. See \ref w_task_set_system() for
 * more information.
 */
static inline void
w_task_system (void)
{
    w_task_set_is_system (w_task_current (), true);
}

/*!
 * Checks whether a task is a system task.
 *
 * \param task Task.
 * \return Whether the task is a system task.
 */
W_EXPORT bool w_task_get_is_system (w_task_t *task)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Exits a task. This can be used to exit from a task at any point, without
 * needing to return from the task function.
 *
 * This function \m must be called from inside a task, once the task scheduler
 * has been started by using \ref w_task_run_scheduler(). Otherwise, calling
 * this function is an error and the execution of the program will be aborted.
 */
W_EXPORT void w_task_exit (void);

/*!
 * Runs the task scheduler. The scheduler will pick a task, switch to it, and
 * wait for the task to give up the CPU using \ref w_task_yield(), at which
 * point a new task will be chosen and switched to. The process will continue
 * while there are normal tasks pending to finish.
 *
 * This function \b must be called in the main function of a program.
 * Typically:
 *
 * \code
 * extern void process_argument (void*);
 *
 * int main (int argc, char **argv)
 * {
 *     while (argc--)
 *         w_task_prepare (process_argument, *argv++, 0);
 *     w_task_run_scheduler ();
 *     return 0;
 * }
 * \endcode
 */
W_EXPORT void w_task_run_scheduler (void);

/*!
 * Explicitly give up the CPU. The current task will allow other tasks to run,
 * and it will be re-scheduled again once the other tasks have had the chance
 * to run.
 *
 * This function \m must be called from inside a task, once the task scheduler
 * has been started by using \ref w_task_run_scheduler(). Otherwise, calling
 * this function is an error and the execution of the program will be aborted.
 */
W_EXPORT void w_task_yield (void);


W_EXPORT w_io_result_t w_task_yield_io_read (w_io_t *io, void *buf, size_t len)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT w_io_result_t w_task_yield_io_write (w_io_t *io, const void *buf, size_t len)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*\}*/

/*----------------------------------------------------------[ lists ]-----*/

/*!
 * \defgroup wlist Lists
 * \addtogroup wlist
 * \{
 *
 * List data structure. The implementation uses a doubly-linked list,
 * meaning that most operations are very efficient, and the list can be
 * traversed both forward and backward. Slow operations are those that
 * use numeric indexes to refer to elements in the list: \ref w_list_at,
 * \ref w_list_insert_at and \ref w_list_del_at. The later should be
 * avoided if possible, but still they are provided as a convenience.
 *
 * Negative numeric indexes may be passed to functions, with the same
 * meaning as in Python: \c -1 refers to the last element, \c -2 to the
 * element before the last, and so on.
 *
 * <b>Lists of objects</b>
 *
 * If a list is meant to contain objects (see \ref wobj), it is possible
 * to let the list reference-count the objects (using \ref w_obj_ref and
 * \ref w_obj_unref) by passing \c true when creating a list with
 * \ref w_list_new. If enabled, whenever an item is added to the list,
 * its reference count will be increased, and it will be decreased when
 * the item is removed from the list.
 */

/*!
 * List type.
 */
W_OBJ (w_list_t)
{
    w_obj_t parent;
    size_t  size;
    bool    refs;
    /* actual data is stored in the private area of the list */
};

/*!
 * Get the number of elements in a list.
 */
static inline size_t w_list_size (w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline size_t
w_list_size (w_list_t *list)
{
    w_assert (list);
    return list->size;
}

/*! 
 * Checks whether a list is empty.
 */
static inline bool w_list_is_empty (w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline bool
w_list_is_empty (w_list_t *list)
{
    w_assert (list);
    return list->size > 0;
}

/*!
 * Creates a new list.
 * \param refs Whether to handle references to objects in the list.
 * \return A new, empty list.
 */
W_EXPORT w_list_t* w_list_new (bool refs)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN;

/*! Clears all elements from a list. */
W_EXPORT void w_list_clear (w_list_t *list)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Obtains the value stored at a given position.
 * \param index Position in the list. Negative indexes count from the end of
 *              the list, i.e. \c -1 would be the last element.
 */
W_EXPORT void* w_list_at (const w_list_t *list, long index)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Appends an element to the end of a list. */
W_EXPORT void w_list_push_head (w_list_t *list, void *item)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Inserts an element in the beginning of the list. */
W_EXPORT void w_list_push_tail (w_list_t *list, void *item)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Removes the element from the beginning of the list and returns it.
 * Note that this will \b not decrease the reference counter when reference
 * counting is enabled: it is assumed that the caller will use the returned
 * item.
 */
W_EXPORT void* w_list_pop_head (w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Removes the element from the end of the list and returns it.
 * Note that this will \b not decrease the reference counter when reference
 * counting is enabled: it is assumed that the caller will use the returned
 * item.
 */
W_EXPORT void* w_list_pop_tail (w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Obtains the element at the first position in the list. */
W_EXPORT void* w_list_head (const w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Obtains the element at the last position in the list. */
W_EXPORT void* w_list_tail (const w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Obtains a pointer to the first element in the list. */
W_EXPORT w_iterator_t w_list_first (const w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Obtains a pointer to the last element in the list. */
W_EXPORT w_iterator_t w_list_last (const w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Obtains a pointer to the next element. */
W_EXPORT w_iterator_t w_list_next (const w_list_t *list, w_iterator_t i)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*! Obtains a pointer to the previous element. */
W_EXPORT w_iterator_t w_list_prev (const w_list_t *list, w_iterator_t i)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Applies a function to each element in a list, in forward order.
 *
 * Each element in the list, in forward order, will be passed to function
 * \e f. The function can mutate the element and return a different one,
 * which will replace the original element in the list. An optional context
 * (\e ctx) pointer will be passed to \e f every time it is called.
 */
W_EXPORT void w_list_traverse (const w_list_t *list, w_traverse_fun_t f, void *ctx)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Applies a function to each element in a list, in reverse order.
 *
 * \see w_list_traverse
 */
W_EXPORT void w_list_traverse_rev (const w_list_t *list, w_traverse_fun_t f, void *ctx)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Defines a loop over all items in a list. This can be used as an
 * alternative to \ref w_list_traverse. Typical usage:
 * \code
 * w_list_t *list = make_string_list ();
 * w_iterator_t i;
 *
 * w_list_foreach (list, i)
 *    w_io_format (w_stdout, "$s\n", (const char*) *i);
 * \endcode
 *
 * \see w_list_foreach_rev, w_list_traverse.
 */
#define w_list_foreach(_v, _l)                \
    for (w_iterator_t _v = w_list_first (_l); \
         _v != NULL;                          \
         _v = w_list_next ((_l), _v))

/*!
 * Defines a loop, in reverse order, over all items in a list. This can be
 * used as an alternative to \ref w_list_traverse_rev.
 *
 * \see w_list_foreach, w_list_traverse_reverse.
 */
#define w_list_foreach_reverse(_v, _l)       \
    for (w_iterator_t _v = w_list_last (_l); \
         _v != NULL;                         \
         _v = w_list_prev ((_l), _v))

/*!
 * Inserts an element in a list before a particular position.
 * The \e item is inserted in the \e list before the position pointed by
 * \e i.
 */
W_EXPORT void w_list_insert_before (w_list_t *list, w_iterator_t i, void *item)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Inserts an element in a list after a particular position.
 * The \e item is inserted in the \e list after the position pointed by
 * \e i.
 */
W_EXPORT void w_list_insert_after (w_list_t *list, w_iterator_t i, void *item)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Inserts an element in a list at a given position.
 * The \e item is inserted at the given numeric \e index. Note that some
 * particular indexes like \c 0 (first position) and \c -1 (last position)
 * will be optimized out, but in general this function runs in \e O(n).
 */
W_EXPORT void w_list_insert_at (w_list_t *list, long index, void *item)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Deletes the element at a given position in a list.
 */
W_EXPORT void w_list_del (w_list_t *list, w_iterator_t i)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Deletes the element at a given (numeric) position in a list.
 */
W_EXPORT void w_list_del_at (w_list_t *list, long index)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Deletes the element at the beginning of the list. Contrary to
 * \ref w_list_pop_head, the element is \b not returned, and the
 * reference counter decresed (if reference counting enabled).
 */
static inline void w_list_del_head (w_list_t *list)
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline void
w_list_del_head (w_list_t *list)
{
    w_assert (list);
    w_list_del (list, w_list_first (list));
}

/*!
 * Deletes the element at the end of the list. Contrary to
 * \ref w_list_pop_tail, the element is \b not returned, and the
 * reference counter decresed (if reference counting enabled).
 */
static inline void w_list_del_tail (w_list_t *list)
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline void
w_list_del_tail (w_list_t *list)
{
    w_assert (list);
    w_list_del (list, w_list_last (list));
}

/* Commonly-used aliases */
static inline void w_list_insert (w_list_t *list, w_iterator_t i, void *item)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

static inline void
w_list_insert (w_list_t *list, w_iterator_t i, void* item)
{
    w_assert (list);
    w_assert (i);
    w_list_insert_before (list, i, item);
}

static inline void w_list_append (w_list_t *list, void *item)
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline void
w_list_append (w_list_t *list, void *item)
{
    w_assert (list);
    w_list_push_tail (list, item);
}

static inline void* w_list_pop (w_list_t *list)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline void*
w_list_pop (w_list_t *list)
{
    w_assert (list);
    return w_list_pop_tail (list);
}

/*\}*/

/*---------------------------------------------------[ dictionaries ]-----*/

/*!
 * \defgroup wdict Hash-based dictionaries
 * \addtogroup wdict
 * \{
 */

typedef struct w_dict_node w_dict_node_t;

W_OBJ (w_dict_t)
{
    w_obj_t         parent;
    w_dict_node_t **nodes;
    w_dict_node_t  *first;
    size_t          count;
    size_t          size;
    bool            refs;
};

/*!
 * Create a new dictionary.
 * \param refs Automatically update reference-count for values stored in the
 *             hash table. Passing \c true assumes that all items in the
 *             dictionary will be objects (derived from \ref w_obj_t).
 */
W_EXPORT w_dict_t* w_dict_new (bool refs)
    W_FUNCTION_ATTR_NOT_NULL_RETURN;

/*!
 * Clears the contents of a dictionary.
 */
W_EXPORT void w_dict_clear (w_dict_t *d)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Get the number of items in a dictionary.
 */
static inline size_t w_dict_size (const w_dict_t *d)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline size_t
w_dict_size (const w_dict_t *d)
{
    w_assert (d);
    return d->count;
}

/*!
 * Check whether a dictionary is empty.
 */
static inline bool w_dict_is_empty (const w_dict_t *d)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline bool
w_dict_is_empty (const w_dict_t *d)
{
    w_assert (d);
    return d->count == 0;
}

W_EXPORT void* w_dict_getn (const w_dict_t *d, const char *key, size_t keylen)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT void w_dict_setn (w_dict_t *d, const char *key, size_t keylen, void *data)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT void w_dict_deln (w_dict_t *d, const char *key, size_t keylen)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Delete an item from a dictionary given its key.
 */
W_EXPORT void w_dict_del (w_dict_t *d, const char *key)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Set an item in a dictionary.
 */
W_EXPORT void w_dict_set (w_dict_t *d, const char *key, void *data)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Get an item from a dictionary.
 */
W_EXPORT void* w_dict_get (const w_dict_t *d, const char *key)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Update a dictionary with the contents of another. For each key present in
 * the source dictionary, copy it and its value to the destination one, if
 * the key already existed in the destination, the value gets overwritten.
 * \param d Destination dictionary.
 * \param o Source dictionary.
 */
W_EXPORT void w_dict_update (w_dict_t *d, const w_dict_t *o)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT void w_dict_traverse (w_dict_t *d, w_traverse_fun_t f, void *ctx)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT void w_dict_traverse_keys (w_dict_t *d, w_traverse_fun_t f, void *ctx)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT void w_dict_traverse_values (w_dict_t *d, w_traverse_fun_t f, void *ctx)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT w_iterator_t w_dict_first (const w_dict_t *d)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT w_iterator_t w_dict_next (const w_dict_t *d, w_iterator_t i)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT const char* w_dict_iterator_get_key (w_iterator_t i)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

#define w_dict_foreach(_i, _d)                \
    for (w_iterator_t _i = w_dict_first (_d); \
         _i != NULL;                          \
         _i = w_dict_next ((_d), _i))

/*\}*/

/*----------------------------------------------------[ CLI parsing ]-----*/

/*!
 * \defgroup wopt Command line parsing
 * \addtogroup wopt
 * \{
 */

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

/*!
 * Type of option parsing action callbacks.
 */
typedef w_opt_status_t (*w_opt_action_t)(const w_opt_context_t*);


/*!
 * Command line option information.
 */
struct w_opt
{
	unsigned       narg;   /*!< Number of arguments consumed.           */
	unsigned char  letter; /*!< Letter for short option style parsing.  */
	const char    *string; /*!< String for long option style parsing.   */
	w_opt_action_t action; /*!< Action performed when option is parsed. */
	void          *extra;  /*!< Additional argument to the action.      */
	const char    *info;   /*!< Text describing the option.             */
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
	char         **argv;
	const w_opt_t *option;
	void          *userdata;
	char         **argument;
};


#define _W_M_(x) \
    W_EXPORT w_opt_status_t x(const w_opt_context_t*) \
        W_FUNCTION_ATTR_WARN_UNUSED_RESULT \
        W_FUNCTION_ATTR_NOT_NULL ((1))
_W_M_(W_OPT_BOOL);
_W_M_(W_OPT_INT);
_W_M_(W_OPT_UINT);
_W_M_(W_OPT_LONG);
_W_M_(W_OPT_ULONG);
_W_M_(W_OPT_FLOAT);
_W_M_(W_OPT_DOUBLE);
_W_M_(W_OPT_STRING);
_W_M_(W_OPT_TIME_PERIOD);
_W_M_(W_OPT_DATA_SIZE);
#undef _W_M_

W_EXPORT w_opt_status_t w_opt_files_action(const w_opt_context_t*)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

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
                         const char   *syntax)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2, 3));

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
                              char         **argv)
    W_FUNCTION_ATTR_NOT_NULL ((1, 6));

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
W_EXPORT bool w_opt_parse_io (const w_opt_t *opt,
                              w_io_t        *input,
                              char         **msg)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

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
struct w_parse
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
typedef struct w_parse w_parse_t;

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
W_EXPORT void* w_parse_run (w_parse_t    *p,
                            w_io_t       *input,
                            int           comment,
                            w_parse_fun_t parse_fun,
                            void         *context,
                            char         **msg)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2, 4));

/*!
 * Formats an error string and raises an error.
 *
 * \sa w_parse_ferror(), w_parse_rerror()
 * \param fmt Format string (as passed to \ref w_io_format)
 * \param ... Arguments for the format string.
 */
W_EXPORT void w_parse_error (w_parse_t *p, const char *fmt, ...)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Format an error string. The formatted string will be saved in the
 * \ref w_parse_t::error field.
 *
 * \param fmt Format string (as passed to \ref w_io_format).
 * \param ... Arguments for the format string.
 */
W_EXPORT void w_parse_ferror (w_parse_t *p, const char *fmt, ...)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Raise a parsing error. Make sure you free intermediate strucutures and
 * data parsing may have created before calling this function, otherwise
 * memory will leak.
 *
 * \sa w_parse_ferror(), w_parse_error()
 */
W_EXPORT void w_parse_rerror (w_parse_t *p)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Gets the next character in the input, skipping over comments. If comments
 * are enabled i.e. the \ref w_parse_t::comment field is different than
 * \c 0, then when a comment character is found, the entire line is skipped.
 */
W_EXPORT void w_parse_getchar (w_parse_t *p)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Skips whitespace in the input. All characters for which \c isspace()
 * returns true will be ignored until the first non-blank or the end of
 * the input stream is found.
 */
W_EXPORT void w_parse_skip_ws (w_parse_t *p)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Gets a string enclosed in double-quotes from the input. Escape characters
 * in the string are interpreted, same way as the C compiler does. This
 * function never raises errors, but returns \c NULL when there is some
 * error in the input. The caller is responsible for calling \ref w_free()
 * on the returned string.
 */
W_EXPORT char* w_parse_string (w_parse_t *p)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Gets a C-like identifier. Identifiers are the same as in C: a sequence of
 * non-blank character, being the first one a letter or an underscore, and
 * the rest letters, numbers and underscores. This function never raises
 * errors, but returns \c NULL when there is some error in the input. The
 * caller is responsible for calling \ref w_free() on the returned string.
 */
W_EXPORT char* w_parse_ident (w_parse_t *p)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Gets a single word from the input. A \e word here is any sequence of
 * non-whitespace characters. This function will never raise errors, but
 * returns \c NULL when the word cannot be read. The caller is responsible
 * for calling \ref w_free() on the returned string.
 */
W_EXPORT char* w_parse_word (w_parse_t *p)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Parses a floating point value as \c double.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
W_EXPORT bool w_parse_double (w_parse_t *p, double *value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Parses a aigned integer as an <tt>unsigned long</tt> value. Note that
 * prefix \c 0x will cause the number to be parsed as hexadecimal, and
 * prefix \c 0 as octal.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
W_EXPORT bool w_parse_ulong (w_parse_t *p, unsigned long *value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Parses a aigned integer as a \c long value. Note that prefix \c 0x will
 * cause the number to be parsed as hexadecimal, and prefix \c 0 as octal.
 *
 * \param value Pointer to where to store the result.
 * \return Whether the value was successfully parsed.
 */
W_EXPORT bool w_parse_long (w_parse_t *p, long *value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

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
 * \param p Pointer to a \c w_parse_t.
 * \param c Character to be matched.
 */
static inline void w_parse_match (w_parse_t *p, int c)
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline void
w_parse_match (w_parse_t *p, int c)
{
    w_assert (p);
    w_parse_match_with_cleanup(p, c, (void)0);
}

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
 * functions can be used, and once working with the buffer has finished,
 * its contents must be deallocated using \ref w_buf_clear:
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
 * w_buf_clear (&b);
 * \endcode
 *
 * In the above example, a buffer is created, some strings appended to it
 * (there are convenience functions to use them as string buffers) and
 * finally the resulting string is printed and the buffer-held resources
 * freed.
 */

typedef struct w_buf w_buf_t;

/*!
 * \brief A variable-length buffer for arbitrary data.
 * Can hold any kind of data, including null characters, as data length is
 * tracked separately. To initialize a buffer, do something like this:
 * \code
 *   w_buf_t mybuffer = W_BUF;
 * \endcode
 */
struct w_buf
{
    char  *data;  /*!< Actual data */
    size_t size;  /*!< Data length */
    size_t alloc; /*!< Allocated buffer size */
};


/*!
 * Initializer for \ref w_buf_t
 */
#define W_BUF { NULL, 0, 0 }


/*!
 * Get the length of a buffer.
 */
static inline size_t w_buf_size (const w_buf_t *buf)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline size_t w_buf_size (const w_buf_t *buf)
{
    w_assert (buf);
    return buf->size;
}

/*!
 * Get a pointer to the internal data stored by the buffer.
 * Note that this may be \c NULL.
 */
static inline char* w_buf_data (w_buf_t *buf)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline char*
w_buf_data (w_buf_t *buf)
{
    w_assert (buf);
    return buf->data;
}

static inline const char* w_buf_const_data (const w_buf_t *buf)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline const char*
w_buf_const_data (const w_buf_t *buf)
{
    w_assert (buf);
    return buf->data;
}

/*!
 * Checks whether a buffer is empty.
 */
static inline bool w_buf_is_empty (const w_buf_t *buf)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline bool
w_buf_is_empty (const w_buf_t *buf)
{
    w_assert (buf);
    return buf->size == 0;
}

/*!
 * Adjust the size of a buffer keeping contents.
 * This is mostly useful for trimming contents, when shrinking the buffer.
 * When a buffer grows, random data is liklely to appear at the end.
 * \param buf A \ref w_buf_t buffer
 * \param size New size of the buffer
 */
W_EXPORT void w_buf_resize (w_buf_t *buf, size_t size)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Set the contents of a buffer to a C string.
 * \param buf A \ref w_buf_t buffer
 * \param str String to set the buffer to
 */
W_EXPORT void w_buf_set_str (w_buf_t    *buf,
                             const char *str)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Appends the contents of a memory block to a buffer.
 * \param buf A \ref w_buf_t buffer
 * \param ptr Pointer to the block of memory
 * \param size Length of the memory block
 */
W_EXPORT void w_buf_append_mem (w_buf_t    *buf,
                                const void *ptr,
                                size_t      size)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Appends a character to a buffer
 * \param buf A \ref w_buf_t buffer
 * \param chr A character
 */
W_EXPORT void w_buf_append_char (w_buf_t *buf,
                                 int      chr)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Appends a string to a buffer.
 * \param buf A \ref w_buf_t buffer
 * \param str A string
 */
W_EXPORT void w_buf_append_str (w_buf_t    *buf,
                                const char *str)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Appends a buffer to another buffer
 * \param buf A \ref w_buf_t buffer
 * \param src Another \ref w_buf_t which contains the data to be appended
 */
W_EXPORT void w_buf_append_buf (w_buf_t       *buf,
                                const w_buf_t *src)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Appends formatted text into a buffer. Available formatting options are
 * the same as for \ref w_io_format.
 * \param buf A \ref w_buf_t buffer.
 * \param fmt Format string.
 */
W_EXPORT w_io_result_t w_buf_format (w_buf_t    *buf,
                                     const char *fmt,
                                     ...)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT w_io_result_t w_buf_formatv (w_buf_t    *buf,
                                      const char *fmt,
                                      va_list     args)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Frees the contents of a buffer.
 * \param buf A \ref w_buf_t buffer
 */
W_EXPORT void w_buf_clear (w_buf_t *buf)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Get the buffer as a C string.
 * Note that if the buffer contains embedded null characters, functions like
 * \c strlen() will not report the full length of the buffer.
 * \param buf A \ref w_buf_t buffer
 */
W_EXPORT char* w_buf_str (w_buf_t *buf)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*\}*/

/*--------------------------------------------------[ input/output ]------*/

/*!
 * \defgroup wio Input/output
 * \addtogroup wio
 * \{
 */

struct w_io_result {
    union {
        ssize_t error;
        ssize_t bytes;
    };
};


#define W__LCAT3(__a, __b) __a ## __b
#define W__LCAT2(_name, _line) W__LCAT3(_name ## _, _line)
#define W__LCAT(_name) W__LCAT2(_name, __LINE__)


#define W_IO_RESULT(_v) \
    ((w_io_result_t) { .bytes = (_v) })

#define W_IO_RESULT_ERROR(_v) \
    ((w_io_result_t) { .error = - (_v) })

#define W_IO_RESULT_EOF \
    ((w_io_result_t) { .error = -W_IO_EOF })

#define W_IO_RESULT_SUCCESS \
    ((w_io_result_t) { .error = W_IO_SUCCESS })


#define W_IO_CHAIN(_r, _iocall)                           \
    do {                                                  \
        w_io_result_t W__LCAT (io_chain_r) = (_iocall);   \
        if (w_io_result_error (W__LCAT (io_chain_r)))     \
            return W__LCAT (io_chain_r);                  \
        (_r).bytes += W__LCAT (io_chain_r).bytes;         \
    } while (0);

#define W_IO_CHECK(_iocall)                               \
    do {                                                  \
        w_io_result_t W__LCAT (io_check_r) = (_iocall);   \
        if (w_io_result_error (W__LCAT (io_check_r)))     \
            return W__LCAT (io_check_r);                  \
    } while (0)

#define W_IO_CHECK_RETURN(_iocall, _retval)               \
    do {                                                  \
        w_io_result_t W__LCAT (io_check_do) = (_iocall);  \
        if (w_io_failed (W__LCAT (io_check_do)))          \
            return (_retval);                             \
    } while (0)

#define W_IO_CHECK_BYTES(_iocall1, _action, _iocall2)     \
    do {                                                  \
        w_io_result_t W__LCAT (io_check_b1) = (_iocall1); \
        if (w_io_result_error (W__LCAT (io_check_b1)))    \
            return W__LCAT( io_check_b1);                 \
        w_io_result_t W__LCAT (io_check_b2) = (_iocall2); \
        if (w_io_result_error (W__LCAT (io_check_b2)))    \
            return W__LCAT (io_check_b2);                 \
        if (W__LCAT (io_check_b1).bytes !=                \
            W__LCAT (io_check_b2).bytes)                  \
            return W_IO_RESULT_ERROR(1);                  \
        _action W__LCAT (io_check_b2);                    \
    } while (0)

#define W_IO_NORESULT(_iocall)                            \
    do {                                                  \
        W_UNUSED w_io_result_t W__LCAT (io_ignore) =      \
            (_iocall);                                    \
        (void) W__LCAT (io_ignore);                       \
    } while (0)


enum w_io_error
{
    W_IO_SUCCESS = 0,
    W_IO_EOF     = 0xFFE0FFF, /*!< End of file reached.  */
};


static inline unsigned
w_io_result_error (w_io_result_t r) {
    return (r.error < 0 && r.error != -W_IO_EOF) ? -r.error : W_IO_SUCCESS;
}

static inline size_t
w_io_result_bytes (w_io_result_t r) {
    return (r.error < 0 || r.error == -W_IO_EOF) ? 0 : r.bytes;
}

static inline bool
w_io_failed (w_io_result_t r) {
    return w_io_result_error (r) != W_IO_SUCCESS;
}

static inline bool
w_io_eof (w_io_result_t r) {
    return r.error < 0 && r.error == -W_IO_EOF;
}


/*!
 * Input/output descriptor.
 */
W_OBJ_DEF (w_io_t)
{
    w_obj_t parent;
    int     backch;

    w_io_result_t (*close) (w_io_t *io);
    w_io_result_t (*write) (w_io_t *io, const void *buf, size_t size);
    w_io_result_t (*read ) (w_io_t *io, void       *buf, size_t size);
    w_io_result_t (*flush) (w_io_t *io);
    int           (*getfd) (w_io_t *io);
};


/*!
 * Initializes a base I/O stream.
 */
W_EXPORT void w_io_init (w_io_t *io)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Closes an input/output descriptor. If the \c close callback of the
 * descriptor is \c NULL, then no action is performed.
 *
 * \param io An input/output descriptor.
 */
W_EXPORT w_io_result_t w_io_close (w_io_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Writes data to an input/output descriptor. If the descriptor has no
 * \c write callback, then \c -1 is returned and \c errno is set to
 * \c EBADF.
 *
 * \param io  An input/output descriptor.
 * \param buf Pointer to the data to be written.
 * \param size Result of the operation.
 */
W_EXPORT w_io_result_t w_io_write (w_io_t     *io,
                                   const void *buf,
                                   size_t      size)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Reads data from an input/output descriptor. If the descriptor has no
 * \c read callback, then \c -1 is returned and \c errno is set to
 * \c EBADF.
 */
W_EXPORT w_io_result_t w_io_read (w_io_t *io,
                                  void   *buf,
                                  size_t  size)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

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
W_EXPORT w_io_result_t w_io_read_until (w_io_t  *io,
                                        w_buf_t *data,
                                        w_buf_t *overflow,
                                        int      stopchar,
                                        unsigned maxbytes)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2, 3));

/*!
 * Reads a line from an input stream.
 * This is a convenience macro that calls \ref w_io_read_until passing \c
 * '\n' as stop character.
 */
static inline w_io_result_t w_io_read_line (w_io_t  *io,
                                            w_buf_t *data,
                                            w_buf_t *overflow,
                                            unsigned maxbytes)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2, 3));

static inline w_io_result_t w_io_read_line (w_io_t  *io,
                                            w_buf_t *data,
                                            w_buf_t *overflow,
                                            unsigned maxbytes)
{
    w_assert (io);
    w_assert (data);
    w_assert (overflow);
    return w_io_read_until (io, data, overflow, '\n', maxbytes);
}

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
W_EXPORT w_io_result_t w_io_format (w_io_t     *io,
                                    const char *fmt,
                                    ...)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Formats text and writes it to an I/O object. This version accepts
 * a standard variable argument list. For the available formatting options,
 * read the documentation for \ref w_io_format().
 * \param io   An input/output descriptor.
 * \param fmt  Format string.
 * \param args Argument list.
 */
W_EXPORT w_io_result_t w_io_formatv (w_io_t     *io,
                                     const char *fmt,
                                     va_list     args)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT w_io_result_t w_io_format_long (w_io_t *io, long value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT w_io_result_t w_io_format_ulong (w_io_t *io, unsigned long value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT w_io_result_t w_io_format_double (w_io_t *io, double value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT w_io_result_t w_io_format_ulong_hex (w_io_t *io, unsigned long value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT w_io_result_t w_io_format_ulong_oct (w_io_t *io, unsigned long value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

#define w_print(...) \
        W_IO_NORESULT (w_io_format (w_stdout, __VA_ARGS__))
#define w_printerr(...) \
        W_IO_NORESULT (w_io_format (w_stderr, __VA_ARGS__))

/*!
 * Reads formatted input from an I/O object.
 * \param io  An input/output descriptor.
 * \param fmt Format string.
 */
W_EXPORT ssize_t w_io_fscan (w_io_t     *io,
                             const char *fmt,
                             ...)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Reads formatted input from an I/O object. This version accepts a standard
 * variable argument list.
 * \param io  An input/output descriptor.
 * \param fmt Format string.
 * \param args Argument list.
 */
W_EXPORT ssize_t w_io_fscanv (w_io_t     *io,
                              const char *fmt,
                              va_list     args)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));


W_EXPORT bool w_io_fscan_float (w_io_t *io, float *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_double (w_io_t *io, double *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_int (w_io_t *io, int *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_uint (w_io_t *io, unsigned int *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_long (w_io_t *io, long *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_ulong     (w_io_t *io, unsigned long *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_ulong_hex (w_io_t *io, unsigned long *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_ulong_oct (w_io_t *io, unsigned long *result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_io_fscan_word (w_io_t *io, char **result)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Reads a single character from an I/O object.
 * \param io An input/output descriptor.
 * \return   The read character, or either \ref W_IO_EOF if the end of file
 *           was reached, or \ref W_IO_ERR if there was some error.
 */
W_EXPORT int w_io_getchar (w_io_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Writes a single character to an I/O object.
 * \param io An input/output descriptor.
 * \param ch Character.
 * \return   Number of bytes written, or a negative value on error.
 */
W_EXPORT w_io_result_t w_io_putchar (w_io_t *io, int ch)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 */
W_EXPORT void w_io_putback (w_io_t *io, int ch)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Flushes pending buffered data.
 * \param io An input/output descriptor.
 * \return   Whether there was some error.
 */
W_EXPORT w_io_result_t w_io_flush (w_io_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT int w_io_get_fd (w_io_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Input/output object on Unix file descriptors.
 */
W_OBJ (w_io_unix_t)
{
    w_io_t parent;
    int    fd;
};

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
                                 unsigned    perm)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Create an I/O object to be used with an Unix file descriptor.
 * This function allows initialization from an existing, valid file
 * descriptor.
 * \param fd Unix file descriptor.
 * \sa W_IO_UNIX_FD
 */
W_EXPORT w_io_t* w_io_unix_open_fd (int fd)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT;

/*!
 * Initialize an I/O object to be used with an Unix file descriptor.
 * This calls \c open() and then uses \ref w_io_unix_init_fd.
 * This function is not intended to be used directly, but it is provided as
 * a convenience for code extending \ref w_io_unix_t
 */
W_EXPORT bool w_io_unix_init (w_io_unix_t *io,
                              const char  *path,
                              int          mode,
                              unsigned     perm)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Initialize an I/O object to be used with an Unix file descriptor.
 * This function is not inteded to be used directly, but it is provided as
 * a convenience for code extending \ref w_io_unix_t.
 * \sa W_IO_UNIX_FD
 */
W_EXPORT void w_io_unix_init_fd (w_io_unix_t *io, int fd)
    W_FUNCTION_ATTR_NOT_NULL ((1));


/*!
 * Socket kinds.
 * \see w_io_socket_open
 */
enum w_io_socket_kind
{
    W_IO_SOCKET_UNIX, /*!< Unix named socket.    */
    W_IO_SOCKET_TCP4, /*!< IPv4 TCP socket.      */
};
typedef enum w_io_socket_kind w_io_socket_kind_t;

/*!
 * Modes available for server-side sockets.
 * \see w_io_socket_serve
 */
enum w_io_socket_serve_mode
{
    W_IO_SOCKET_SINGLE, /*!< Serve one client at a time.                   */
    W_IO_SOCKET_THREAD, /*!< Each client is serviced using a new thread.   */
    W_IO_SOCKET_FORK,   /*!< Each client is serviced by forking a process. */
};
typedef enum w_io_socket_serve_mode w_io_socket_serve_mode_t;


#define W_IO_SOCKET_SA_LEN 1024

/*!
 * Performs input/output on sockets.
 */
W_OBJ (w_io_socket_t)
{
    w_io_unix_t        parent;
    w_io_socket_kind_t kind;
    size_t             slen;
    bool               bound;
    char               sa[W_IO_SOCKET_SA_LEN];
};


/*!
 * Obtain the kind of a socket.
 */
static inline w_io_socket_kind_t w_io_socket_get_kind (w_io_socket_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline w_io_socket_kind_t
w_io_socket_get_kind (w_io_socket_t *io)
{
    w_assert (io);
    return io->kind;
}

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
W_EXPORT w_io_t* w_io_socket_open (w_io_socket_kind_t kind, ...)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT;

/*!
 * Perform basic socket initialization. In general applications should use
 * \ref w_io_socket_open instead of this function, but it is provided to be
 * used from code extending \ref w_io_socket_t.
 * \param io   An input/output object.
 * \param kind Socket kind.
 * \return Whether initialization was successful.
 * \sa w_io_socket_open
 */
W_EXPORT bool w_io_socket_init (w_io_socket_t *io,
                                w_io_socket_kind_t kind, ...)
    W_FUNCTION_ATTR_NOT_NULL ((1));

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
W_EXPORT bool w_io_socket_serve (w_io_socket_t *io,
                                 w_io_socket_serve_mode_t mode,
                                 bool (*handler) (w_io_socket_t*))
    W_FUNCTION_ATTR_NOT_NULL ((1, 3));

/*!
 * Connect a socket to a server. This makes a connection to the host
 * specified when creating the socket with \ref w_io_socket_open, and
 * puts it in client mode. Once the socket is successfully connected,
 * read and write operations can be performed in the socket.
 * \return Whether the connection was setup successfully.
 */
W_EXPORT bool w_io_socket_connect (w_io_socket_t *io)
    W_FUNCTION_ATTR_NOT_NULL ((1));

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
W_EXPORT bool w_io_socket_send_eof (w_io_socket_t *io)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Obtain the path in the filesystem for an Unix socket.
 */
W_EXPORT const char* w_io_socket_unix_path (w_io_socket_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));


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
 */
W_EXPORT w_io_t* w_io_stdio_open (FILE *fp)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Initialize an I/O object to be used with a C standard file descriptor.
 * This function is not meant to be used directly, but is provided as
 * a convenience for other code extending \ref w_io_stdio_t.
 */
W_EXPORT void w_io_stdio_init (w_io_stdio_t *io, FILE *fp)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

#endif /* W_CONF_STDIO */


/*!
 * Perform input/output on memory buffers.
 */
W_OBJ (w_io_buf_t)
{
    w_io_t   parent;
    w_buf_t  buf;
    size_t   pos;
    w_buf_t *bufp;
};


/*!
 * Create an I/O object to be used with a buffer.
 * \param buf Pointer to a w_buf_t. Passing NULL will initialize a new
 *            \ref w_buf_t internally which can be retrieved with
 *            \ref w_io_buf_get_buffer. If you pass a non-NULL buffer, then
 *            you will be responsible to call \ref w_buf_clear on it.
 */
W_EXPORT w_io_t* w_io_buf_open (w_buf_t *buf)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN;

/*!
 * Initialize an I/O object in the stack to be used with a buffer.
 * This function is not meant to be used directly, but is provided as
 * a convenience for other code extending \ref w_io_buf_t, or wanting to
 * quickly allocate a \ref w_io_buf_t in the stack.
 *
 * \param io I/O buffer object.
 * \param buf Buffer with optional initial data.
 * \param append Whether \i buf will be used for appending data.
 */
W_EXPORT void w_io_buf_init (w_io_buf_t *io, w_buf_t *buf, bool append)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Obtain a pointer to the buffer being used by a \ref w_io_buf_t.
 */
static inline w_buf_t* w_io_buf_get_buffer (w_io_buf_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline w_buf_t*
w_io_buf_get_buffer (w_io_buf_t *io)
{
    w_assert (io);
    return io->bufp;
}

/*!
 * Obtain a string representation of a \ref w_io_buf_t I/O object.
 */
static inline char* w_io_buf_str (w_io_buf_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline char* w_io_buf_str (w_io_buf_t *io)
{
    w_assert (io);
    return w_buf_str (io->bufp);
}

/*!
 * Perform input/output on fixed-size memory regions.
 */
W_OBJ (w_io_mem_t)
{
    w_io_t   parent;
    uint8_t *data;
    size_t   size;
    size_t   pos;
};


/*!
 * Create an I/O object to be used with a fixed-size memory region.
 *
 * \param data Pointer to a memory region.
 * \param size Size, in bytes, of the memory region.
 */
W_EXPORT w_io_t* w_io_mem_open (uint8_t *data, size_t size)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Initialize an I/O object in the stack to be used with a fixed-size memory
 * region. This function is not meant to be used directly, but is provided as
 * a convenience for other code extending \ref w_io_mem_t, or wanting to
 * quickly allocate a \ref w_io_mem_T in the stack.
 *
 * \param io In-memory I/O object.
 * \param data Pointer to a memory region.
 * \param size Size, in bytes, of the memory region.
 */
W_EXPORT void w_io_mem_init (w_io_mem_t *io, uint8_t *data, size_t size)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Obtains the base pointer to the data buffer.
 *
 * \param io In-memory I/O object.
 * \return   Base pointer to data buffer.
 */
static inline uint8_t* w_io_mem_data (w_io_mem_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline uint8_t*
w_io_mem_data (w_io_mem_t *io)
{
    w_assert (io);
    return io->data;
}

/*!
 * Obtains the size of the memory region.
 *
 * \param io In-memory I/O object.
 * \return   Size of the memory region.
 */
static inline size_t w_io_mem_size (w_io_mem_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline size_t w_io_mem_size (w_io_mem_t *io)
{
    w_assert (io);
    return io->size;
}

/*\}*/


/*---------------------------------------------------[ variant type ]-----*/

/*!
 * \defgroup wvariant Variant type
 * \addtogroup wvariant
 * \{
 *
 * Box-like container that can hold values of different types. Currently,
 * the following type are suported:
 *  - Strings (internally stored as a \ref w_buf_t).
 *  - Integral numbers (internally stored as \c long).
 *  - Floating point numbers (internally stored as \c double).
 *  - Booleans (internally stored as a \c bool).
 *  - Dictionaries (\ref w_dict_t).
 *  - Lists (\ref w_list_t).
 *  - Null value.
 *  - Invalid value (none of the above).
 *
 * The stored value can be mutated at any time, and the container will
 * handle freeing the memory used by string buffers, and calling
 * \ref w_obj_ref and \ref w_obj_unref as needed for object types (e.g.
 * lists).
 *
 * Most of the functionality is implemented as macros, therefore operations
 * on \ref w_variant_t tend to be fast, particularly when using base types.
 */

/*! Possible types to be stored in a \ref w_variant_t */
enum w_variant_type
{
    W_VARIANT_TYPE_INVALID, /*!< Invalid / no value    */
    W_VARIANT_TYPE_NULL,    /*!< Null                  */
    W_VARIANT_TYPE_STRING,  /*!< String                */
    W_VARIANT_TYPE_NUMBER,  /*!< Number                */
    W_VARIANT_TYPE_FLOAT,   /*!< Floating point number */
    W_VARIANT_TYPE_BOOL,    /*!< Boolean               */
    W_VARIANT_TYPE_DICT,    /*!< Dictionary            */
    W_VARIANT_TYPE_LIST,    /*!< List                  */
    W_VARIANT_TYPE_OBJECT,  /*!< Object                */

    /*
     * Note that this is used as a convenience that allows to
     * easily instantiate W_VARIANT_TYPE_STRING values from a
     * w_buf_t, but the actual type of the created value will
     * be always W_VARIANT_TYPE_STRING.
     */
    W_VARIANT_TYPE_BUFFER,
};

typedef enum w_variant_type w_variant_type_t;

union w_variant_value
{
    w_buf_t   stringbuf;
    long      number;
    double    fpnumber;
    bool      boolean;
    w_list_t *list;
    w_dict_t *dict;
    w_obj_t  *obj;
};

typedef union w_variant_value w_variant_value_t;

/*!
 * Variant box-container type.
 */
W_OBJ (w_variant_t)
{
    w_obj_t           parent;
    w_variant_type_t  type;
    w_variant_value_t value;
};

/*!
 * Static initilizer for variant values. This can be used to create
 * variant values in the stack. Note that it it still needed to use
 * \ref w_variant_clear before the variable goes out of scope, to
 * ensure that the reference counter of lists and dicts is properly
 * updated:
 *
 * \code
 * w_list_t *l = w_list_new (false);
 * w_variant_t v = W_VARIANT;
 * w_variant_set_list (&v, l);
 * w_obj_unref (l);
 * // use the variant...
 * w_variant_clear (&v);
 * \endcode
 *
 * Variants initialized this way will have invalid type until a
 * value of some type is stored in them.
 */
#define W_VARIANT                                            \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_INVALID,                \
        .value.stringbuf = W_BUF,                            \
     })

/*! Statically initilizes a variant with a null value. */
#define W_VARIANT_NULL                                       \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_NULL,                   \
        .value.stringbuf = W_BUF,                            \
     })

/*! Statically initilizes a variant with a string value. */
#define W_VARIANT_STRING(str)                                \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_STRING,                 \
        .value.stringbuf = { w_str_dup (str), strlen (str) } \
     })

/*! Statically initilizes a variant with a numeric value. */
#define W_VARIANT_NUMBER(num)                                \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_NUMBER,                 \
        .value.number    = (num),                            \
     })

/*! Statically initilizes a variant with a float value. */
#define W_VARIANT_FLOAT(num)                                 \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_FLOAT,                  \
        .value.fpnumber  = (num),                            \
     })

/*! Statically initilizes a variant with a boolean value. */
#define W_VARIANT_BOOL(val)                                  \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_BOOL,                   \
        .value.boolean   = (val),                            \
     })

/*! Statically initilizes a variant with a dictionary value. */
#define W_VARIANT_DICT(val)                                  \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_DICT,                   \
        .value.dict       w_obj_ref (val),                   \
     })

/*! Statically initilizes a variant with a list value. */
#define W_VARIANT_LIST(val)                                  \
    ((w_variant_t) {                                         \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_LIST,                   \
        .value.list      = w_obj_ref (val),                  \
     })

/*! Statically initilizes a variant with an object value. */
#define W_VARIANT_OBJECT(val)                                \
    ((w_variant_t*) {                                        \
        .parent          = W_OBJ_STATIC (w_variant_clear),   \
        .type            = W_VARIANT_OBJECT,                 \
        .value.obj       = w_obj_ref (val),                  \
     })

/*! Obtains the type of the value stored in a variant. */
static inline w_variant_type_t w_variant_type (const w_variant_t *v)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Obtains the type of the value stored in the variant.
 */
static inline w_variant_type_t
w_variant_type (const w_variant_t *v)
{
    w_assert (v);
    return v->type;
}

/*!
 * Create a new variant and assign it a value.
 * If you do not want to set an initial value, pass \ref
 * W_VARIANT_INVALID as \e type. Also, for most types it is needed to
 * pass the value itself:
 *
 * \code
 * variant = w_variant_new (W_VARIANT_STRING, "Hello");
 * \endcode
 *
 * Types who do not have an associated value (mainly nulls and invalid
 * values) do not need the extra function argument:
 *
 * \code
 * variant = w_variant_new (W_VARIANT_INVALID);
 * variant = w_variant_new (W_VARIANT_NULL);
 * \endcode
 */
W_EXPORT w_variant_t* w_variant_new (w_variant_type_t type, ...)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN;

/*! Clears a variant, setting it to invalid. */
W_EXPORT w_variant_t* w_variant_clear (w_variant_t *variant)
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));


#define W__VARIANTS_SIMPLE(F)                  \
    F (number, NUMBER, long,   value.number  ) \
    F (float,  FLOAT,  double, value.fpnumber) \
    F (bool,   BOOL,   bool,   value.boolean )

#define W__VARIANTS_OBJREF(F)                  \
    F (dict,   DICT,   w_dict_t*, value.dict ) \
    F (list,   LIST,   w_list_t*, value.list ) \
    F (object, OBJECT, w_obj_t*,  value.obj  )

#define W__VARIANTS_OTHERS(F) \
    F (string,  STRING,  , )  \
    F (null,    NULL,    , )  \
    F (invalid, INVALID, , )

#define W__VARIANTS(F)     \
    W__VARIANTS_SIMPLE (F) \
    W__VARIANTS_OBJREF (F) \
    W__VARIANTS_OTHERS (F)


#define W__VARIANT_IS(N, U, T, F)                            \
    static inline bool w_variant_is_ ## N (w_variant_t *v)   \
        W_FUNCTION_ATTR_WARN_UNUSED_RESULT                   \
        W_FUNCTION_ATTR_NOT_NULL ((1));                      \
    static inline bool w_variant_is_ ## N (w_variant_t *v) { \
        w_assert (v);                                        \
        return v->type == W_VARIANT_TYPE_ ## U;              \
    }

W__VARIANTS (W__VARIANT_IS)


#define W__VARIANT_GET_SIMPLE(N, U, T, F)                    \
    static inline T w_variant_ ## N (const w_variant_t *v)   \
        W_FUNCTION_ATTR_WARN_UNUSED_RESULT                   \
        W_FUNCTION_ATTR_NOT_NULL ((1));                      \
    static inline T w_variant_ ## N (const w_variant_t *v) { \
        w_assert (v);                                        \
        return v->F;                                         \
    }

W__VARIANTS_SIMPLE (W__VARIANT_GET_SIMPLE)
W__VARIANTS_OBJREF (W__VARIANT_GET_SIMPLE)


#define W__VARIANT_SET_SIMPLE(N, U, T, F)                              \
    static inline void w_variant_set_ ## N (w_variant_t *v, T value)   \
        W_FUNCTION_ATTR_NOT_NULL ((1));                                \
    static inline void w_variant_set_ ## N (w_variant_t *v, T value) { \
        w_assert (v);                                                  \
        w_variant_clear (v);                                           \
        v->type = W_VARIANT_TYPE_ ## U;                                \
        v->F = value;                                                  \
    }

W__VARIANTS_SIMPLE (W__VARIANT_SET_SIMPLE)


#define W__VARIANT_SET_OBJREF(N, U, T, F)                              \
    static inline void w_variant_set_ ## N (w_variant_t *v, T value)   \
        W_FUNCTION_ATTR_NOT_NULL ((1, 2));                             \
    static inline void w_variant_set_ ## N (w_variant_t *v, T value) { \
        w_assert (v);                                                  \
        w_variant_clear (v);                                           \
        v->type = W_VARIANT_TYPE_ ## U;                                \
        v->F = w_obj_ref (value);                                      \
    }

W__VARIANTS_OBJREF (W__VARIANT_SET_OBJREF)

/*!
 * \fn long w_variant_number (w_variant_t *v)
 *      Obtains the numeric value stored in a variant.
 *
 * \fn double w_variant_float (w_variant_t *v)
 *      Obtains the floating point numeric value stored in a variant.
 *
 * \fn bool w_variant_bool (w_variant_t *v)
 *      Obtains the boolean value stored in a variant.
 *
 * \fn w_dict_t* w_variant_dict (w_variant_t *v)
 *      Obtains the dictionary stored in a variant.
 *
 * \fn w_ list_t* w_variant_list (w_variant_t *v)
 *      Obtains the list stored in a variant.
 *
 * \fn w_obj_t* w_variant_object (w_variant_t *v)
 *      Obtains the object stored in a variant.
 */

/*! Obtains the string value stored in a variant. */
static inline char* w_variant_string (w_variant_t *v)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline char*
w_variant_string (w_variant_t *v)
{
    w_assert (v);
    return w_buf_str (&v->value.stringbuf);
}

/*! Obtains the string value stored in a variant, as a buffer. */
static inline const w_buf_t* w_variant_buffer (const w_variant_t *v)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL_RETURN
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline const w_buf_t*
w_variant_buffer (const w_variant_t *v)
{
    w_assert (v);
    return &v->value.stringbuf;
}


/*! Assigns buffer contents as string value to a variant, mutating it if needed. */
static inline void w_variant_set_buffer (w_variant_t *v, const w_buf_t *b)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

static inline void
w_variant_set_buffer (w_variant_t *v, const w_buf_t *b)
{
    w_assert (v);
    w_assert (b);
    w_variant_clear (v);
    v->type = W_VARIANT_TYPE_STRING;
    w_buf_append_buf (&v->value.stringbuf, b);
}

/*! Assigns a string value to a variant, mutating it if needed. */
static inline void w_variant_set_string (w_variant_t *v, const char *s)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

static inline void
w_variant_set_string (w_variant_t *v, const char *s)
{
    w_assert (v);
    w_assert (s);
    w_variant_clear (v);
    v->type = W_VARIANT_TYPE_STRING;
    w_buf_set_str (&v->value.stringbuf, s);
}

/*! Assigns \e null to a variant, mutating it if needed. */
static inline void w_variant_set_null (w_variant_t *v)
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline void
w_variant_set_null (w_variant_t *v)
{
    w_assert (v);
    w_variant_clear (v);
    v->type = W_VARIANT_TYPE_NULL;
}

/*! Makes a variant to be invalid. */
static inline void w_variant_set_invalid (w_variant_t *v)
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline void w_variant_set_invalid (w_variant_t *v)
{
    w_assert (v);
    w_variant_clear (v);  // Also sets the type to invalid.
}

/*!
 * \fn void w_variant_set_number (w_variant_t *v, long value)
 *      Assigns a numeric value to a variant, mutating it if needed.
 *
 * \fn void w_variant_set_float (w_variant_t *v, double value)
 *      Assigns a floating point number value to a variant, mutating it if needed.
 *
 * \fn void w_variant_set_bool (w_variant_t *v, bool value)
 *      Assigns a boolean value to a variant, mutating it if needed.
 *
 * \fn void w_variant_set_list (w_variant_t *v, w_list_t *value)
 *      Assigns a list to a variant, mutating it if needed.
 *
 * \fn void w_variant_set_dict (w_variant_t *v, w_dict_t *value)
 *      Assigns a dictionary to a variant, mutating it if needed.
 *
 * \fn void w_variant_set_object (w_variant_t *v, w_obj_t *value)
 *      Assigns an object to a variant, mutating it if needed.
 */

/*\}*/

/*---------------------------------------[ tnetstring serialization ]-----*/

/*!
 * \defgroup wtnets Serialization to/from tnetstrings.
 * \addtogroup wtnets
 * \{
 *
 * A more-or-less formal definition of tnetstrings can be found at
 * http://tnetstrings.org/
 */

W_EXPORT w_io_result_t w_tnetstr_dump (w_buf_t *buffer, const w_variant_t *value)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT w_io_result_t w_tnetstr_dump_null (w_buf_t *buffer)
    W_FUNCTION_ATTR_NOT_NULL ((1));

#define W__TNS_INLINE_WRITER_TYPES(F)                    \
    F (static inline, float,  double,          double)   \
    F (static inline, number, long,            long)     \
    F (static inline, list,   const w_list_t*, w_list_t) \
    F (static inline, dict,   const w_dict_t*, w_dict_t)

#define W__TNS_PARSER_TYPES(F)                           \
    F (W_EXPORT,      bool,   bool,            bool)     \
    F (W_EXPORT,      string, const char*,     w_buf_t)  \
    W__TNS_INLINE_WRITER_TYPES (F)

#define W__TNS_DUMP_TYPES(F)                             \
    F (W_EXPORT,      buffer, const w_buf_t*,  w_buf_t)  \
    W__TNS_PARSER_TYPES (F)

#define W__TNS_READ_TYPES(F) \
    F (, string, , w_buf_t ) \
    F (, bool  , , bool    ) \
    W__TNS_INLINE_WRITER_TYPES (F)


#define W__TNS_DECLARE_DUMPER(D, N, T, WT)          \
    W_EXPORT w_io_result_t                          \
    w_tnetstr_dump_ ## N (w_buf_t *buffer, T value) \
        W_FUNCTION_ATTR_NOT_NULL ();

W__TNS_DUMP_TYPES (W__TNS_DECLARE_DUMPER)


#define W__TNS_DECLARE_WRITER(D, N, T, WT)      \
    D w_io_result_t                             \
    w_tnetstr_write_ ## N (w_io_t *io, T value) \
        W_FUNCTION_ATTR_WARN_UNUSED_RESULT      \
        W_FUNCTION_ATTR_NOT_NULL ();

W__TNS_DUMP_TYPES (W__TNS_DECLARE_WRITER)


#define W__TNS_DECLARE_PARSER(D, N, T, WT)                                 \
    W_EXPORT bool w_tnetstr_parse_ ## N (const w_buf_t *buffer, WT *value) \
        W_FUNCTION_ATTR_WARN_UNUSED_RESULT                                 \
        W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W__TNS_PARSER_TYPES (W__TNS_DECLARE_PARSER)


#define W__TNS_DEFINE_INLINE_WRITER(D, N, T, WT)                  \
    D w_io_result_t w_tnetstr_write_ ## N (w_io_t *io, T value) { \
        w_assert (io);                                            \
        w_buf_t buf = W_BUF;                                      \
        W_IO_CHECK_BYTES (w_tnetstr_dump_ ## N (&buf, value),     \
                          return,                                 \
                          w_io_write (io, buf.data, buf.size)); }

W__TNS_INLINE_WRITER_TYPES (W__TNS_DEFINE_INLINE_WRITER)


W_EXPORT w_io_result_t w_tnetstr_write (w_io_t *io, const w_variant_t *value)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

W_EXPORT w_io_result_t w_tnetstr_write_null (w_io_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT w_variant_t* w_tnetstr_parse (const w_buf_t *buffer)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_tnetstr_parse_null (const w_buf_t *buffer)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

W_EXPORT bool w_tnetstr_read_to_buffer (w_io_t *io, w_buf_t *buffer)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

static inline w_variant_t* w_tnetstr_read (w_io_t *io)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));


static inline w_variant_t*
w_tnetstr_read (w_io_t *io)
{
    w_buf_t buf = W_BUF;
    w_variant_t *variant = NULL;
    w_assert (io);
    if (w_tnetstr_read_to_buffer (io, &buf)) goto E;
    variant = w_tnetstr_parse (&buf);
E:  w_buf_clear (&buf);
    return variant;
}


#define W__TNS_DEFINE_INLINE_READER(D, N, T, WT)         \
    static inline bool                                   \
    w_tnetstr_read_ ## N (w_io_t *io, WT *value)         \
        W_FUNCTION_ATTR_WARN_UNUSED_RESULT               \
        W_FUNCTION_ATTR_NOT_NULL ((1, 2));               \
    static inline bool                                   \
    w_tnetstr_read_ ## N (w_io_t *io, WT *value) {       \
        w_assert (io);                                   \
        w_assert (value);                                \
        w_buf_t buf = W_BUF;                             \
        if (w_tnetstr_read_to_buffer (io, &buf)) goto E; \
        if (w_tnetstr_parse_ ## N (&buf, value)) goto E; \
        w_buf_clear (&buf); return false;                \
    E:  w_buf_clear (&buf); return true;                 \
    }

W__TNS_READ_TYPES (W__TNS_DEFINE_INLINE_READER)

/*\}*/

/*------------------------------------------------------[ metatypes ]-----*/


/*!
 * \defgroup wtype Meta types
 * \addtogroup wtype
 * \{
 */

enum w_meta_type {
    W_META_TYPE_NONE = 0,
    W_META_TYPE_I8,
    W_META_TYPE_U8,
    W_META_TYPE_I16,
    W_META_TYPE_U16,
    W_META_TYPE_I32,
    W_META_TYPE_U32,
    W_META_TYPE_I64,
    W_META_TYPE_U64,
    W_META_TYPE_FLT,
    W_META_TYPE_DBL,
    W_META_TYPE_BOOL,
    W_META_TYPE_STR,
    W_META_TYPE_REG,
};

typedef enum w_meta_type w_meta_type_t;

typedef struct w_meta_item w_meta_item_t;

struct w_meta_item
{
    const char          *name; /*!< Name. */
    w_meta_type_t        type; /*!< Type. */
    unsigned long        alen; /*!< Array length. Zero if single-item. */
    ptrdiff_t            voff; /*!< Value offset. */
    const w_meta_item_t *mref; /*!< Reference to other meta info. */
};

typedef const w_meta_item_t w_meta_t[];


#define W_META(name)          { name, W_META_TYPE_NONE, 0, 0, NULL }
#define W_META_END            { NULL, W_META_TYPE_NONE, 0, 0, NULL }

#define W_META_I8(s, m)       { #m, W_META_TYPE_I8,    0, w_offsetof (s, m), NULL }
#define W_META_I8_V(s, m, l)  { #m, W_META_TYPE_I8,  (l), w_offsetof (s, m), NULL }
#define W_META_I16(s, m)      { #m, W_META_TYPE_I16,   0, w_offsetof (s, m), NULL }
#define W_META_I16_V(s, m, l) { #m, W_META_TYPE_I16, (l), w_offsetof (s, m), NULL }
#define W_META_I32(s, m)      { #m, W_META_TYPE_I32,   0, w_offsetof (s, m), NULL }
#define W_META_I32_V(s, m, l) { #m, W_META_TYPE_I32, (l), w_offsetof (s, m), NULL }
#define W_META_I64(s, m)      { #m, W_META_TYPE_I64,   0, w_offsetof (s, m), NULL }
#define W_META_I64_V(s, m, l) { #m, W_META_TYPE_I64, (l), w_offsetof (s, m), NULL }
#define W_META_U8(s, m)       { #m, W_META_TYPE_U8,    0, w_offsetof (s, m), NULL }
#define W_META_U8_V(s, m, l)  { #m, W_META_TYPE_U8,  (l), w_offsetof (s, m), NULL }
#define W_META_U16(s, m)      { #m, W_META_TYPE_U16,   0, w_offsetof (s, m), NULL }
#define W_META_U16_V(s, m, l) { #m, W_META_TYPE_U16, (l), w_offsetof (s, m), NULL }
#define W_META_U32(s, m)      { #m, W_META_TYPE_U32,   0, w_offsetof (s, m), NULL }
#define W_META_U32_V(s, m, l) { #m, W_META_TYPE_U32, (l), w_offsetof (s, m), NULL }
#define W_META_U64(s, m)      { #m, W_META_TYPE_U64,   0, w_offsetof (s, m), NULL }
#define W_META_U64_V(s, m, l) { #m, W_META_TYPE_U64, (l), w_offsetof (s, m), NULL }
#define W_META_FLT(s, m)      { #m, W_META_TYPE_FLT,   0, w_offsetof (s, m), NULL }
#define W_META_FLT_V(s, m, l) { #m, W_META_TYPE_FLT, (l), w_offsetof (s, m), NULL }
#define W_META_DBL(s, m)      { #m, W_META_TYPE_DBL,   0, w_offsetof (s, m), NULL }
#define W_META_DBL_V(s, m, l) { #m, W_META_TYPE_DBL, (l), w_offsetof (s, m), NULL }
#define W_META_BOOL(s, m)     { #m, W_META_TYPE_BOOL,  0, w_offsetof (s, m), NULL }
#define W_META_BOOL_V(s, m, l){ #m, W_META_TYPE_BOOL,(l), w_offsetof (s, m), NULL }
#define W_META_STR(s, m)      { #m, W_META_TYPE_STR,   0, w_offsetof (s, m), NULL }
#define W_META_STR_V(s, m, l) { #m, W_META_TYPE_STR, (l), w_offsetof (s, m), NULL }
#define W_META_REG(s, m, p)   { #m, W_META_TYPE_REG,   0, w_offsetof (s, m), (p)  }
#define W_META_REG_V(s,m,p,l) { #m, W_META_TYPE_REG, (l), w_offsetof (s, m), (p)  }

#define w_meta_desc_name(m)     ((m)[0].name)
#define w_meta_desc_items(m)    ((m) + 1)
#define w_meta_item_next(i)     ((i) + 1)
#define w_meta_item_is_valid(i) ((i)->name != NULL)

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
enum w_cfg_type
{
	W_CFG_END    = W_VARIANT_TYPE_NULL,    /*!< Marks end of parameter lists. */
	W_CFG_NONE   = W_VARIANT_TYPE_INVALID, /*!< Invalid node.                 */
	W_CFG_STRING = W_VARIANT_TYPE_STRING,  /*!< Node containing a string.     */
	W_CFG_NUMBER = W_VARIANT_TYPE_FLOAT,   /*!< Node containing a number.     */
	W_CFG_NODE   = W_VARIANT_TYPE_DICT,    /*!< Node containing a subnode.    */
};

typedef enum w_cfg_type w_cfg_type_t;

/*!
 * Create a new configuration object.
 */
#define w_cfg_new( ) \
    w_dict_new (true)

/*!
 * Checks whether a key exists in a configuration object.
 */
W_EXPORT bool w_cfg_has (const w_cfg_t *cf, const char *key)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Deletes a key from a configuration object.
 */
W_EXPORT bool w_cfg_del (w_cfg_t *cf, const char *key)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

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
W_EXPORT bool w_cfg_set (w_cfg_t *cf, ...)
    W_FUNCTION_ATTR_NOT_NULL ((1));

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
W_EXPORT bool w_cfg_get (const w_cfg_t *cf, ...)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Obtain the type of a configuration object node.
 */
W_EXPORT w_cfg_type_t w_cfg_type (const w_cfg_t *cf, const char *key)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Dump configuration to a stream.
 * \param cf     Configuration object.
 * \param output Output stream where to write.
 * \return       Whether writing was successful.
 */
W_EXPORT w_io_result_t w_cfg_dump (const w_cfg_t *cf, w_io_t *output)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Load configuration from a stream.
 * \param input  Input stream
 * \param msg    Pointer to where to store an error message, if there is one.
 * \return       Valid configuration object or \c NULL if there is some error.
 */
W_EXPORT w_cfg_t* w_cfg_load (w_io_t *input, char **msg)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Dump configuration to a file.
 * \param cf   Configuration object.
 * \param path Path to a file.
 * \return     Whether writing was successful.
 */
W_EXPORT w_io_result_t w_cfg_dump_file (const w_cfg_t *cf, const char *path)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Load configuration from a file.
 * \param path Path to a file.
 * \param msg  Pointer to where to store an error message, if there is one.
 * \return     Valid configuration object or \c NULL if there is some error.
 */
W_EXPORT w_cfg_t* w_cfg_load_file (const char *path, char **msg)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

#define w_cfg_set_string(cf, key, val) \
	w_cfg_set ((cf), W_CFG_STRING, key, val, W_CFG_END)

#define w_cfg_set_number(cf, key, val) \
	w_cfg_set ((cf), W_CFG_NUMBER, key, val, W_CFG_END)

#define w_cfg_set_node(cf, key, val) \
	w_cfg_set ((cf), W_CFG_NODE, key, val, W_CFG_END)

#define _W_G(func_name, conf_type, c_type)             \
	static inline c_type w_cfg_get_ ## func_name        \
	(w_cfg_t *cf, const char *key, c_type defval)        \
	    W_FUNCTION_ATTR_WARN_UNUSED_RESULT                \
	    W_FUNCTION_ATTR_NOT_NULL ((1, 2));                 \
	static inline c_type w_cfg_get_ ## func_name            \
	(w_cfg_t *cf, const char *key, c_type defval)            \
	{ c_type value;                                           \
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

/*---------------------------------------------------------[ events ]-----*/

/*!
 * \defgroup wev Event handling
 * \addtogroup wev
 * \{
 */

W_OBJ_DECL (w_event_t);
W_OBJ_DECL (w_event_loop_t);

enum w_event_type
{
    W_EVENT_TIMER,
    W_EVENT_SIGNAL,
    W_EVENT_IO,
    W_EVENT_FD,
    W_EVENT_IDLE,
};
typedef enum w_event_type w_event_type_t;


enum w_event_flags {
    W_EVENT_IN      = 1 << 0,
    W_EVENT_OUT     = 1 << 1,
    W_EVENT_ONESHOT = 1 << 2,
    W_EVENT_REPEAT  = 1 << 3,
};
typedef enum w_event_flags w_event_flags_t;

typedef double w_timestamp_t;


typedef bool (*w_event_callback_t) (w_event_loop_t*, w_event_t*);


/*!
 * Returns the current time as used by the event system.
 * Whenever possible, use \ref w_loop_now instead, which is faster.
 */
w_timestamp_t w_timestamp_now (void)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT;


W_OBJ_DEF (w_event_t)
{
    w_obj_t            parent;
    w_event_type_t     type;
    w_event_callback_t callback;
    w_event_flags_t    flags;

    /* Value in use depends on the value of "type" */
    union {
        int            fd;     /* W_EVENT_FD     */
        w_io_t        *io;     /* W_EVENT_IO     */
        int            signum; /* W_EVENT_SIGNAL */
        w_timestamp_t  time;   /* W_EVENT_TIMER  */
    };
};


static inline w_event_type_t w_event_get_type (const w_event_t *event)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline w_event_type_t
w_event_get_type (const w_event_t *event)
{
    w_assert (event);
    return event->type;
}


w_event_t* w_event_new (w_event_type_t     type,
                        w_event_callback_t callback,
                        ...)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((2));


W_OBJ_DEF (w_event_loop_t)
{
    w_obj_t       parent;
    bool          running;
    w_list_t     *events;
    w_list_t     *idle_events;
    w_timestamp_t now;
};


/*!
 * Get the current time from the event loop.
 * Actually, this returns the time when the last event started to be
 * handled, which is usually enough for most operation, while still being
 * much faster than \ref w_timestamp_now.
 *
 * \return A \ref w_timestamp_t value.
 */
static inline w_timestamp_t w_event_loop_now (const w_event_loop_t *loop)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline w_timestamp_t
w_event_loop_now (const w_event_loop_t *loop)
{
    w_assert (loop);
    return loop->now;
}

/*!
 * Checks whether an event loop is running.
 */
static inline bool w_event_loop_is_running (const w_event_loop_t *loop)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT
    W_FUNCTION_ATTR_NOT_NULL ((1));

static inline bool
w_event_loop_is_running (const w_event_loop_t *loop)
{
    w_assert (loop);
    return loop->running;
}

/*!
 * Creates a new event loop. Event loops can be used to listen for
 * notifications on signals, timers, and input/output streams (both
 * low-level Unix file descriptors and \ref w_io_t instances).
 *
 * For each platform, the most efficient mechanism available is used:
 *
 *  - \c epoll() on Linux.
 *  - \c kqueue() on BSD.
 *
 * This function may return \c NULL if the platform is not supported,
 * or an error happened during initialization of the platform dependant
 * mechanism.
 *
 * \todo Fall-back based on \c poll(2) or \c select(2).
 */
w_event_loop_t* w_event_loop_new (void)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT;

/*!
 * Runs an event loop indefinitely, until it is stopped. The function
 * will continously keep handling events, and will not return until
 * \ref w_event_loop_stop is used.
 * \return Whether the event loop exited due to errors.
 */
bool w_event_loop_run (w_event_loop_t *loop)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Stops an event loop. Note that already-received events will still
 * be handled before \ref w_event_loop_run returns.
 */
void w_event_loop_stop (w_event_loop_t *loop)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*!
 * Adds an event to the event loop.
 * \return Whether there was an error when trying to add the event.
 */
bool w_event_loop_add (w_event_loop_t *loop, w_event_t *event)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

/*!
 * Removes an event from an event loop.
 * \return Whether there was an error when trying to remove the event.
 */
bool w_event_loop_del (w_event_loop_t *loop, w_event_t *event)
    W_FUNCTION_ATTR_NOT_NULL ((1, 2));

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
W_EXPORT bool w_tty_size (unsigned *cols, unsigned *rows);

/*!
 * Obtains the width of a row of the controlling terminal.
 *
 * \return Terminal width.
 */
W_EXPORT unsigned w_tty_cols (void)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT;

/*!
 * Obtains the height of the the controlling terminal.
 *
 * \return Terminal height.
 */
W_EXPORT unsigned w_tty_rows (void)
    W_FUNCTION_ATTR_WARN_UNUSED_RESULT;


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
W_EXPORT bool w_tty_size_notify (w_tty_notify_fun_t function,
                                 void              *context)
    W_FUNCTION_ATTR_NOT_NULL ((1));

/*\}*/

#endif /* !__wheel_h__ */

