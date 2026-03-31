/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2012, 2014-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sudo_debug.h>
#include <sudo_util.h>

/*
 * Create a new key=value pair and return it.
 * The caller is responsible for freeing the string.
 */
char *
sudo_new_key_val_v1(const char *key, const char *val)
{
    size_t key_len = strlen(key);
    size_t val_len = strlen(val);
    char *cp, *str;
    debug_decl(sudo_new_key_val, SUDO_DEBUG_UTIL);

    cp = str = malloc(key_len + 1 + val_len + 1);
    if (cp != NULL) {
	memcpy(cp, key, key_len);
	cp += key_len;
	*cp++ = '=';
	memcpy(cp, val, val_len);
	cp += val_len;
	*cp = '\0';
    }

    debug_return_str(str);
}
