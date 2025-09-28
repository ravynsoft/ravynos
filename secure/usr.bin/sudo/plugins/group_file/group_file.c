/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2014 Todd C. Miller <Todd.Miller@sudo.ws>
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
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>

#include <sudo_plugin.h>
#include <sudo_compat.h>

/*
 * Sample sudoers group plugin that uses an extra group file with the
 * same format as /etc/group.
 */

static sudo_printf_t sudo_log;

extern void mysetgrfile(const char *);
extern int mysetgroupent(int);
extern void myendgrent(void);
extern struct group *mygetgrnam(const char *);

static int
sample_init(int version, sudo_printf_t sudo_printf, char *const argv[])
{
    struct stat sb;

    sudo_log = sudo_printf;

    if (SUDO_API_VERSION_GET_MAJOR(version) != GROUP_API_VERSION_MAJOR) {
	sudo_log(SUDO_CONV_ERROR_MSG,
	    "group_file: incompatible major version %d, expected %d\n",
	    SUDO_API_VERSION_GET_MAJOR(version),
	    GROUP_API_VERSION_MAJOR);
	return -1;
    }

    /* Check that the group file exists and has a safe mode. */
    if (argv == NULL || argv[0] == NULL) {
	sudo_log(SUDO_CONV_ERROR_MSG,
	    "group_file: path to group file not specified\n");
	return -1;
    }
    if (stat(argv[0], &sb) != 0) {
	sudo_log(SUDO_CONV_ERROR_MSG,
	    "group_file: %s: %s\n", argv[0], strerror(errno));
	return -1;
    }
    if ((sb.st_mode & (S_IWGRP|S_IWOTH)) != 0) {
	sudo_log(SUDO_CONV_ERROR_MSG,
	    "%s must be only be writable by owner\n", argv[0]);
	return -1;
    }

    mysetgrfile(argv[0]);
    if (!mysetgroupent(1))
	return false;

    return true;
}

static void
sample_cleanup(void)
{
    myendgrent();
}

/*
 * Returns true if "user" is a member of "group", else false.
 */
static int
sample_query(const char *user, const char *group, const struct passwd *pwd)
{
    struct group *grp;
    char **member;

    grp = mygetgrnam(group);
    if (grp != NULL && grp->gr_mem != NULL) {
	for (member = grp->gr_mem; *member != NULL; member++) {
	    if (strcasecmp(user, *member) == 0)
		return true;
	}
    }

    return false;
}

sudo_dso_public struct sudoers_group_plugin group_plugin = {
    GROUP_API_VERSION,
    sample_init,
    sample_cleanup,
    sample_query
};
