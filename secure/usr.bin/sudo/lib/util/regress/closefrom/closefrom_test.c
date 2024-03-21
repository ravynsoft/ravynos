/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * Test that sudo_closefrom() works as expected.
 */

int
main(int argc, char *argv[])
{
    int ch, fds[2], flag, maxfd, minfd, errors = 0, ntests = 0;
    initprogname(argc > 0 ? argv[0] : "closefrom_test");

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

    /* We use pipe() because it doesn't rely on the filesystem. */
    ntests++;
    if (pipe(fds) == -1) {
	sudo_warn("%s", "pipe");
	errors++;
	goto done;
    }
    maxfd = MAX(fds[0], fds[1]);
    minfd = MIN(fds[0], fds[1]);

    /* Close any fds greater than fds[0] and fds[1]. */
    sudo_closefrom(maxfd + 1);

    /* Verify that sudo_closefrom() didn't close fds[0] or fds[1]. */
    ntests++;
    if (fcntl(fds[0], F_GETFL, 0) == -1) {
	sudo_warnx("fd %d closed prematurely", fds[0]);
	errors++;
	goto done;
    }
    ntests++;
    if (fcntl(fds[1], F_GETFL, 0) == -1) {
	sudo_warnx("fd %d closed prematurely", fds[1]);
	errors++;
	goto done;
    }

    /* Close fds[0], fds[1] and above. */
    sudo_closefrom(minfd);

    /* Verify that sudo_closefrom() closed both fds. */
    ntests++;
    flag = fcntl(fds[0], F_GETFD, 0);
#ifdef __APPLE__
    /* We only set the close-on-exec flag on macOS. */
    if (flag == 1)
	flag = -1;
#endif
    if (flag != -1) {
	sudo_warnx("fd %d still open", fds[0]);
	errors++;
	goto done;
    }
    ntests++;
    flag = fcntl(fds[1], F_GETFD, 0);
#ifdef __APPLE__
    /* We only set the close-on-exec flag on macOS. */
    if (flag == 1)
	flag = -1;
#endif
    if (flag != -1) {
	sudo_warnx("fd %d still open", fds[1]);
	errors++;
	goto done;
    }

done:
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
