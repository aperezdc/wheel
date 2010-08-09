/*
 * wstr.h
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wstr_h__
#define __wstr_h__

#include "wmem.h"
#include "wdef.h"
#include <stddef.h>
#include <string.h>
#include <stdarg.h>


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


#endif /* !__wstr_h__ */

