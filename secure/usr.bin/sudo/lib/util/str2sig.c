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

#ifndef HAVE_STR2SIG

#include <errno.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>

#if !defined(HAVE_SIGABBREV_NP)
# if defined(HAVE_DECL_SYS_SIGNAME) && HAVE_DECL_SYS_SIGNAME == 1
#   define sigabbrev_np(_x)	sys_signame[(_x)]
# elif defined(HAVE_DECL__SYS_SIGNAME) && HAVE_DECL__SYS_SIGNAME == 1
#   define sigabbrev_np(_x)	_sys_signame[(_x)]
# elif defined(HAVE_SYS_SIGABBREV)
#   define sigabbrev_np(_x)	sys_sigabbrev[(_x)]
#  if defined(HAVE_DECL_SYS_SIGABBREV) && HAVE_DECL_SYS_SIGABBREV == 0
    /* sys_sigabbrev is not declared by glibc */
    extern const char *const sys_sigabbrev[NSIG];
#  endif
# else
#   define sigabbrev_np(_x)	sudo_sys_signame[(_x)]
    extern const char *const sudo_sys_signame[NSIG];
# endif
#endif /* !HAVE_SIGABBREV_NP */

/*
 * Many systems use aliases for source backward compatibility.
 */
static struct sigalias {
    const char *name;
    int number;
} sigaliases[] = {
#ifdef SIGABRT
    { "ABRT", SIGABRT },
#endif
#ifdef SIGCLD
    { "CLD",  SIGCLD },
#endif
#ifdef SIGIO
    { "IO",   SIGIO },
#endif
#ifdef SIGIOT
    { "IOT",  SIGIOT },
#endif
#ifdef SIGLOST
    { "LOST", SIGLOST },
#endif
#ifdef SIGPOLL
    { "POLL", SIGPOLL },
#endif
    { NULL, -1 }
};

/*
 * Translate signal name to number.
 */
int
sudo_str2sig(const char *signame, int *result)
{
    struct sigalias *alias;
    const char *errstr;
    int signo;

    /* Could be a signal number encoded as a string. */
    if (isdigit((unsigned char)signame[0])) {
	signo = (int)sudo_strtonum(signame, 0, NSIG - 1, &errstr);
	if (errstr != NULL)
	    return -1;
	*result = signo;
	return 0;
    }

    /* Check real-time signals. */
#if defined(SIGRTMIN)
    if (strncmp(signame, "RTMIN", 5) == 0) {
	if (signame[5] == '\0') {
	    *result = SIGRTMIN;
	    return 0;
	}
	if (signame[5] == '+') {
	    if (isdigit((unsigned char)signame[6])) {
# ifdef _SC_RTSIG_MAX
		const long rtmax = sysconf(_SC_RTSIG_MAX);
# else
		const long rtmax = SIGRTMAX - SIGRTMIN;
# endif
		const int off = signame[6] - '0';

		if (rtmax > 0 && off < rtmax / 2) {
		    *result = SIGRTMIN + off;
		    return 0;
		}
	    }
	}
    }
#endif
#if defined(SIGRTMAX)
    if (strncmp(signame, "RTMAX", 5) == 0) {
	if (signame[5] == '\0') {
	    *result = SIGRTMAX;
	    return 0;
	}
	if (signame[5] == '-') {
	    if (isdigit((unsigned char)signame[6])) {
# ifdef _SC_RTSIG_MAX
		const long rtmax = sysconf(_SC_RTSIG_MAX);
# else
		const long rtmax = SIGRTMAX - SIGRTMIN;
# endif
		const int off = signame[6] - '0';

		if (rtmax > 0 && off < rtmax / 2) {
		    *result = SIGRTMAX - off;
		    return 0;
		}
	    }
	}
    }
#endif

    /* Check aliases. */
    for (alias = sigaliases; alias->name != NULL; alias++) {
	if (strcmp(signame, alias->name) == 0) {
	    *result = alias->number;
	    return 0;
	}
    }

    for (signo = 1; signo < NSIG; signo++) {
	const char *cp = sigabbrev_np(signo);
	if (cp != NULL) {
	    /* On macOS sys_signame[] may contain lower-case names. */
	    if (strcasecmp(signame, cp) == 0) {
		*result = signo;
		return 0;
	    }
	}
    }

    errno = EINVAL;
    return -1;
}
#endif /* HAVE_STR2SIG */
