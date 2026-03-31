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

#include <sys/socket.h>

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_iolog.h>
#include <sudo_queue.h>
#include <logsrvd.h>

sudo_dso_public int main(int argc, char *argv[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v] conf_file\n", getprogname());
    exit(EXIT_FAILURE);
}

/*
 * Simple test driver for logsrvd_conf_read().
 * Just pases the file, errors to standard error.
 */
int
main(int argc, char *argv[])
{
    bool verbose = false;
    int ch, ntests, errors = 0;

    initprogname(argc > 0 ? argv[0] : "conf_test");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    verbose = true;
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc < 1)
	usage();

    for (ntests = 0; ntests < argc; ntests++) {
	const char *path = argv[ntests];
	if (verbose)
	    printf("reading %s\n", path);
	if (!logsrvd_conf_read(path))
	    errors++;
    }
    logsrvd_conf_cleanup();

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
