/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2016-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

/* Trivial reference-counted strings. */
struct rcstr {
    int refcnt;
    char str[];
};

/*
 * Allocate a reference-counted string and copy src to it.
 * Returns the newly-created string with a refcnt of 1.
 */
char *
sudo_rcstr_dup(const char *src)
{
    size_t len = strlen(src);
    char *dst;
    debug_decl(sudo_rcstr_dup, SUDO_DEBUG_UTIL);

    dst = sudo_rcstr_alloc(len);
    if (dst != NULL) {
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    debug_return_ptr(dst);
}

char *
sudo_rcstr_alloc(size_t len)
{
    struct rcstr *rcs;
    debug_decl(sudo_rcstr_dup, SUDO_DEBUG_UTIL);

    rcs = malloc(sizeof(struct rcstr) + len + 1);
    if (rcs == NULL)
	return NULL;

    rcs->refcnt = 1;
    rcs->str[0] = '\0';
    /* cppcheck-suppress memleak */
    debug_return_ptr(rcs->str); // -V773
}

char *
sudo_rcstr_addref(const char *s)
{
    struct rcstr *rcs;
    debug_decl(sudo_rcstr_dup, SUDO_DEBUG_UTIL);

    if (s == NULL)
	debug_return_ptr(NULL);

    rcs = __containerof((const void *)s, struct rcstr, str);
    rcs->refcnt++;
    debug_return_ptr(rcs->str);
}

void
sudo_rcstr_delref(const char *s)
{
    struct rcstr *rcs;
    debug_decl(sudo_rcstr_dup, SUDO_DEBUG_UTIL);

    if (s != NULL) {
	rcs = __containerof((const void *)s, struct rcstr, str);
	if (--rcs->refcnt == 0) {
	    rcs->str[0] = '\0';
	    free(rcs);
	}
    }
    debug_return;
}
