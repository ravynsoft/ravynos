/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdlib.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>

/*
 * Return a malloc()ed copy of the system hostname, or NULL if 
 * malloc() or gethostname() fails.
 */
char *
sudo_gethostname_v1(void)
{
    char *hname;
    size_t host_name_max;

#ifdef _SC_HOST_NAME_MAX
    host_name_max = (size_t)sysconf(_SC_HOST_NAME_MAX);
    if (host_name_max == (size_t)-1)
#endif
	host_name_max = 255;	/* POSIX and historic BSD */

    hname = malloc(host_name_max + 1);
    if (hname != NULL) {
	if (gethostname(hname, host_name_max + 1) == 0 && *hname != '\0') {
	    /* Old gethostname() may not NUL-terminate if there is no room. */
	    hname[host_name_max] = '\0';
	} else {
	    free(hname);
	    hname = NULL;
	}
    }
    return hname;
}
