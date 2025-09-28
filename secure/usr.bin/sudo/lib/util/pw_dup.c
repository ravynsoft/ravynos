/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2000, 2002, 2012-2014
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#ifndef HAVE_PW_DUP

#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include <sudo_compat.h>

#define PW_SIZE(name, size)				\
do {							\
	if (pw->name) {					\
		size = strlen(pw->name) + 1;		\
		total += size;				\
	}						\
} while (0)

#define PW_COPY(name, size)				\
do {							\
	if (pw->name) {					\
		(void)memcpy(cp, pw->name, size);	\
		newpw->name = cp;			\
		cp += size;				\
	}						\
} while (0)

struct passwd *
sudo_pw_dup(const struct passwd *pw)
{
	size_t nsize = 0, psize = 0, gsize = 0, dsize = 0, ssize = 0, total;
#ifdef HAVE_LOGIN_CAP_H
	size_t csize = 0;
#endif
	struct passwd *newpw;
	char *cp;

	/* Allocate in one big chunk for easy freeing */
	total = sizeof(struct passwd);
	PW_SIZE(pw_name, nsize);
	PW_SIZE(pw_passwd, psize);
#ifdef HAVE_LOGIN_CAP_H
	PW_SIZE(pw_class, csize);
#endif
	PW_SIZE(pw_gecos, gsize);
	PW_SIZE(pw_dir, dsize);
	PW_SIZE(pw_shell, ssize);

	if ((cp = malloc(total)) == NULL)
		return NULL;
	newpw = (struct passwd *)cp;

	/*
	 * Copy in passwd contents and make strings relative to space
	 * at the end of the buffer.
	 */
	(void)memcpy(newpw, pw, sizeof(struct passwd));
	cp += sizeof(struct passwd);

	PW_COPY(pw_name, nsize);
	PW_COPY(pw_passwd, psize);
#ifdef HAVE_LOGIN_CAP_H
	PW_COPY(pw_class, csize);
#endif
	PW_COPY(pw_gecos, gsize);
	PW_COPY(pw_dir, dsize);
	PW_COPY(pw_shell, ssize);

	return newpw;
}
#endif /* HAVE_PW_DUP */
