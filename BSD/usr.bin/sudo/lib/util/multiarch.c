/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef __linux__
# include <sys/stat.h>
# include <sys/utsname.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sudo_compat.h>
#include <sudo_util.h>

# if defined(__linux__)
/* 
 * On Linux systems that use multi-arch, the actual DSO may be in a
 * machine-specific subdirectory.  If the specified path contains
 * /lib/ or /libexec/, insert a multi-arch directory after it.
 * If sb is non-NULL, stat(2) will be called on the new path, filling in sb.
 * Returns a dynamically allocated string on success and NULL on failure.
 */
char *
sudo_stat_multiarch_v1(const char *path, struct stat *sb)
{
#  if defined(__ILP32__)
    const char *libdirs[] = { "/libx32/", "/lib/", "/libexec/", NULL };
#  elif defined(__LP64__)
    const char *libdirs[] = { "/lib64/", "/lib/", "/libexec/", NULL };
#  else
    const char *libdirs[] = { "/lib32/", "/lib/", "/libexec/", NULL };
#  endif
    const char **lp, *lib, *slash;
    struct utsname unamebuf;
    char *newpath = NULL;
    size_t len;

    if (uname(&unamebuf) == -1)
	return NULL;

    for (lp = libdirs; *lp != NULL; lp++) {
	/* Replace lib64, lib32, libx32 with lib in new path. */
	const char *newlib = lp == libdirs ? "/lib/" : *lp;

	/* Search for lib dir in path, find the trailing slash. */
	lib = strstr(path, *lp);
	if (lib == NULL)
	    continue;
	slash = lib + strlen(*lp) - 1;

	/* Make sure there isn't already a machine-linux-gnu dir. */
	len = strcspn(slash + 1, "/-");
	if (strncmp(slash + 1 + len, "-linux-gnu/", 11) == 0) {
	    /* Multiarch already present. */
	    break;
	}

	/* Add machine-linux-gnu dir after /lib/ or /libexec/. */
	if (asprintf(&newpath, "%.*s%s%s-linux-gnu%s",
	    (int)(lib - path), path, newlib, unamebuf.machine, slash) == -1) {
	    newpath = NULL;
	    break;
	}

	/* If sb was set, use stat(2) to make sure newpath exists. */
	if (sb == NULL || stat(newpath, sb) == 0)
	    break;
	free(newpath);
	newpath = NULL;
    }

    return newpath;
}
#else
char *
sudo_stat_multiarch_v1(const char *path, struct stat *sb)
{
    return NULL;
}
#endif /* __linux__ */
