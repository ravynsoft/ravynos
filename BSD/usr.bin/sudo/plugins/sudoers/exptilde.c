/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <grp.h>
#include <pwd.h>

#include <sudoers.h>
#include <pwutil.h>

/*
 * Expand leading tilde in *path, which must be dynamically allocated.
 * Replaces path with the expanded version as needed, freeing the old one.
 * Returns true on success, false on failure.
 */
bool
expand_tilde(char **path, const char *user)
{
    char *npath, *opath = *path;
    char *slash = NULL;
    struct passwd *pw;
    int len;
    debug_decl(expand_tilde, SUDOERS_DEBUG_UTIL);

    switch (*opath++) {
    case '/':
	/* A fully-qualified path, nothing to do. */
	debug_return_bool(true);
    case '~':
	/* See below. */
	break;
    default:
	/* Not a fully-qualified path or one that starts with a tilde. */
	debug_return_bool(false);
    }

    switch (*opath) {
    case '\0':
	/* format: ~ */
	break;
    case '/':
	/* format: ~/foo */
	opath++;
	break;
    default:
	/* format: ~user/foo */
	user = opath;
	slash = strchr(opath, '/');
	if (slash != NULL) {
	    *slash = '\0';
	    opath = slash + 1;
	} else {
	    opath = (char *)"";
	}
    }
    pw = sudo_getpwnam(user);
    if (slash != NULL)
	*slash = '/';
    if (pw == NULL) {
	/* Unknown user. */
	sudo_warnx(U_("unknown user %s"), user);
	debug_return_bool(false);
    }

    len = asprintf(&npath, "%s%s%s", pw->pw_dir, *opath ? "/" : "", opath);
    sudo_pw_delref(pw);
    if (len == -1) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

    free(*path);
    *path = npath;
    debug_return_bool(true);
}
