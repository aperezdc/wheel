/*
 * wio-unix.c
 * Copyright (C) 2010-2014 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * .. _wio-unix:
 *
 * Input/Output on Unix file descriptors
 * =====================================
 *
 * Once a Unix stream object has been initialized, they can be operated used
 * the common :ref:`stream functions <wio-functions>`.
 */

/**
 * Types
 * -----
 */

/*~t w_io_unix_t
 *
 * Performs input/output using Unix file descriptors.
 */

/**
 * Functions
 * ---------
 */

#include "wheel.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


static w_io_result_t
w_io_unix_close (w_io_t *iobase)
{
    w_io_unix_t *io = (w_io_unix_t*) iobase;
    if (io->fd >= 0) {
        if (close (io->fd) == -1)
            return W_IO_RESULT_ERROR (errno);
    }
    return W_IO_RESULT_SUCCESS;
}


static w_io_result_t
w_io_unix_write (w_io_t *io, const void *bufp, size_t len)
{
    ssize_t ret, n = len;
    const char *buf = bufp;

    while (len > 0) {
        do {
            ret = write (((w_io_unix_t*) io)->fd, buf, len);
        } while (ret < 0 && errno == EINTR);

        if (ret == -1)
            return W_IO_RESULT_ERROR (errno);

        buf += ret;
        len -= ret;
    }

    return W_IO_RESULT (n);
}


static w_io_result_t
w_io_unix_read (w_io_t *io, void *buf, size_t len)
{
    ssize_t ret;

    do {
        ret = read (((w_io_unix_t*) io)->fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    if (ret == -1)
        return W_IO_RESULT_ERROR (errno);
    if (ret == 0)
        return W_IO_RESULT_EOF;

    return W_IO_RESULT (ret);
}


static w_io_result_t
w_io_unix_flush (w_io_t *io)
{
    if (fsync (((w_io_unix_t*) io)->fd) == -1)
        return W_IO_RESULT_ERROR (errno);

    return W_IO_RESULT_SUCCESS;
}


static int
w_io_unix_getfd (w_io_t *io)
{
    return ((w_io_unix_t*) io)->fd;
}


/*~f w_io_t* w_io_unix_open (const char *path, int mode, unsigned permissions)
 *
 * Creates a stream object to be used with an Unix file descriptor by opening
 * the file at `path` with the given `mode` and `permissions`.
 *
 * This is a convenience function that calls ``open()`` and then uses
 * :func:`w_io_unix_open_fd()`.
 *
 * If opening the file fails, ``NULL`` is returned.
 */
w_io_t*
w_io_unix_open (const char *path, int mode, unsigned perm)
{
    w_assert (path);

    int fd;
    return ((fd = open (path, mode, perm)) < 0) ? NULL : w_io_unix_open_fd (fd);
}


/*~f w_io_t* w_io_unix_open_fd (int fd)
 *
 * Creates a stream object to be used with an Unix file descriptor.
 */
w_io_t*
w_io_unix_open_fd (int fd)
{
    w_io_unix_t *io = w_obj_new (w_io_unix_t);
    w_io_unix_init_fd (io, fd);
    return (w_io_t*) io;
}


/*~f bool w_io_unix_init (w_io_unix_t *stream, const char *path, int mode, unsigned permissions)
 *
 * Initializes a stream object (possibly allocated in the stack) to be used
 * with an Unix file descriptor by opening the file at `path` with the given
 * `mode` and `permissions`.
 *
 * This is a convenience function that calls ``open()`` and then uses
 * :func:`w_io_unix_init_fd()`.
 *
 * The return value indicates whether the file was opened successfully.
 */
bool
w_io_unix_init (w_io_unix_t *io, const char *path, int mode, unsigned perm)
{
    w_assert (io);
    w_assert (path);

    int fd;
    if ((fd = open (path, mode, perm)) < 0)
        return false;

    w_io_unix_init_fd (io, fd);
    return true;
}


/*~f void w_io_unix_init_fd (w_io_unix_t *stream, int fd)
 *
 * Initializes a stream object (possibly allocated in the stack) to be used
 * with an Unix file descriptor.
 */
void
w_io_unix_init_fd (w_io_unix_t *io, int fd)
{
    w_assert (io);
    w_assert (fd >= 0);

    w_io_init ((w_io_t*) io);

    io->parent.close = w_io_unix_close;
    io->parent.write = w_io_unix_write;
    io->parent.read  = w_io_unix_read;
    io->parent.flush = w_io_unix_flush;
    io->parent.getfd = w_io_unix_getfd;
    io->fd = fd;
}


static w_io_unix_t stdout_data =
{
    .parent = {
        .parent = W_OBJ_STATIC (w_io_unix_close),
        .backch = W_IO_EOF,
        .close  = w_io_unix_close,
        .write  = w_io_unix_write,
        .read   = w_io_unix_read,
        .flush  = w_io_unix_flush,
        .getfd  = w_io_unix_getfd,
    },
    .fd = STDOUT_FILENO,
};

static w_io_unix_t stderr_data =
{
    .parent = {
        .parent = W_OBJ_STATIC (w_io_unix_close),
        .backch = W_IO_EOF,
        .close  = w_io_unix_close,
        .write  = w_io_unix_write,
        .read   = w_io_unix_read,
        .flush  = w_io_unix_flush,
        .getfd  = w_io_unix_getfd,
    },
    .fd = STDERR_FILENO,
};

static w_io_unix_t stdin_data =
{
    .parent = {
        .parent = W_OBJ_STATIC (w_io_unix_close),
        .backch = W_IO_EOF,
        .close  = w_io_unix_close,
        .write  = w_io_unix_write,
        .read   = w_io_unix_read,
        .flush  = w_io_unix_flush,
        .getfd  = w_io_unix_getfd,
    },
    .fd = STDIN_FILENO,
};

/* Public standard I/O objects */
w_io_t *w_stdout = &stdout_data.parent;
w_io_t *w_stderr = &stderr_data.parent;
w_io_t *w_stdin  = &stdin_data.parent;
