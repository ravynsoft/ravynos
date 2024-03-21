/*
 * Copyright (c) 2012 Darren Tucker (dtucker at zip com au).
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
 * openssh_bsd_setres_id.c - setresuid() and setresgid() wrappers
 *
 * This file is part of zsh, the Z shell.
 *
 * It is based on the file openbsd-compat/bsd-setres_id.c in OpenSSH 7.9p1,
 * which is subject to the copyright notice above.  The zsh modifications are
 * licensed as follows:
 *
 * Copyright (c) 2019 Daniel Shahaf
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Daniel Shahaf or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Daniel Shahaf and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Daniel Shahaf and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Daniel Shahaf and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zsh.mdh"
#include "openssh_bsd_setres_id.pro"

#include <sys/types.h>

#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#ifdef __NetBSD__
/*
 * On NetBSD, setreuid() does not reset the saved uid if the real uid
 * is not modified. Better to use setuid() that resets all of real,
 * effective and saved uids to the specified value. Same for setregid().
 */
#define BROKEN_SETREUID
#define BROKEN_SETREGID
#endif

#if defined(ZSH_IMPLEMENT_SETRESGID) || defined(BROKEN_SETRESGID)
int
setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
	int ret = 0, saved_errno;

	if (rgid != sgid) {
		errno = ENOSYS;
		return -1;
	}
#if defined(ZSH_HAVE_NATIVE_SETREGID) && !defined(BROKEN_SETREGID)
	if (setregid(rgid, egid) < 0) {
		saved_errno = errno;
		zwarnnam("setregid", "to gid %L: %e", (long)rgid, errno);
		errno = saved_errno;
		ret = -1;
	}
#else
	if (setegid(egid) < 0) {
		saved_errno = errno;
		zwarnnam("setegid", "to gid %L: %e", (long)(unsigned int)egid, errno);
		errno = saved_errno;
		ret = -1;
	}
	if (setgid(rgid) < 0) {
		saved_errno = errno;
		zwarnnam("setgid", "to gid %L: %e", (long)rgid, errno);
		errno = saved_errno;
		ret = -1;
	}
#endif
	return ret;
}
#endif

#if defined(ZSH_IMPLEMENT_SETRESUID) || defined(BROKEN_SETRESUID)
int
setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	int ret = 0, saved_errno;

	if (ruid != suid) {
		errno = ENOSYS;
		return -1;
	}
#if defined(ZSH_HAVE_NATIVE_SETREUID) && !defined(BROKEN_SETREUID)
	if (setreuid(ruid, euid) < 0) {
		saved_errno = errno;
		zwarnnam("setreuid", "to uid %L: %e", (long)ruid, errno);
		errno = saved_errno;
		ret = -1;
	}
#else

# ifndef SETEUID_BREAKS_SETUID
	if (seteuid(euid) < 0) {
		saved_errno = errno;
		zwarnnam("seteuid", "to uid %L: %e", (long)euid, errno);
		errno = saved_errno;
		ret = -1;
	}
# endif
	if (setuid(ruid) < 0) {
		saved_errno = errno;
		zwarnnam("setuid", "to uid %L: %e", (long)ruid, errno);
		errno = saved_errno;
		ret = -1;
	}
#endif
	return ret;
}
#endif
