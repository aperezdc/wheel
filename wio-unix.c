/*
 * wio-unix.c
 * Copyright (C) 2010 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <unistd.h>
#include <errno.h>


/*
 * NOTE: Casting void* to ptrdiff_t is more portable, because depending on
 * the platform pointers may not fit in integers and compilers could give
 * warnings or even refuse to compile this file.
 */

wbool
w_io_unix_close (void *udata)
{
    int fd = (ptrdiff_t) udata;
    return (close (fd) == 0);
}


ssize_t
w_io_unix_write (void *udata, const void *buf, size_t len)
{
    int fd = (ptrdiff_t) udata;
    ssize_t ret, n = len;

    while (len > 0) {
        do {
            ret = write (fd, buf, len);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            return ret;
        }

        buf += ret;
        len -= ret;
    }

    return n;
}


ssize_t
w_io_unix_read (void *udata, void *buf, size_t len)
{
    int fd = (ptrdiff_t) udata;
    ssize_t ret;

    do {
        ret = read (fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    return ret;
}

