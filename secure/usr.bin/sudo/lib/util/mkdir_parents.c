/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_debug.h>
#include <sudo_util.h>

#ifndef O_NOFOLLOW
# define O_NOFOLLOW 0
#endif

/*
 * Returns true if fd is a directory, else false.
 * Warns on failure if not quiet.
 */
static bool
is_dir(int dfd, const char *name, int namelen, bool quiet)
{
    struct stat sb;
    debug_decl(is_dir, SUDO_DEBUG_UTIL);

    if (fstat(dfd, &sb) != 0) {
	if (!quiet) {
	    sudo_warn(U_("unable to stat %.*s"), namelen, name);
	}
	debug_return_bool(false);
    }
    if (!S_ISDIR(sb.st_mode)) {
	if (!quiet) {
	    sudo_warnx(U_("%.*s exists but is not a directory (0%o)"),
		namelen, name, (unsigned int) sb.st_mode);
	}
	debug_return_bool(false);
    }

    debug_return_bool(true);
}

/*
 * Create any parent directories needed by path (but not path itself)
 * and return an open fd for the parent directory or -1 on error.
 */
int
sudo_open_parent_dir_v1(const char *path, uid_t uid, gid_t gid, mode_t mode,
    bool quiet)
{
    const char *cp, *ep, *pathend;
    char name[PATH_MAX];
    int parentfd;
    debug_decl(sudo_open_parent_dir, SUDO_DEBUG_UTIL);

    /* Starting parent dir is either root or cwd. */
    cp = path;
    if (*cp == '/') {
	do {
	    cp++;
	} while (*cp == '/');
	parentfd = open("/", O_RDONLY|O_NONBLOCK);
    } else {
	parentfd = open(".", O_RDONLY|O_NONBLOCK);
    }
    if (parentfd == -1) {
	if (!quiet)
	    sudo_warn(U_("unable to open %s"), *path == '/' ? "/" : ".");
	debug_return_int(-1);
    }

    /* Iterate over path components, skipping the last one. */
    pathend = cp + strlen(cp);
    for (cp = sudo_strsplit(cp, pathend, "/", &ep); cp != NULL && ep < pathend;
	cp = sudo_strsplit(NULL, pathend, "/", &ep)) {
	size_t len = (size_t)(ep - cp);
	int dfd;

	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "mkdir %.*s, mode 0%o, uid %d, gid %d", (int)(ep - path), path,
	    (unsigned int)mode, (int)uid, (int)gid);
	if (len >= sizeof(name)) {
	    errno = ENAMETOOLONG;
	    if (!quiet)
		sudo_warn(U_("unable to mkdir %.*s"), (int)(ep - path), path);
	    goto bad;
	}
	memcpy(name, cp, len);
	name[len] = '\0';
reopen:
	dfd = openat(parentfd, name, O_RDONLY|O_NONBLOCK, 0);
	if (dfd == -1) {
	    if (errno != ENOENT) {
		if (!quiet) {
		    sudo_warn(U_("unable to open %.*s"),
			(int)(ep - path), path);
		}
		goto bad;
	    }
	    if (mkdirat(parentfd, name, mode) == 0) {
		dfd = openat(parentfd, name, O_RDONLY|O_NONBLOCK|O_NOFOLLOW, 0);
		if (dfd == -1) {
		    if (!quiet) {
			sudo_warn(U_("unable to open %.*s"),
			    (int)(ep - path), path);
		    }
		    goto bad;
		}
		/* Make sure the path we created is still a directory. */
		if (!is_dir(dfd, path, (int)(ep - path), quiet)) {
		    close(dfd);
		    goto bad;
		}
		if (uid != (uid_t)-1 && gid != (gid_t)-1) {
		    if (fchown(dfd, uid, gid) != 0) {
			sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
			    "%s: unable to chown %d:%d %.*s", __func__,
			    (int)uid, (int)gid, (int)(ep - path), path);
		    }
		}
	    } else {
		if (errno == EEXIST)
		    goto reopen;
		if (!quiet) {
		    sudo_warn(U_("unable to mkdir %.*s"),
			(int)(ep - path), path);
		}
		goto bad;
	    }
	} else {
	    /* Already exists, make sure it is a directory. */
	    if (!is_dir(dfd, path, (int)(ep - path), quiet)) {
		close(dfd);
		goto bad;
	    }
	}
	close(parentfd);
	parentfd = dfd;
    }

    debug_return_int(parentfd);
bad:
    if (parentfd != -1)
	close(parentfd);
    debug_return_int(-1);
}

/*
 * Create any parent directories needed by path (but not path itself).
 * Not currently used.
 */
bool
sudo_mkdir_parents_v1(const char *path, uid_t uid, gid_t gid, mode_t mode,
    bool quiet)
{
    int fd;
    debug_decl(sudo_mkdir_parents, SUDO_DEBUG_UTIL);

    fd = sudo_open_parent_dir(path, uid, gid, mode, quiet);
    if (fd == -1)
	debug_return_bool(false);
    close(fd);
    debug_return_bool(true);
}
