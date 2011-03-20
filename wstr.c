/*
 * wstr.c
 * Copyright (C) 2010-2011 Adrian Perez <aperez@igalia.com>
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#define _GNU_SOURCE /* Required for HUGE_VAL macros */
#include "wheel.h"
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>


char*
w_strfmtv(const char *fmt, va_list argl)
{
	char *ret;
	int len;
#if defined(__APPLE__) && defined(__MACH__)
# define args_copy argl
#else
	va_list args_copy;
	va_copy(args_copy, argl);
#endif
	w_assert(fmt != NULL);

	len = vsnprintf(NULL, 0, fmt, argl);
	ret = w_alloc(char, ++len);
	vsnprintf(ret, len, fmt, args_copy);

	return ret;
}


char*
w_strfmt(const char *fmt, ...)
{
	char *ret;
	va_list argl;
	w_assert(fmt != NULL);

	va_start(argl, fmt);
	ret = w_strfmtv(fmt, argl);
	va_end(argl);
	return ret;
}


unsigned
w_hashstr(const char *str)
{
	register unsigned ret = 0;
	register unsigned ctr = 0;
	w_assert(str != NULL);

	while (*str) {
		ret ^= *str++ << ctr;
		ctr  = (ctr + 1) % sizeof(void*);
	}
	return ret;
}


unsigned
w_hashstrn(const char *str, size_t len)
{
	register unsigned ret = 0;
	register unsigned ctr = 0;
	w_assert(str != NULL);

	while (len-- && *str) {
		ret ^= *str++ << ctr;
		ctr  = (ctr + 1) % sizeof(void*);
	}
	return ret;
}


wbool
w_str_bool(const char *str, wbool *opt)
{
	w_assert(str != NULL);
	w_assert(opt != NULL);

	switch (strlen(str)) {
		case 1: /* One character length. */
			switch (*str) {
				case '0': case 'f': case 'F': case 'n': case 'N':
					*opt = W_NO;
					break;
				case '1': case 't': case 'T': case 'y': case 'Y':
					*opt = W_YES;
					break;
				default:
					return W_NO;
			}
			return W_YES;

		/* FIXME Tests below may be speed up. */

		case 2: /* Two characters: "no", "ok" */
			if (!w_strcasecmp("no", str)) return (*opt = W_NO , W_YES);
			if (!w_strcasecmp("ok", str)) return (*opt = W_YES, W_YES);
			return W_NO;
		case 3: /* Three characters: "yes", "nah", "nop" */
			if (!w_strcasecmp("yes", str)) return (*opt = W_YES, W_YES);
			if (!w_strcasecmp("nop", str)) return (*opt = W_NO , W_YES);
			if (!w_strcasecmp("nah", str)) return (*opt = W_NO , W_YES);
			return W_NO;
		case 4: /* Four characters: "yeah", "okay", "nope", true */
			if (!w_strcasecmp("true", str)) return (*opt = W_YES, W_YES);
			if (!w_strcasecmp("yeah", str)) return (*opt = W_YES, W_YES);
			if (!w_strcasecmp("okay", str)) return (*opt = W_YES, W_YES);
			if (!w_strcasecmp("nope", str)) return (*opt = W_NO , W_YES);
			return W_NO;
		case 5: /* Five characters: "false". */
			if (!w_strcasecmp("false", str)) return (*opt = W_NO, W_YES);
			return W_NO;
	}

	return W_NO;
}



wbool
w_str_int(const char *str, int *val)
{
	long v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtol(str, &chkstr, 0);
	if ((*str != '\0') && (*chkstr == '\0') && (v <= INT_MAX) && (v >= INT_MIN)
	    && !(((v == LONG_MIN) || (v == LONG_MAX)) && (errno == ERANGE)))
    {
        return (*val = v, W_YES);
    }

	return W_NO;
}


wbool
w_str_uint(const char *str, unsigned *val)
{
	unsigned long v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtoull(str, &chkstr, 0);
	if ((*str != '\0') && (*chkstr == '\0') && (v <= UINT_MAX) &&
			!((v == ULONG_MAX) && (errno == ERANGE)))
		return (*val = v, W_YES);

	return W_NO;
}


wbool
w_str_long(const char *str, long *val)
{
	long v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtol(str, &chkstr, 0);
	if ((*str != '\0') && (*chkstr == '\0') &&
	        !((v == LONG_MAX || v == LONG_MIN) && (errno == ERANGE)))
		return (*val = v, W_YES);

	return W_NO;
}


wbool
w_str_ulong(const char *str, unsigned long *val)
{
	unsigned long v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtoul(str, &chkstr, 0);
	if ((*str != '\0') && (*chkstr == '\0') &&
	        !(v == ULONG_MAX && errno == ERANGE))
		return (*val = v, W_YES);

	return W_NO;
}


#ifdef HUGE_VALF
wbool
w_str_float(const char *str, float *val)
{
	float v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtof(str, &chkstr);
	if (((v == HUGE_VALF) || (v == -HUGE_VALF)) && (errno == ERANGE))
		return W_NO;

	return (*val = v, W_YES);
}
#endif /* HUGE_VALF */


#ifdef HUGE_VAL
wbool
w_str_double(const char *str, double *val)
{
	double v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtod(str, &chkstr);
	if (((v == HUGE_VAL) || (v == -HUGE_VAL)) && (errno == ERANGE))
		return W_NO;

	return (*val = v, W_YES);
}
#endif /* HUGE_VAL */


