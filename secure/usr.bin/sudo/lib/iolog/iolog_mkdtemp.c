/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <time.h>
#include <errno.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_util.h>
#include <sudo_iolog.h>

/*
 * Create temporary directory and any parent directories as needed.
 */
bool
iolog_mkdtemp(char *path)
{
    const mode_t iolog_filemode = iolog_get_file_mode();
    const mode_t iolog_dirmode = iolog_get_dir_mode();
    const uid_t iolog_uid = iolog_get_uid();
    const gid_t iolog_gid = iolog_get_gid();
    bool ok = false, uid_changed = false;
    char *dir = sudo_basename(path);
    mode_t omask;
    int dfd;
    debug_decl(iolog_mkdtemp, SUDO_DEBUG_UTIL);

    /* umask must not be more restrictive than the file modes. */
    omask = umask(ACCESSPERMS & ~(iolog_filemode|iolog_dirmode));

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
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "mkdtemp %s", path);
	/* We cannot retry mkdtemp() so always open as iolog user */
	if (!uid_changed)
	    uid_changed = iolog_swapids(false);
	if (mkdtempat(dfd, dir) == NULL) {
	    sudo_warn(U_("unable to mkdir %s"), path);
	    ok = false;
	} else {
	    if (fchmodat(dfd, dir, iolog_dirmode, 0) != 0) {
		/* Not a fatal error, pre-existing mode is 0700. */
		sudo_warn(U_("unable to change mode of %s to 0%o"),
		    path, (unsigned int)iolog_dirmode);
	    }
	    ok = true;
	}
	close(dfd);
    }

    umask(omask);

    if (uid_changed) {
	if (!iolog_swapids(true))
	    ok = false;
    }
    debug_return_bool(ok);
}
