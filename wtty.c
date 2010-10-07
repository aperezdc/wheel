/*
 * wtty.c
 * Copyright (C) 2006 Connectical Labs.
 * Adrian Perez <moebius@connectical.net>
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
/*
 * These are (-1) when we're not tracking terminal size with the signal
 * handler for SIGWINCH.
 */
static int s_terminal_rows = -1;
static int s_terminal_cols = -1;
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


/* TODO Finish implementation of this function. */
wbool
w_tty_size_notify(w_tty_notify_fun_t function, void *context)
{
	w_unused(function);
	w_unused(context);
#ifdef SIGWINCH
	w_unused(s_terminal_cols);
	w_unused(s_terminal_rows);
	return W_YES;
#else
	return W_NO;
#endif
}


