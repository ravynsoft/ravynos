/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * Implement "id -G" using sudo_getgrouplist2().
 */

int
main(int argc, char *argv[])
{
    char *username = NULL;
    GETGROUPS_T *groups = NULL;
    struct passwd *pw;
    int ch, i, ngroups;
    gid_t basegid;

    initprogname(argc > 0 ? argv[0] : "getgids");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v] [user]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    if (argc > 0)
	username = argv[0];

    if (username != NULL) {
	if ((pw = getpwnam(username)) == NULL)
	    sudo_fatalx("unknown user name %s", username);
    } else {
	if ((pw = getpwuid(getuid())) == NULL)
	    sudo_fatalx("unknown user ID %u", (unsigned int)getuid());
    }
    basegid = pw->pw_gid;
    if ((username = strdup(pw->pw_name)) == NULL)
	sudo_fatal(NULL);

    if (sudo_getgrouplist2(username, basegid, &groups, &ngroups) == -1)
	sudo_fatal("sudo_getgroulist2");

    for (i = 0; i < ngroups; i++) {
	printf("%s%u", i ? " " : "", (unsigned int)groups[i]);
    }
    putchar('\n');
    return EXIT_SUCCESS;
}
