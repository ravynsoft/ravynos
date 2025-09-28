/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2017 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudoers.h>

sudo_dso_public int main(int argc, char *argv[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v] [inputfile]\n", getprogname());
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    FILE *fp = stdin;
    char pattern[1024], string[1024];
    int ch, errors = 0, tests = 0, got, want;

    initprogname(argc > 0 ? argv[0] : "check_env_pattern");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignored */
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc > 0) {
	if ((fp = fopen(argv[0], "r")) == NULL) {
	    perror(argv[0]);
	    return EXIT_FAILURE;
	}
    }

    /*
     * Read in test file, which is formatted thusly:
     *
     * pattern string 1/0
     *
     */
    for (;;) {
	bool full_match = false;

	got = fscanf(fp, "%s %s %d\n", pattern, string, &want);
	if (got == EOF)
	    break;
	if (got == 3) {
	    got = matches_env_pattern(pattern, string, &full_match);
	    if (full_match)
		got++;
	    if (got != want) {
		fprintf(stderr,
		    "%s: %s %s: want %d, got %d\n",
		    getprogname(), pattern, string, want, got);
		errors++;
	    }
	    tests++;
	}
    }
    if (tests != 0) {
	printf("%s: %d test%s run, %d errors, %d%% success rate\n",
	    getprogname(), tests, tests == 1 ? "" : "s", errors,
	    (tests - errors) * 100 / tests);
    }
    return errors;
}
