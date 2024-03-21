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
#endif
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

/*
 * Create directory and any parent directories as needed.
 */
bool
iolog_mkdirs(const char *path)
{
    const mode_t iolog_filemode = iolog_get_file_mode();
    const mode_t iolog_dirmode = iolog_get_dir_mode();
    const uid_t iolog_uid = iolog_get_uid();
    const gid_t iolog_gid = iolog_get_gid();
    bool ok = true, uid_changed = false;
    struct stat sb;
    mode_t omask;
    int dfd;
    debug_decl(iolog_mkdirs, SUDO_DEBUG_UTIL);

    dfd = open(path, O_RDONLY|O_NONBLOCK);
    if (dfd == -1 && errno == EACCES) {
	/* Try again as the I/O log owner (for NFS). */
	if (iolog_swapids(false)) {
	    dfd = open(path, O_RDONLY|O_NONBLOCK);
	    if (!iolog_swapids(true)) {
		ok = false;
		goto done;
	    }
	}
    }
    if (dfd != -1 && fstat(dfd, &sb) != -1) {
	if (S_ISDIR(sb.st_mode)) {
	    if (sb.st_uid != iolog_uid || sb.st_gid != iolog_gid) {
		if (fchown(dfd, iolog_uid, iolog_gid) != 0) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
			"%s: unable to chown %d:%d %s", __func__,
			(int)iolog_uid, (int)iolog_gid, path);
		}
	    }
	    if ((sb.st_mode & ALLPERMS) != iolog_dirmode) {
		if (fchmod(dfd, iolog_dirmode) != 0) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
			"%s: unable to chmod 0%o %s", __func__,
			(int)iolog_dirmode, path);
		}
	    }
	} else {
	    sudo_warnx(U_("%s exists but is not a directory (0%o)"),
		path, (unsigned int) sb.st_mode);
	    errno = ENOTDIR;
	    ok = false;
	}
	goto done;
    }

    /* umask must not be more restrictive than the file modes. */
    omask = umask(ACCESSPERMS & ~(iolog_filemode|iolog_dirmode));

    ok = false;
    if (dfd != -1)
	close(dfd);
    dfd = sudo_open_parent_dir(path, iolog_uid, iolog_gid, iolog_dirmode, true);
    if (dfd == -1 && errno == EACCES) {
	/* Try again as the I/O log owner (for NFS). */
	uid_changed = iolog_swapids(false);
	if (uid_changed)
	    dfd = sudo_open_parent_dir(path, (uid_t)-1, (gid_t)-1,
		iolog_dirmode, false);
    }
    if (dfd != -1) {
	/* Create final path component. */
	const char *base = sudo_basename(path);
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "mkdir %s, mode 0%o", path, (unsigned int) iolog_dirmode);
	ok = mkdirat(dfd, base, iolog_dirmode) == 0 || errno == EEXIST;
	if (!ok) {
	    if (errno == EACCES && !uid_changed) {
		/* Try again as the I/O log owner (for NFS). */
		uid_changed = iolog_swapids(false);
		if (uid_changed)
		    ok = mkdirat(dfd, base, iolog_dirmode) == 0 || errno == EEXIST;
	    }
	    if (!ok)
		sudo_warn(U_("unable to mkdir %s"), path);
	} else {
	    if (fchownat(dfd, base, iolog_uid, iolog_gid, AT_SYMLINK_NOFOLLOW) != 0) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "%s: unable to chown %d:%d %s", __func__,
		    (int)iolog_uid, (int)iolog_gid, path);
	    }
	}
    }
    if (uid_changed) {
	if (!iolog_swapids(true))
	    ok = false;
    }

    umask(omask);

done:
    if (dfd != -1)
	close(dfd);
    debug_return_bool(ok);
}
