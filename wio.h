/*
 * wio.h
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wio_h__
#define __wio_h__

#include "wdef.h"
#include <stddef.h>
#include <stdarg.h>

#define _K(x) (0xff + (x))

enum
{
	W_IO_EOF  = _K(1),
	W_IO_OK   = _K(2),
	W_IO_ERR  = _K(3),
	W_IO_NONE = _K(4),

	/* Public flags: two higher bytes. */
	W_IO_BUFF = 0x00010000,

	W_IO_PUBLIC_MASK = 0xffff0000,
};

#undef _K

typedef struct w_io w_io_t;

typedef int (*w_fini_fun_t)(void *context);
typedef int (*w_getc_fun_t)(void *context);
typedef int (*w_putc_fun_t)(int c, void *context);


W_EXPORT extern w_io_t *W_ERR;
W_EXPORT extern w_io_t *W_OUT;
W_EXPORT extern w_io_t *W_IN;


W_EXPORT w_io_t* w_io_new(
		w_getc_fun_t getf,
		w_putc_fun_t putf,
		w_fini_fun_t finf,
		void *context);

W_EXPORT int w_io_getc(w_io_t *io);
W_EXPORT int w_io_putc(w_io_t *io, int c);
W_EXPORT int w_io_read(w_io_t *io, void *buf, size_t sz);
W_EXPORT int w_io_write(w_io_t *io, const void *buf, size_t sz);
W_EXPORT int w_io_close(w_io_t *io);
W_EXPORT int w_io_flush(w_io_t *io);

W_EXPORT int w_io_putf (w_io_t *io, const char *fmt, ...);
W_EXPORT int w_io_putfv(w_io_t *io, const char *fmt, va_list args);

W_EXPORT int w_io_getf (w_io_t *io, const char *fmt, ...);
W_EXPORT int w_io_getfv(w_io_t *io, const char *fmt, va_list args);

#endif /* !__wio_h__ */

