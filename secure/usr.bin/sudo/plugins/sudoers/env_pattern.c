/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2017 Todd C. Miller <Todd.Miller@sudo.ws>
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

/* extern for regress tests */
bool
matches_env_pattern(const char *pattern, const char *var, bool *full_match)
{
    size_t len, sep_pos;
    bool iswild = false, match = false;
    bool saw_sep = false;
    const char *cp;
    debug_decl(matches_env_pattern, SUDOERS_DEBUG_ENV);

    /* Locate position of the '=' separator in var=value. */
    sep_pos = strcspn(var, "=");

    /* Locate '*' wildcard and compute len. */
    for (cp = pattern; *cp != '\0'; cp++) {
	if (*cp == '*') {
	    iswild = true;
	    break;
	}
    }
    len = (size_t)(cp - pattern);

    if (iswild) {
	/* Match up to the '*' wildcard. */
	if (strncmp(pattern, var, len) == 0) {
	    while (*cp != '\0') {
		if (*cp == '*') {
		    /* Collapse sequential '*'s */
		    do {
			cp++;
		    } while (*cp == '*');
		    /* A '*' at the end of a pattern matches anything. */
		    if (*cp == '\0') {
			match = true;
			break;
		    }
		    /* Keep track of whether we matched an equal sign. */
		    if (*cp == '=')
			saw_sep = true;
		    /* Look for first match of text after the '*' */
		    while ((saw_sep || len != sep_pos) &&
			var[len] != '\0' && var[len] != *cp)
			len++;
		}
		if (var[len] != *cp)
		    break;
		cp++;
		len++;
	    }
	    if (*cp == '\0' && (len == sep_pos || var[len] == '\0'))
		match = true;
	}
    } else {
	if (strncmp(pattern, var, len) == 0 &&
	    (len == sep_pos || var[len] == '\0')) {
	    match = true;
	}
    }
    if (match)
	*full_match = len > sep_pos + 1;
    debug_return_bool(match);
}
