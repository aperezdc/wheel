/*
 * wtty.h
 * Copyright (C) 2006 Connectical Labs.
 * Adrian Perez <moebius@connectical.net>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __wtty_h__
#define __wtty_h__

/*!
 * \file wtty.h
 *
 * \copyright 2006 Connectical Labs.
 * \author Adrian Perez
 *
 * Provides utility functions which deal with the terminal where \c stdout
 * and \c stderr streams are pointing by default (the controlling terminal).
 *
 * Note that these functions may not work on non-Unix operating systems. If
 * a function does not work in the system or it does partially, it will do
 * the best possible effort. For example, functions returning the terminal
 * size will say the terminal has 80 columns and 24 rows (which is a known
 * safe value for those parameters).
 */

#include "wdef.h"

/*!
 * Obtains the dimensions (width & height) of the controlling terminal.
 *
 * \param cols Reference to a variable for the number of columns.
 * \param rows Reference to a variable for the number of rows.
 *
 * \return Wether the terminal size was guessed properly.
 */
W_EXPORT wbool w_tty_size(unsigned *cols, unsigned *rows);


/*!
 * Obtains the width of a row of the controlling terminal.
 *
 * \return Terminal width.
 */
W_EXPORT unsigned w_tty_cols(void);


/*!
 * Obtains the height of the the controlling terminal.
 *
 * \return Terminal height.
 */
W_EXPORT unsigned w_tty_rows(void);


typedef void (*w_tty_notify_fun_t)(unsigned, unsigned, void*);

/*!
 * Enables automatic tracking of the terminal size. Whenever the terminal
 * size changes, the supplied callback function will be called with the new
 * size of the terminal.
 *
 * \param function The function which will be notified of updates. Pass \c
 *		NULL in order to disable notifications and restore the signal handler
 *		for \c SIGWINCH.
 * \param context Context information which will be passed back to the
 *		callback function.
 * \return Wether the signal handler was correctly installed.
 *
 * \note This functionality requires the \c SIGWINCH signal to be defined.
 */
W_EXPORT wbool w_tty_size_notify(w_tty_notify_fun_t function, void *context);


#endif /* !__wtty_h__ */

