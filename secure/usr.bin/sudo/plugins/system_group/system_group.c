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
#include <grp.h>

#include <sudo_compat.h>
#include <sudo_dso.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

/*
 * Sudoers group plugin that does group name-based lookups using the system
 * group database functions, similar to how sudo behaved prior to 1.7.3.
 * This can be used on systems where lookups by group ID are problematic.
 */

typedef struct group * (*sysgroup_getgrnam_t)(const char *);
typedef struct group * (*sysgroup_getgrgid_t)(gid_t);
typedef void (*sysgroup_gr_delref_t)(struct group *);

static sysgroup_getgrnam_t sysgroup_getgrnam;
static sysgroup_getgrgid_t sysgroup_getgrgid;
static sysgroup_gr_delref_t sysgroup_gr_delref;
static bool need_setent;

static int
sysgroup_init(int version, sudo_printf_t plugin_printf, char *const argv[])
{
    void *handle;

    if (SUDO_API_VERSION_GET_MAJOR(version) != GROUP_API_VERSION_MAJOR) {
	plugin_printf(SUDO_CONV_ERROR_MSG,
	    "sysgroup_group: incompatible major version %d, expected %d\n",
	    SUDO_API_VERSION_GET_MAJOR(version),
	    GROUP_API_VERSION_MAJOR);
	return -1;
    }

    /* Share group cache with sudo if possible. */
    handle = sudo_dso_findsym(SUDO_DSO_DEFAULT, "sudo_getgrnam");
    if (handle != NULL) {
	sysgroup_getgrnam = (sysgroup_getgrnam_t)handle;
    } else {
	sysgroup_getgrnam = (sysgroup_getgrnam_t)getgrnam;
	need_setent = true;
    }

    handle = sudo_dso_findsym(SUDO_DSO_DEFAULT, "sudo_getgrgid");
    if (handle != NULL) {
	sysgroup_getgrgid = (sysgroup_getgrgid_t)handle;
    } else {
	sysgroup_getgrgid = (sysgroup_getgrgid_t)getgrgid;
	need_setent = true;
    }

    handle = sudo_dso_findsym(SUDO_DSO_DEFAULT, "sudo_gr_delref");
    if (handle != NULL)
	sysgroup_gr_delref = (sysgroup_gr_delref_t)handle;

    if (need_setent)
	setgrent();

    return true;
}

static void
sysgroup_cleanup(void)
{
    if (need_setent)
	endgrent();
}

/*
 * Returns true if "user" is a member of "group", else false.
 */
static int
sysgroup_query(const char *user, const char *group, const struct passwd *pwd)
{
    char **member;
    struct group *grp;

    grp = sysgroup_getgrnam(group);
    if (grp == NULL && group[0] == '#' && group[1] != '\0') {
	const char *errstr;
	gid_t gid = sudo_strtoid(group + 1, &errstr);
	if (errstr == NULL)
	    grp = sysgroup_getgrgid(gid);
    }
    if (grp != NULL) {
	if (grp->gr_mem != NULL) {
	    for (member = grp->gr_mem; *member != NULL; member++) {
		if (strcasecmp(user, *member) == 0) {
		    if (sysgroup_gr_delref)
			sysgroup_gr_delref(grp);
		    return true;
		}
	    }
	}
	if (sysgroup_gr_delref)
	    sysgroup_gr_delref(grp);
    }

    return false;
}

sudo_dso_public struct sudoers_group_plugin group_plugin = {
    GROUP_API_VERSION,
    sysgroup_init,
    sysgroup_cleanup,
    sysgroup_query
};
