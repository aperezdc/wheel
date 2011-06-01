/*
 * wtty.c
 * Copyright (C) 2006 Adrian Perez <the.lightman@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>


#ifndef W_TTY_DEFAULT_COLS
#define W_TTY_DEFAULT_COLS 80
#endif /* !W_TTY_DEFAULT_COLS */

#ifndef W_TTY_DEFAULT_ROWS
#define W_TTY_DEFAULT_ROWS 24
#endif /* !W_TTY_DEFAULT_ROWS */


#ifdef SIGWINCH
static w_tty_notify_fun_t s_terminal_resize_cb  = NULL;
static void              *s_terminal_resize_ctx = NULL;
#endif /* SIGWINCH */


unsigned
w_tty_cols(void)
{
	unsigned cols;

	if (w_tty_size(&cols, NULL))
		return cols;
	else
		return W_TTY_DEFAULT_COLS;
}


unsigned
w_tty_rows(void)
{
	unsigned rows;

	if (w_tty_size(NULL, &rows))
		return rows;
	else
		return W_TTY_DEFAULT_ROWS;
}


wbool
w_tty_size(unsigned *cols, unsigned *rows)
{
#ifdef TIOCGWINSZ
	struct winsize tty_size;
	int tty_fd;
#endif

	/* No place where to store stuff exists -> no action taken. */
	if ((cols == NULL) && (rows == NULL)) return W_NO;

	if (cols != NULL) *cols = W_TTY_DEFAULT_COLS;
	if (rows != NULL) *rows = W_TTY_DEFAULT_ROWS;

#ifdef TIOCGWINSZ
	if ((tty_fd = open("/dev/tty", O_RDONLY, 0)) == -1)
		return W_NO;

	if (ioctl(tty_fd, TIOCGWINSZ, &tty_size) == -1) {
		close(tty_fd);
		return W_NO;
	}
	close(tty_fd);

	if (cols != NULL) *cols = tty_size.ws_col;
	if (rows != NULL) *rows = tty_size.ws_row;
#endif

	return W_YES;
}


#ifdef SIGWINCH
static void
update_tty_size (int signum)
{
    unsigned cols, rows;
    w_assert (signum == SIGWINCH);
    w_unused (signum);

    if (s_terminal_resize_cb) {
        w_tty_size (&cols, &rows);
        (*s_terminal_resize_cb) (cols, rows, s_terminal_resize_ctx);
    }
}
#endif /* SIGWINCH */


/* TODO Finish implementation of this function. */
wbool
w_tty_size_notify(w_tty_notify_fun_t function, void *context)
{
	w_unused(function);
	w_unused(context);
#ifdef SIGWINCH
	struct sigaction sa;

    s_terminal_resize_cb  = function;
    s_terminal_resize_ctx = context;

    sa.sa_handler = (function == NULL) ? SIG_DFL : update_tty_size;
    sa.sa_flags  |= SA_RESTART;
    sigfillset (&sa.sa_mask);

    return sigaction (SIGWINCH, &sa, NULL) < 0;
#else /* !SIGWINCH */
	return W_NO;
#endif /* SIGWINCH */
}


