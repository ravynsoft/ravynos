/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015, 2020-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdlib.h>
#include <string.h>

#include <sudo_compat.h>
#include <sudo_util.h>

/*
 * Declare/define __progname[] if necessary.
 * Assumes __progname[] is present if we have getprogname(3).
 */
#ifndef HAVE_SETPROGNAME
# if defined(HAVE_GETPROGNAME) || defined(HAVE___PROGNAME)
extern const char *__progname;
# else
static const char *__progname = "";
# endif /* HAVE_GETPROGNAME || HAVE___PROGNAME */
#endif /* HAVE_SETPROGNAME */

#ifndef HAVE_GETPROGNAME
const char *
sudo_getprogname(void)
{
    return __progname;
}
#endif

#ifndef HAVE_SETPROGNAME
void
sudo_setprogname(const char *name)
{
    __progname = sudo_basename(name);
}
#endif

void
initprogname2(const char *name, const char * const * allowed)
{
    const char *progname;
    size_t i;

    /* Fall back on "name" if getprogname() returns an empty string. */
    if ((progname = getprogname()) != NULL && *progname != '\0') {
	name = progname;
    } else {
	/* Make sure user-specified name is relative. */
	name = sudo_basename(name);
    }

    /* Check for libtool prefix and strip it if present. */
    if (name[0] == 'l' && name[1] == 't' && name[2] == '-' && name[3] != '\0')
	name += 3;

    /* Check allow list if present (first element is the default). */
    if (allowed != NULL) {
	for (i = 0; ; i++) {
	    if (allowed[i] == NULL) {
		name = allowed[0];
		break;
	    }
	    if (strcmp(allowed[i], name) == 0)
		break;
	}
    }

    /* Update internal progname if needed. */
    if (name != progname)
	setprogname(name);
    return;
}

void
initprogname(const char *name)
{
    initprogname2(name, NULL);
}
