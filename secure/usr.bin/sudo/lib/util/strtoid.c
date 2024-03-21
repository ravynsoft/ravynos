/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>			/* for id_t */

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_gettext.h>
#include <sudo_util.h>

/*
 * Make sure that the ID ends with a valid separator char.
 */
static bool
valid_separator(const char *p, const char *ep, const char *sep)
{
    bool valid = false;

    if (ep != p) {
	/* check for valid separator (including '\0') */
	if (sep == NULL)
	    sep = "";
	do {
	    if (*ep == *sep)
		valid = true;
	} while (*sep++ != '\0');
    }
    return valid;
}

/*
 * Parse a uid/gid in string form.
 * If sep is non-NULL, it contains valid separator characters (e.g. comma, space)
 * If endp is non-NULL it is set to the next char after the ID.
 * On success, returns the parsed ID and clears errstr.
 * On error, returns 0 and sets errstr.
 */
id_t
sudo_strtoidx_v1(const char *p, const char *sep, char **endp, const char **errstrp)
{
    const char *errstr;
    char *ep;
    id_t ret;
    debug_decl(sudo_strtoid, SUDO_DEBUG_UTIL);

    ret = (id_t)sudo_strtonumx(p, INT_MIN, UINT_MAX, &ep, &errstr);
    if (errstr == NULL) {
	/*
	 * Disallow id -1 (UINT_MAX), which means "no change"
	 * and check for a valid separator (if specified).
	 */
	if (ret == (id_t)-1 || ret == (id_t)UINT_MAX || !valid_separator(p, ep, sep)) {
	    errstr = N_("invalid value");
	    errno = EINVAL;
	    ret = 0;
	}
    }
    if (errstrp != NULL)
	*errstrp = errstr;
    if (endp != NULL)
	*endp = ep;
    debug_return_id_t(ret);
}

/* Backward compatibility */
id_t
sudo_strtoid_v1(const char *p, const char *sep, char **endp, const char **errstrp)
{
    return sudo_strtoidx_v1(p, sep, endp, errstrp);
}

/* Simplified interface */
id_t
sudo_strtoid_v2(const char *p, const char **errstrp)
{
    return sudo_strtoidx_v1(p, NULL, NULL, errstrp);
}
