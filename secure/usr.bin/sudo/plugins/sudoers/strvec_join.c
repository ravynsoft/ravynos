/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sudoers.h>

#ifdef HAVE_STRLCPY
# define cpy_default	strlcpy
#else
# define cpy_default	sudo_strlcpy
#endif

/*
 * Join a NULL-terminated array of strings using the specified separator
 * char.  If non-NULL, the copy function must have strlcpy-like semantics.
 */
char *
strvec_join(char *const argv[], char sep, size_t (*cpy)(char *, const char *, size_t))
{
    char *dst, *result = NULL;
    char *const *av;
    size_t n, size = 0;
    debug_decl(strvec_join, SUDOERS_DEBUG_UTIL);

    for (av = argv; *av != NULL; av++)
	size += strlen(*av) + 1;
    if (size == 0 || (result = malloc(size)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_ptr(NULL);
    }

    if (cpy == NULL)
	cpy = cpy_default;
    for (dst = result, av = argv; *av != NULL; av++) {
	n = cpy(dst, *av, size);
	if (n >= size) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    free(result);
	    debug_return_ptr(NULL);
	}
	dst += n;
	size -= n;
	*dst++ = sep;
	size--;
    }
    dst[-1] = '\0';

    debug_return_str(result);
}
