/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef COMPAT_FNMATCH_H
#define COMPAT_FNMATCH_H

#define	FNM_NOMATCH	1		/* String does not match pattern */

#define	FNM_PATHNAME	(1 << 0)	/* Globbing chars don't match '/' */
#define	FNM_PERIOD	(1 << 1)	/* Leading '.' in string must exactly */
#define	FNM_NOESCAPE	(1 << 2)	/* Backslash treated as ordinary char */
#define	FNM_LEADING_DIR	(1 << 3)	/* Only match the leading directory */
#define	FNM_CASEFOLD	(1 << 4)	/* Case insensitive matching */

sudo_dso_public int sudo_fnmatch(const char *pattern, const char *string, int flags);

#define fnmatch(_a, _b, _c)	sudo_fnmatch((_a), (_b), (_c))

#endif /* COMPAT_FNMATCH_H */
