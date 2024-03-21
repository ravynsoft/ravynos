/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019, 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * Convert struct list_members to a comma-separated string with
 * the given variable name.  Escapes backslashes and commas.
 */
char *
serialize_list(const char *varname, struct list_members *members)
{
    struct list_member *lm, *next;
    size_t len, result_size;
    char *cp, *result;
    debug_decl(serialize_list, SUDOERS_DEBUG_PLUGIN);

    result_size = strlen(varname) + 1;
    SLIST_FOREACH(lm, members, entries) {
	for (cp = lm->value; *cp != '\0'; cp++) {
	    result_size++;
	    if (*cp == '\\' || *cp == ',')
		result_size++;
	}
	result_size++;
    }
    if ((result = malloc(result_size)) == NULL)
	goto bad;
    /* No need to check len for overflow here. */
    len = strlcpy(result, varname, result_size);
    result[len++] = '=';
    SLIST_FOREACH_SAFE(lm, members, entries, next) {
	for (cp = lm->value; *cp != '\0'; cp++) {
	    bool escape = (*cp == '\\' || *cp == ',');
	    if (len + 1 + escape >= result_size) {
		sudo_warnx(U_("internal error, %s overflow"), __func__);
		goto bad;
	    }
	    if (escape)
		result[len++] = '\\';
	    result[len++] = *cp;
	}
	if (next != NULL) {
	    if (len + 1 >= result_size) {
		sudo_warnx(U_("internal error, %s overflow"), __func__);
		goto bad;
	    }
	    result[len++] = ',';
	}
	result[len] = '\0';
    }
    debug_return_str(result);
bad:
    free(result);
    debug_return_str(NULL);
}
