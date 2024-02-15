/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2010-2015, 2017-2019
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sudoers.h>

/*
 * Check the given command against the specified allowlist (NULL-terminated).
 * On success, rewrites cmnd based on the allowlist and returns true.
 * On failure, returns false.
 */
static bool
cmnd_allowed(char *cmnd, size_t cmnd_size, struct stat *cmnd_sbp,
    char * const *allowlist)
{
    const char *cmnd_base;
    char * const *al;
    debug_decl(cmnd_allowed, SUDOERS_DEBUG_UTIL);

    if (!sudo_goodpath(cmnd, cmnd_sbp))
	debug_return_bool(false);

    if (allowlist == NULL)
	debug_return_bool(true);	/* nothing to check */

    /* We compare the base names to avoid excessive stat()ing. */
    cmnd_base = sudo_basename(cmnd);

    for (al = allowlist; *al != NULL; al++) {
	const char *base, *path = *al;
	struct stat sb;

	base = sudo_basename(path);
	if (strcmp(cmnd_base, base) != 0)
	    continue;

	if (sudo_goodpath(path, &sb) &&
	    sb.st_dev == cmnd_sbp->st_dev && sb.st_ino == cmnd_sbp->st_ino) {
	    /* Overwrite cmnd with safe version from allowlist. */
	    if (strlcpy(cmnd, path, cmnd_size) < cmnd_size)
		debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

/*
 * This function finds the full pathname for a command and stores it
 * in a statically allocated array, filling in a pointer to the array.
 * Returns FOUND if the command was found, NOT_FOUND if it was not found,
 * NOT_FOUND_DOT if it would have been found but it is in '.' and
 * ignore_dot is set or NOT_FOUND_ERROR if an error occurred.
 * The caller is responsible for freeing the output file.
 */
int
find_path(const char *infile, char **outfile, struct stat *sbp,
    const char *path, bool ignore_dot, char * const *allowlist)
{
    char command[PATH_MAX];
    const char *cp, *ep, *pathend;
    bool found = false;
    bool checkdot = false;
    int len;
    debug_decl(find_path, SUDOERS_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"resolving %s", infile);

    /*
     * If we were given a fully qualified or relative path
     * there is no need to look at $PATH.
     */
    if (strchr(infile, '/') != NULL) {
	if (strlcpy(command, infile, sizeof(command)) >= sizeof(command)) {
	    errno = ENAMETOOLONG;
	    debug_return_int(NOT_FOUND_ERROR);
	}
	found = cmnd_allowed(command, sizeof(command), sbp, allowlist);
	goto done;
    }

    if (path == NULL)
	debug_return_int(NOT_FOUND);

    pathend = path + strlen(path);
    for (cp = sudo_strsplit(path, pathend, ":", &ep); cp != NULL;
	cp = sudo_strsplit(NULL, pathend, ":", &ep)) {

	/*
	 * Search current dir last if it is in PATH.
	 * This will miss sneaky things like using './' or './/' (XXX)
	 */
	if (cp == ep || (*cp == '.' && cp + 1 == ep)) {
	    checkdot = 1;
	    continue;
	}

	/*
	 * Resolve the path and exit the loop if found.
	 */
	len = snprintf(command, sizeof(command), "%.*s/%s",
	    (int)(ep - cp), cp, infile);
	if (len < 0 || len >= ssizeof(command)) {
	    errno = ENAMETOOLONG;
	    debug_return_int(NOT_FOUND_ERROR);
	}
	found = cmnd_allowed(command, sizeof(command), sbp, allowlist);
	if (found)
	    break;
    }

    /*
     * Check current dir if dot was in the PATH
     */
    if (!found && checkdot) {
	len = snprintf(command, sizeof(command), "./%s", infile);
	if (len < 0 || len >= ssizeof(command)) {
	    errno = ENAMETOOLONG;
	    debug_return_int(NOT_FOUND_ERROR);
	}
	found = cmnd_allowed(command, sizeof(command), sbp, allowlist);
	if (found && ignore_dot)
	    debug_return_int(NOT_FOUND_DOT);
    }

done:
    if (found) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "found %s", command);
	if ((*outfile = strdup(command)) == NULL)
	    debug_return_int(NOT_FOUND_ERROR);
	debug_return_int(FOUND);
    }
    debug_return_int(NOT_FOUND);
}
