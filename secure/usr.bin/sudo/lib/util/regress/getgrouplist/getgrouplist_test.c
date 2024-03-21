/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * Test that sudo_getgrouplist2() works as expected.
 */

int
main(int argc, char *argv[])
{
    int errors = 0;
#ifndef HAVE_GETGROUPLIST_2
    GETGROUPS_T *groups = NULL;
    struct passwd *pw;
    struct group *grp;
    char *username;
    int ch, i, j, ntests = 0;
    int ngroups;
    gid_t basegid;

    initprogname(argc > 0 ? argv[0] : "getgrouplist_test");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    if ((pw = getpwuid(0)) == NULL)
	sudo_fatal_nodebug("getpwuid(0)");
    basegid = pw->pw_gid;
    if ((username = strdup(pw->pw_name)) == NULL)
	sudo_fatal_nodebug(NULL);

    if (sudo_getgrouplist2(username, basegid, &groups, &ngroups) == -1)
	sudo_fatal_nodebug("sudo_getgroulist2");

    for (i = 0; i < ngroups; i++) {
	ntests++;

	/* Verify group ID exists. */
	if ((grp = getgrgid(groups[i])) == NULL) {
	    sudo_warnx_nodebug("unable to look up group ID %u",
		(unsigned int)groups[i]);
	    errors++;
	    continue;
	}

	/* Check user's primary gid from the passwd file. */
	if (grp->gr_gid == basegid)
	    continue;

	/* Verify group membership. */
	for (j = 0; grp->gr_mem[j] != NULL; j++) {
	    if (strcmp(username, grp->gr_mem[j]) == 0) {
		/* match */
		break;
	    }
	}
	if (grp->gr_mem[j] == NULL) {
	    sudo_warnx_nodebug("unable to find %s in group %s",
		username, grp->gr_name);
	    errors++;
	    continue;
	}
    }
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    free(username);
    free(groups);
#endif /* HAVE_GETGROUPLIST_2 */
    return errors;
}
