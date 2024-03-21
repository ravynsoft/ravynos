/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2001, 2003, 2004, 2008-2011, 2013, 2015, 2017, 2018, 2022
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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

#if (!defined(HAVE_MKDTEMPAT) && !defined(HAVE_MKDTEMPAT_NP)) || \
    (!defined(HAVE_MKOSTEMPSAT) && !defined(HAVE_MKOSTEMPSAT_NP))

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_rand.h>
#include <pathnames.h>

#define MKTEMP_FILE	1
#define MKTEMP_DIR	2

#define TEMPCHARS	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define NUM_CHARS	(sizeof(TEMPCHARS) - 1)
#define MIN_X		6

#define MKOTEMP_FLAGS	(O_APPEND | O_CLOEXEC | O_SYNC)

static int
mktemp_internal(int dfd, char *path, int slen, int mode, int flags)
{
	char *start, *cp, *ep;
	const char tempchars[] = TEMPCHARS;
	unsigned int tries;
	size_t len;
	int fd;

	len = strlen(path);
	if (len < MIN_X || slen < 0 || (size_t)slen > len - MIN_X) {
		errno = EINVAL;
		return -1;
	}
	ep = path + len - slen;

	for (start = ep; start > path && start[-1] == 'X'; start--)
		;
	if (ep - start < MIN_X) {
		errno = EINVAL;
		return -1;
	}

	if (flags & ~MKOTEMP_FLAGS) {
		errno = EINVAL;
		return -1;
	}
	flags |= O_CREAT | O_EXCL | O_RDWR;

	tries = INT_MAX;
	do {
		cp = start;
		do {
			unsigned short rbuf[16];
			size_t i;

			/*
			 * Avoid lots of arc4random() calls by using
			 * a buffer sized for up to 16 Xs at a time.
			 */
			arc4random_buf(rbuf, sizeof(rbuf));
			for (i = 0; i < nitems(rbuf) && cp != ep; i++)
				*cp++ = tempchars[rbuf[i] % NUM_CHARS];
		} while (cp != ep);

		switch (mode) {
		case MKTEMP_FILE:
			fd = openat(dfd, path, flags, S_IRUSR|S_IWUSR);
			if (fd != -1 || errno != EEXIST)
				return fd;
			break;
		case MKTEMP_DIR:
			if (mkdirat(dfd, path, S_IRWXU) == 0)
				return 0;
			if (errno != EEXIST)
				return -1;
			break;
		}
	} while (--tries);

	errno = EEXIST;
	return -1;
}

char *
sudo_mkdtemp(char *path)
{
	if (mktemp_internal(AT_FDCWD, path, 0, MKTEMP_DIR, 0) == -1)
		return NULL;
	return path;
}

char *
sudo_mkdtempat(int dfd, char *path)
{
	if (mktemp_internal(dfd, path, 0, MKTEMP_DIR, 0) == -1)
		return NULL;
	return path;
}

int
sudo_mkostempsat(int dfd, char *path, int slen, int flags)
{
	return mktemp_internal(dfd, path, slen, MKTEMP_FILE, flags);
}

#ifdef notyet
int
sudo_mkostemps(char *path, int slen, int flags)
{
	return mktemp_internal(AT_FDCWD, path, slen, MKTEMP_FILE, flags);
}
#endif

int
sudo_mkstemp(char *path)
{
	return mktemp_internal(AT_FDCWD, path, 0, MKTEMP_FILE, 0);
}

#ifdef notyet
int
sudo_mkostemp(char *path, int flags)
{
	return mktemp_internal(AT_FDCWD, path, 0, MKTEMP_FILE, flags);
}
#endif

int
sudo_mkstemps(char *path, int slen)
{
	return mktemp_internal(AT_FDCWD, path, slen, MKTEMP_FILE, 0);
}
#endif /* !HAVE_MKDTEMPAT || !HAVE_MKOSTEMPSAT */
