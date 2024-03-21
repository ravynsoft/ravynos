/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <termios.h>

#include <sudo_compat.h>

/* Non-standard termios input flags */
#ifndef IUCLC
# define IUCLC		0
#endif
#ifndef IMAXBEL
# define IMAXBEL	0
#endif

/* Non-standard termios local flags */
#ifndef IEXTEN
# define IEXTEN		0
#endif

/*
 * Set termios to raw mode (BSD extension).
 */
void
sudo_cfmakeraw(struct termios *term)
{
    /* Set terminal to raw mode */
    CLR(term->c_iflag,
	IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IMAXBEL|IUCLC);
    CLR(term->c_oflag, OPOST);
    CLR(term->c_lflag, ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    CLR(term->c_cflag, CSIZE|PARENB);
    SET(term->c_cflag, CS8);
    term->c_cc[VMIN] = 1;
    term->c_cc[VTIME] = 0;
}
