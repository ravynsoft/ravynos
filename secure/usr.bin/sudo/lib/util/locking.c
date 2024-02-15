/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2007, 2009-2015
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

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_debug.h>

bool
sudo_lock_file_v1(int fd, int type)
{
    return sudo_lock_region_v1(fd, type, 0);
}

/*
 * Lock/unlock all or part of a file.
 */
#ifdef HAVE_LOCKF
bool
sudo_lock_region_v1(int fd, int type, off_t len)
{
    int op, rc;
    off_t oldpos = -1;
    debug_decl(sudo_lock_region, SUDO_DEBUG_UTIL);

    switch (type) {
	case SUDO_LOCK:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: lock %d:%lld",
		__func__, fd, (long long)len);
	    op = F_LOCK;
	    break;
	case SUDO_TLOCK:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: tlock %d:%lld",
		__func__, fd, (long long)len);
	    op = F_TLOCK;
	    break;
	case SUDO_UNLOCK:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: unlock %d:%lld",
		__func__, fd, (long long)len);
	    op = F_ULOCK;
	    /* Must seek to start of file to unlock the entire thing. */
	    if (len == 0 && (oldpos = lseek(fd, 0, SEEK_CUR)) != -1) {
		if (lseek(fd, 0, SEEK_SET) == -1) {
		    sudo_debug_printf(
			SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
			"unable to seek to beginning");
		}
	    }
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: bad lock type %d",
		__func__, type);
	    errno = EINVAL;
	    debug_return_bool(false);
    }
    rc = lockf(fd, op, len);
    if (oldpos != -1) {
	if (lseek(fd, oldpos, SEEK_SET) == -1) {
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to restore offset");
	}
    }
    debug_return_bool(rc == 0);
}
#else
bool
sudo_lock_region_v1(int fd, int type, off_t len)
{
    struct flock lock;
    int func;
    debug_decl(sudo_lock_file, SUDO_DEBUG_UTIL);

    switch (type) {
	case SUDO_LOCK:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: lock %d:%lld",
		__func__, fd, (long long)len);
	    lock.l_type = F_WRLCK;
	    func = F_SETLKW;
	    break;
	case SUDO_TLOCK:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: tlock %d:%lld",
		__func__, fd, (long long)len);
	    lock.l_type = F_WRLCK;
	    func = F_SETLK;
	    break;
	case SUDO_UNLOCK:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: unlock %d:%lld",
		__func__, fd, (long long)len);
	    lock.l_type = F_UNLCK;
	    func = F_SETLK;
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: bad lock type %d",
		__func__, type);
	    errno = EINVAL;
	    debug_return_bool(false);
    }
    lock.l_start = 0;
    lock.l_len = len;
    lock.l_pid = 0;
    lock.l_whence = len ? SEEK_CUR : SEEK_SET;

    debug_return_bool(fcntl(fd, func, &lock) == 0);
}
#endif
