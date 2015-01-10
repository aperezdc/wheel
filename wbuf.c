/*
 * wbuf.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * Buffers
 * =======
 *
 * Buffers provide a variable-length area of memory in which data may be
 * held and manipulated. Contained data is not interpreted, and length of
 * it is tracked, so it is possible to add null bytes to a buffer.
 *
 * Allocating a buffer is done in the stack using the :macro:`W_BUF` macro.
 * to initialize it. After initialization, all buffer functions can be used,
 * and when the buffer is not needed anymore, its contents can be freed using
 * :func:`w_buf_clear()`.
 *
 * Usage
 * -----
 *
 * .. code-block:: c
 *
 *      // Initialize the buffer.
 *      w_buf_t b = W_BUF;
 *
 *      // Append some string pieces.
 *      w_buf_append_str (&b, "Too much work ");
 *      w_buf_append_str (&b, "and no joy makes");
 *      w_buf_append_str (&b, " Jack a dull boy");
 *
 *      // Buffer contents can be printed directly using the $B format.
 *      w_print ("$B\n", &b);
 *
 *      // Free the memory used by the contents of the buffer.
 *      w_buf_clear (&b);
 */

/**
 * Types
 * -----
 */

/*~t w_buf_t
 *
 * Buffer type.
 */

/**
 * Macros
 * ------
 */

/*~M W_BUF
 *
 * Initializer for buffers. It can be used to initialize buffers directly on
 * the stack:
 *
 * .. code-block:: c
 *
 *      w_buf_t buffer = W_BUF;
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"

#ifndef W_BUF_CHUNK_SIZE
#define W_BUF_CHUNK_SIZE 512
#endif /* !W_BUF_CHUNK_SIZE */


static inline void
_buf_resize (w_buf_t *buf, size_t size)
{
    if (size) {
        size_t nsz = W_BUF_CHUNK_SIZE * ((size / W_BUF_CHUNK_SIZE) + 1);
        if (nsz < size) {
            nsz = size;
        }
        if (nsz != buf->alloc) {
            buf->data  = w_resize (buf->data, char, nsz + 1);
            buf->alloc = nsz;
        }
    }
    else {
        if (buf->data) {
            w_free (buf->data);
        }
        buf->alloc = 0;
    }
}


static inline void
_buf_xresize (w_buf_t *buf, size_t size)
{
    _buf_resize (buf, size);
    buf->size = size;
}


/*~f void w_buf_resize (w_buf_t* buffer, size_t size)
 *
 * Adjust the size of a buffer keeping contents.
 * This is mostly useful for trimming contents, when shrinking the buffer.
 * When a buffer grows, random data is liklely to appear at the end.
 *
 * :param buffer: A w_buf_t buffer
 * :param size: size New size of the buffer
 */
void
w_buf_resize (w_buf_t *buf, size_t size)
{
    w_assert (buf);
    _buf_xresize (buf, size);
}


/*~f void w_buf_set_str (w_buf_t *buffer, const char *string)
 *
 * Set the contents of a buffer to a C string.
 *
 * :param buffer: A w_buf_t buffer
 * :param string: String to set the buffer to.
 */
void
w_buf_set_str (w_buf_t *buf, const char *str)
{
    w_assert (buf);
    w_assert (str);

    size_t slen = strlen (str);
    _buf_xresize (buf, slen);
    memcpy (buf->data, str, slen);
}


/*~f void w_buf_append_mem (w_buf_t *buffer, const void *address, size_t length)
 *
 * Appends the contents of a chunk of memory of `length` bytes starting at
 * `address` to a `buffer`.
 *
 * :param buffer: A w_buf_t buffer.
 * :param address: Pointer to the memory block of memory.
 * :param length: Length of the memory block.
 */
void
w_buf_append_mem (w_buf_t *buf, const void *ptr, size_t len)
{
    w_assert (buf);
    w_assert (ptr);

    size_t bsize = buf->size;
    _buf_xresize (buf, bsize + len);
    memcpy (buf->data + bsize, ptr, len);
}


/*~f void w_buf_append_str (w_buf_t *buffer, const char *string)
 *
 * Appends a `string` to a `buffer`.
 */
void
w_buf_append_str (w_buf_t *buf, const char *str)
{
    size_t bsize;
    size_t slen;

    w_assert (buf);
    w_assert (str);

    bsize = buf->size;
    slen = strlen (str);
    _buf_xresize (buf, bsize + slen);
    memcpy (buf->data + bsize, str, slen);
}


/*~f void w_buf_append_char (w_buf_t *buffer, int character)
 *
 * Appends a `character` to a `buffer`.
 */
void
w_buf_append_char (w_buf_t *buf, int chr)
{
    w_assert (buf);
    _buf_xresize (buf, buf->size + 1);
    buf->data[buf->size - 1] = chr;
}


/*~f void w_buf_append_buf (w_buf_t *buffer, const w_buf_t *other)
 *
 * Appends the contents of `other` buffer to another `buffer`.
 */
void
w_buf_append_buf (w_buf_t *buf, const w_buf_t *src)
{
    w_assert (buf);
    w_assert (src);

    size_t bsize = buf->size;
    _buf_xresize (buf, bsize + src->size);
    memcpy (buf->data + bsize, src->data, src->size);
}


/*~f char* w_buf_str (w_buf_t *buffer)
 *
 * Obtains the contents of a `buffer` as a ``NULL``-terminated C string.
 *
 * .. warning:: If the buffer contains embedded null characters, functions
 *    like ``strlen()`` will not report the full length of the buffer.
 *
 * The returned pointer is owned by the `buffer`, and there two ways in which
 * the memory region can be freed:
 *
 * - Clearing the `buffer` with :func:`w_buf_clear()`. The returned
 *   pointer will be invalid afterwards.
 *
 * - Calling :func:`w_free()` on the returned pointer. The `buffer`
 *   will be invalid and must not be used afterwards.
 *
 * The second way is useful to assemble a string which is returned from a
 * function, for example:
 *
 * .. code-block:: c
 *
 *    char* concat_strings (const char *s, ...)
 *    {
 *      w_buf_t buffer = W_BUF;
 *      w_buf_set_str (&buffer, s);
 *
 *      va_list args;
 *      va_start (args, s);
 *      while ((s = va_args (args, const char*)))
 *        w_buf_append_str (&buffer, s);
 *      va_end (args);
 *
 *      return w_buf_str (&buffer);
 *    }
 */
char*
w_buf_str (w_buf_t *buf)
{
    w_assert (buf);

    if (!buf->size) {
        _buf_xresize (buf, 1);
        buf->size = 0;
    }
    buf->data[buf->size] = '\0';
    return buf->data;
}


/*~f void w_buf_clear (w_buf_t *buffer)
 *
 * Clears a `buffer`, freeing any used memory.
 */
void
w_buf_clear (w_buf_t *buf)
{
    w_assert (buf);
    _buf_xresize (buf, 0);
}


/*~f w_io_result_t w_buf_format (w_buf_t *buffer, const char *format, ...)
 *
 * Appends text with a given `format` into a `buffer`, consuming additional
 * arguments as needed by the `format`.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */
w_io_result_t
w_buf_format (w_buf_t *buf, const char *fmt, ...)
{
    w_assert (buf);
    w_assert (fmt);

    w_io_buf_t io;
    w_io_buf_init (&io, buf, true);

    va_list al;
    va_start (al, fmt);

    w_io_result_t r = w_io_formatv ((w_io_t*) &io, fmt, al);

    va_end (al);
    return r;
}


/*~f w_io_result_t w_buf_formatv (w_buf_t *buffer, const char *format, va_list arguments)
 *
 * Appends text with a given `format` into a `buffer`, consuming additional
 * `arguments` as needed by the `format`.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */
w_io_result_t
w_buf_formatv (w_buf_t *buf, const char *fmt, va_list args)
{
    w_assert (buf);
    w_assert (fmt);

    w_io_buf_t io;
    w_io_buf_init (&io, buf, true);

    return w_io_formatv ((w_io_t*) &io, fmt, args);
}


/*~f bool w_buf_is_empty (const w_buf_t *buffer)
 *
 * Checks whether a `buffer` is empty.
 */

/*~f size_t w_buf_size (const w_buf_t *buffer)
 *
 * Obtains the size of a `buffer`.
 */

/*~f char* w_buf_data (w_buf_t *buffer)
 *
 * Obtains a pointer to the internal data stored by a `buffer`.
 *
 * .. warning:: The returned value may be ``NULL`` when the buffer is empty.
 */

/*~f const char* w_buf_const_data (const w_buf_t *buffer)
 *
 * Obtains a pointer to the internal data stored by a `buffer`, returning it
 * as a ``const`` pointer. This may be used instead of :func:`w_buf_data()`
 * when the data is not going to be modified.
 *
 * .. warning:: The returned value may be ``NULL`` when the buffer is empty.
 */
