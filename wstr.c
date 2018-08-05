/*
 * wstr.c
 * Copyright (C) 2010-2015 Adrian Perez <aperez@igalia.com>
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

#ifdef W_CONF_SIPHASH
static inline int siphash (const uint8_t *in, size_t inlen,
                           const uint8_t *k,
                           uint8_t *out, const size_t outlen);
#include "siphash/siphash.c"
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#endif /* W_CONF_SIPHASH */

/*~f char* w_cstr_format (const char *format, ...)
 *
 * Create a C (``\0``-terminated) string with a given `format`, consuming
 * additional arguments as needed by the `format`.
 *
 * The returned strings must be freed by the caller.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */
char*
w_cstr_format (const char *format, ...)
{
    w_assert (format);

    w_io_buf_t buffer_stream;
    w_io_buf_init (&buffer_stream, NULL, false);

    va_list args;
    va_start (args, format);
    W_IO_NORESULT (w_io_formatv (&buffer_stream.parent, format, args));
    va_end (args);

    return w_io_buf_str (&buffer_stream);
}


/*~f char* w_cstr_formatv (const char *format, va_list arguments)
 *
 * Create a C (``\0``-terminated) string with a given `format`, consuming
 * additional `arguments` as needed by the `format`.
 *
 * The returned strings must be freed by the caller.
 *
 * See :ref:`formatted-output` for the available formatting options.
 */
char*
w_cstr_formatv (const char *format, va_list args)
{
    w_assert (format);

    w_io_buf_t buffer_stream;
    w_io_buf_init (&buffer_stream, NULL, false);
    W_IO_NORESULT (w_io_formatv (&buffer_stream.parent, format, args));
    return w_io_buf_str (&buffer_stream);
}


uint64_t
w_str_hash (const char *str)
{
#ifdef W_CONF_SIPHASH
    return w_str_hashl (str, strlen (str));
#else
	register uint64_t ret = 0;
	register uint64_t ctr = 0;
	w_assert(str != NULL);

	while (*str) {
		ret ^= *str++ << ctr;
		ctr  = (ctr + 1) % sizeof (void*);
	}
	return ret;
#endif /* W_CONF_SIPHASH */
}


uint64_t
w_str_hashl (const char *str, size_t len)
{
#ifdef W_CONF_SIPHASH
    static uint8_t key[16] = { 0, };
    if (key[0] == 0) {
        /* Try getting data from /dev/urandom if possible */
        int fd = open ("/dev/urandom", O_RDONLY);
        if (fd >= 0) {
            int ret = read (fd, key, 16);
            close (fd);
            if (ret < 16)
                fd = -1;
        }
        if (fd < 0) {
            struct timeval tv;
            gettimeofday (&tv, NULL);
            uint32_t *words = (uint32_t*) key;
            words[0] = tv.tv_sec ^ tv.tv_usec;
            words[1] = tv.tv_sec;
            words[2] = tv.tv_usec;
            words[3] = getpid ();
        }
        key[0] |= 0x01;
    }

    uint8_t hash[8] = { 0, };
    siphash ((const uint8_t*) str, len, key, hash, sizeof (hash));
    return *((uint64_t*) hash);
#else
	register uint64_t ret = 0;
	register uint64_t ctr = 0;
	w_assert(str != NULL);

	while (len-- && *str) {
		ret ^= *str++ << ctr;
		ctr  = (ctr + 1) % sizeof (void*);
	}
	return ret;
#endif /* W_CONF_SIPHASH */
}


bool
w_str_bool(const char *str, bool *opt)
{
	w_assert(str != NULL);
	w_assert(opt != NULL);

	switch (strlen(str)) {
		case 1: /* One character length. */
			switch (*str) {
				case '0': case 'f': case 'F': case 'n': case 'N':
					*opt = false;
					break;
				case '1': case 't': case 'T': case 'y': case 'Y':
					*opt = true;
					break;
				default:
					return false;
			}
			return true;

		/* FIXME Tests below may be speed up. */

		case 2: /* Two characters: "no", "ok" */
			if (!w_str_casecmp ("no", str)) return (*opt = false, true);
			if (!w_str_casecmp ("ok", str)) return (*opt = true,  true);
			return false;
		case 3: /* Three characters: "yes", "nah", "nop" */
			if (!w_str_casecmp ("yes", str)) return (*opt = true , true);
			if (!w_str_casecmp ("nop", str)) return (*opt = false, true);
			if (!w_str_casecmp ("nah", str)) return (*opt = false, true);
			return false;
		case 4: /* Four characters: "yeah", "okay", "nope", true */
			if (!w_str_casecmp ("true", str)) return (*opt = true,  true);
			if (!w_str_casecmp ("yeah", str)) return (*opt = true,  true);
			if (!w_str_casecmp ("okay", str)) return (*opt = true,  true);
			if (!w_str_casecmp ("nope", str)) return (*opt = false, true);
			return false;
		case 5: /* Five characters: "false". */
			if (!w_str_casecmp ("false", str)) return (*opt = false, true);
			return false;
	}

	return false;
}



bool
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
        return (*val = v, true);
    }

	return false;
}


bool
w_str_uint(const char *str, unsigned *val)
{
	unsigned long v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtoull(str, &chkstr, 0);
	if ((*str != '\0') && (*chkstr == '\0') && (v <= UINT_MAX) &&
			!((v == ULONG_MAX) && (errno == ERANGE)))
		return (*val = v, true);

	return false;
}


bool
w_str_long(const char *str, long *val)
{
	long v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtol(str, &chkstr, 0);
	if ((*str != '\0') && (*chkstr == '\0') &&
	        !((v == LONG_MAX || v == LONG_MIN) && (errno == ERANGE)))
		return (*val = v, true);

	return false;
}


bool
w_str_ulong(const char *str, unsigned long *val)
{
	unsigned long v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtoul(str, &chkstr, 0);
	if ((*str != '\0') && (*chkstr == '\0') &&
	        !(v == ULONG_MAX && errno == ERANGE))
		return (*val = v, true);

	return false;
}


#ifdef HUGE_VALF
bool
w_str_float(const char *str, float *val)
{
	float v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtof(str, &chkstr);
	if (((v == HUGE_VALF) || (v == -HUGE_VALF)) && (errno == ERANGE))
		return false;

	return (*val = v, true);
}
#endif /* HUGE_VALF */


#ifdef HUGE_VAL
bool
w_str_double(const char *str, double *val)
{
	double v;
	char *chkstr;
	w_assert(str != NULL);
	w_assert(val != NULL);

	v = strtod(str, &chkstr);
	if (((v == HUGE_VAL) || (v == -HUGE_VAL)) && (errno == ERANGE))
		return false;

	return (*val = v, true);
}
#endif /* HUGE_VAL */


bool
w_str_size_bytes (const char *str, unsigned long long *val)
{
    unsigned long long v = 0;
    char *endpos;

    w_assert (str);
    w_assert (val);

    v = strtoull (str, &endpos, 0);

    if (v == ULLONG_MAX && errno == ERANGE)
        return false;

    if (endpos) {
        switch (*endpos) {
            case 'g': case 'G': v *= 1024 * 1024 * 1024; break; /* gigabytes */
            case 'm': case 'M': v *= 1024 * 1024;        break; /* megabytes */
            case 'k': case 'K': v *= 1024;               break; /* kilobytes */
            case 'b': case 'B': case '\0':               break; /* bytes     */
            default : return false;
        }
    }

    return (*val = v, true);
}


bool
w_str_time_period (const char *str, unsigned long long *val)
{
    unsigned long long v = 0;
    char *endpos;

    w_assert (str);
    w_assert (val);

    v = strtoull (str, &endpos, 0);

    if (v == ULLONG_MAX && errno == ERANGE)
        return false;

    if (endpos) {
        switch (*endpos) {
            case 'y': v *= 60 * 60 * 24 * 365; break; /* years   */
            case 'M': v *= 60 * 60 * 24 * 30;  break; /* months  */
            case 'w': v *= 60 * 60 * 24 * 7;   break; /* weeks   */
            case 'd': v *= 60 * 60 * 24;       break; /* days    */
            case 'h': v *= 60 * 60;            break; /* hours   */
            case 'm': v *= 60;                 break; /* minutes */
            case 's': case '\0':               break; /* seconds */
            default : return false;
        }
    }

    return (*val = v, true);
}
