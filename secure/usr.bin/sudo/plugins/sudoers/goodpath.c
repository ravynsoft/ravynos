/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2010-2012, 2014-2016
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
#include <string.h>
#include <errno.h>

#include <sudoers.h>

/*
 * Verify that path is a normal file and executable by root.
 */
bool
sudo_goodpath(const char *path, struct stat *sbp)
{
    bool ret = false;
    struct stat sb;
    debug_decl(sudo_goodpath, SUDOERS_DEBUG_UTIL);

    if (path != NULL) {
	if (sbp == NULL)
	    sbp = &sb;

	if (stat(path, sbp) == 0) {
	    /* Make sure path describes an executable regular file. */
	    if (S_ISREG(sbp->st_mode) && ISSET(sbp->st_mode, S_IXUSR|S_IXGRP|S_IXOTH))
		ret = true;
	    else
		errno = EACCES;
	}
    }
    debug_return_bool(ret);
}
