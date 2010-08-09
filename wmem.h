/*
 * wmem.h
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wmem_h__
#define __wmem_h__

#include "wdef.h"
#include "werr.h"
#include <stddef.h>
#include <stdlib.h>

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


#endif /* !__wmem_h__ */

