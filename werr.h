/*
 * werr.h
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __werr_h__
#define __werr_h__

#include "wdef.h"
#include <stdarg.h>

W_EXPORT void  w_die(const char *fmt, ...);
W_EXPORT void w_vdie(const char *fmt, va_list al);


#ifdef _DEBUG
# include <assert.h>
# define w_assert      assert
#else
# define w_assert(_s)  ((void)0)
#endif

#ifdef _DPRINTF
# define w_dprintf(_x) __w_dprintf _x
W_EXPORT void __w_dprintf(const char *fmt, ...);
#else
# define w_dprintf(_x) ((void)0)
#endif

#endif /* !__werr_h__ */

