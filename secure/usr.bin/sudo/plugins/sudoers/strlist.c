/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sudo_queue.h>
#include <sudo_util.h>
#include <sudoers_debug.h>
#include <strlist.h>

struct sudoers_string *
sudoers_string_alloc(const char *s)
{
    struct sudoers_string *cs;
    debug_decl(sudoers_string_alloc, SUDOERS_DEBUG_UTIL);

    if ((cs = malloc(sizeof(*cs))) != NULL) {
	if ((cs->str = strdup(s)) == NULL) {
	    free(cs);
	    cs = NULL;
	}
    }

    debug_return_ptr(cs);
}

void
sudoers_string_free(struct sudoers_string *cs)
{
    if (cs != NULL) {
	free(cs->str);
	free(cs);
    }
}

struct sudoers_str_list *
str_list_alloc(void)
{
    struct sudoers_str_list *strlist;
    debug_decl(str_list_alloc, SUDOERS_DEBUG_UTIL);

    strlist = malloc(sizeof(*strlist));
    if (strlist != NULL) {
	STAILQ_INIT(strlist);
	strlist->refcnt = 1;
    }

    debug_return_ptr(strlist);
}

void
str_list_free(void *v)
{
    struct sudoers_str_list *strlist = v;
    struct sudoers_string *first;
    debug_decl(str_list_free, SUDOERS_DEBUG_UTIL);

    if (strlist != NULL) {
	if (--strlist->refcnt == 0) {
	    while ((first = STAILQ_FIRST(strlist)) != NULL) {
		STAILQ_REMOVE_HEAD(strlist, entries);
		sudoers_string_free(first);
	    }
	    free(strlist);
	}
    }
    debug_return;
}
