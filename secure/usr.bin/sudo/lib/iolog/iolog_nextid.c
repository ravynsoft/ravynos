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

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

/*
 * Read the on-disk sequence number, set sessid to the next
 * number, and update the on-disk copy.
 * Uses file locking to avoid sequence number collisions.
 */
bool
iolog_nextid(const char *iolog_dir, char sessid[7])
{
    char buf[32], *ep;
    int i, fd = -1;
    unsigned long id = 0;
    size_t len;
    ssize_t nread;
    bool ret = false;
    char pathbuf[PATH_MAX];
    static const char b36char[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const uid_t iolog_uid = iolog_get_uid();
    const gid_t iolog_gid = iolog_get_gid();
    debug_decl(iolog_nextid, SUDO_DEBUG_UTIL);

    /*
     * Create I/O log directory if it doesn't already exist.
     */
    len = strlcpy(pathbuf, iolog_dir, sizeof(pathbuf));
    if (len >= sizeof(pathbuf)) {
	errno = ENAMETOOLONG;
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: %s", __func__, iolog_dir);
	goto done;
    }
    if (!iolog_mkdirs(pathbuf))
	goto done;

    /*
     * Open sequence file
     */
    len = strlcat(pathbuf, "/seq", sizeof(pathbuf));
    if (len >= sizeof(pathbuf)) {
	errno = ENAMETOOLONG;
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: %s/seq", __func__, iolog_dir);
	goto done;
    }
    fd = iolog_openat(AT_FDCWD, pathbuf, O_RDWR|O_CREAT);
    if (fd == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to open %s", __func__, pathbuf);
	goto done;
    }
    if (!sudo_lock_file(fd, SUDO_LOCK)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to lock %s", pathbuf);
	goto done;
    }
    if (fchown(fd, iolog_uid, iolog_gid) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to fchown %d:%d %s", __func__,
	    (int)iolog_uid, (int)iolog_gid, pathbuf);
    }

    /* Read current seq number (base 36). */
    nread = read(fd, buf, sizeof(buf) - 1);
    if (nread != 0) {
	if (nread == -1) {
	    goto done;
	}
	if (buf[nread - 1] == '\n')
	    nread--;
	buf[nread] = '\0';
	id = strtoul(buf, &ep, 36);
	if (ep == buf || *ep != '\0' || id >= iolog_get_maxseq()) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: bad sequence number: %s", pathbuf, buf);
	    id = 0;
	}
    }
    id++;

    /*
     * Convert id to a string and stash in sessid.
     * Note that that least significant digits go at the end of the string.
     */
    for (i = 5; i >= 0; i--) {
	buf[i] = b36char[id % 36];
	id /= 36;
    }
    buf[6] = '\n';

    /* Stash id for logging purposes. */
    memcpy(sessid, buf, 6);
    sessid[6] = '\0';

    /* Rewind and overwrite old seq file, including the NUL byte. */
    if (pwrite(fd, buf, 7, 0) != 7) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to write %s", __func__, pathbuf);
	goto done;
    }
    ret = true;

done:
    if (fd != -1)
	close(fd);
    debug_return_bool(ret);
}
