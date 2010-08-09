/*
 * wio.c
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wio.h"
#include "wdef.h"
#include "wmem.h"

static int _w_null_getc(void *ctx);
static int _w_null_putc(int c, void *ctx);


enum w_io_flag
{
	/* Private flags: two lower bytes. */
	_W_IO_READ   = 0x0001, /* Can read from file. */
	_W_IO_WRITE  = 0x0002, /* Can write to file. */
	_W_IO_ERROR  = 0x0004, /* Error flag set. */
	_W_IO_EOF    = 0x0008, /* End-Of-File reached. */
	_W_IO_STATIC = 0x0010, /* Static buffer. */

	_W_IO_PRIVATE_MASK = 0x0000ffff,
};

typedef enum w_io_flag w_io_flag_t;


struct w_io
{
	w_putc_fun_t putc;    /* Putchar, supplied by user. */
	w_getc_fun_t getc;    /* Getchar, supplied by user. */
	w_fini_fun_t finf;    /* Close function. */
	void        *ctx;     /* Context, passed to above functions. */

	w_io_flag_t  flags;   /* Flags for this I/O object. */

	/* Buffering stuff follows. */
	int          putback; /* Ungetted character. */
	unsigned     bufpos;  /* Position in buffer. */
	unsigned     bufchs;  /* Bytes in buffer. */
	unsigned     buflen;  /* Buffer length. */
	char        *buf;     /* Data buffer. */
};



#ifndef _W_IO_BUFFER_SIZE
#define _W_IO_BUFFER_SIZE 1024
#endif /* !_W_IO_BUFFER_SIZE */


static char _W_ERR_buffer[_W_IO_BUFFER_SIZE];
static char _W_OUT_buffer[_W_IO_BUFFER_SIZE];
static char _W_IN_buffer [_W_IO_BUFFER_SIZE];


static w_io_t _W_ERR =
{
	NULL, /* TODO fix those. */
	_w_null_getc,
	NULL,
	(void*) 2,

	_W_IO_STATIC | _W_IO_WRITE, 0, 0, 0, _W_IO_BUFFER_SIZE, _W_ERR_buffer
};
w_io_t *W_ERR = &_W_ERR;


static w_io_t _W_OUT =
{
	NULL,
	_w_null_getc,
	NULL,
	(void*) 1,

	_W_IO_STATIC | _W_IO_WRITE, 0, 0, 0, _W_IO_BUFFER_SIZE, _W_OUT_buffer
};
w_io_t *W_OUT = &_W_OUT;


static w_io_t _W_IN =
{
	_w_null_putc,
	NULL,
	NULL,
	(void*) 0,

	_W_IO_STATIC | _W_IO_READ, 0, 0, 0, _W_IO_BUFFER_SIZE, _W_IN_buffer
};
w_io_t *W_IN = &_W_IN;


static int
_w_null_getc(void *ctx)
{
	w_unused(ctx);
	return W_IO_EOF;
}


static int
_w_null_putc(int c, void *ctx)
{
	w_unused(ctx);
	return c;
}


w_io_t*
w_io_new(w_getc_fun_t getfn, w_putc_fun_t putfn, w_fini_fun_t finf, void *ctx)
{
	w_io_t *io = w_new(w_io_t);
	io->putback = W_IO_NONE;
	io->getc = _w_null_getc;
	io->putc = _w_null_putc;
	io->finf = finf;
	io->ctx  = ctx;

	if (getfn != NULL) {
		io->flags |= _W_IO_READ;
		io->getc   = getfn;
	}
	if (putfn != NULL) {
		io->flags |= _W_IO_WRITE;
		io->putc   = putfn;
	}

	return io;
}


int
w_io_close(w_io_t *io)
{
	int ret;
	w_assert(io != NULL);

	ret = w_io_flush(io);
	if (io->finf != NULL) (*io->finf)(io->ctx);
	w_free(io);
	return ret;
}



