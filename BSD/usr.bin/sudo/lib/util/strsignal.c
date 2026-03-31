/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2014 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef HAVE_STRSIGNAL

#include <signal.h>

#include <sudo_compat.h>
#include <sudo_gettext.h>

#if defined(HAVE_DECL_SYS_SIGLIST) && HAVE_DECL_SYS_SIGLIST == 1
# define sudo_sys_siglist	sys_siglist
#elif defined(HAVE_DECL__SYS_SIGLIST) && HAVE_DECL__SYS_SIGLIST == 1
# define sudo_sys_siglist	_sys_siglist
#else
extern const char *const sudo_sys_siglist[NSIG];
#endif

/*
 * Get signal description string
 */
char *
sudo_strsignal(int signo)
{
    if (signo > 0 && signo < NSIG && sudo_sys_siglist[signo] != NULL)
	return (char *)sudo_sys_siglist[signo];
    /* XXX - should be "Unknown signal: %d" */
    return (char *)_("Unknown signal");
}
#endif /* HAVE_STRSIGNAL */
