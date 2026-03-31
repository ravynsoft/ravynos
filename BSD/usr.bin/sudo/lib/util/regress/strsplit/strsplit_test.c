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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * Test that sudo_strsplit() works as expected.
 */

struct strsplit_test {
    const char *input;
    size_t input_len;
    const char **output;
};

static const char test1_in[] = " vi ";
static const char *test1_out[] = { "vi", NULL };
static const char test2_in[] = "vi -r ";
static const char *test2_out[] = { "vi", "-r", NULL };
static const char test3_in[] = "vi -r  -R abc\tdef ";
static const char *test3_out[] = { "vi", "-r", "-R", "abc", "def", NULL };
static const char test4_in[] = "vi -r  -R abc\tdef ";
static const char *test4_out[] = { "vi", "-r", "-R", "abc", NULL };
static const char test5_in[] = "";
static const char *test5_out[] = { NULL };

static struct strsplit_test test_data[] = {
    { test1_in, sizeof(test1_in) - 1, test1_out },
    { test2_in, sizeof(test2_in) - 1, test2_out },
    { test3_in, sizeof(test3_in) - 1, test3_out },
    { test4_in, sizeof(test4_in) - 5, test4_out },
    { test5_in, sizeof(test5_in) - 1, test5_out },
    { NULL, 0, NULL }
};

int
main(int argc, char *argv[])
{
    const char *cp, *ep, *input_end;
    int ch, i, j, errors = 0, ntests = 0;
    size_t len;

    initprogname(argc > 0 ? argv[0] : "strsplit_test");

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

    for (i = 0; test_data[i].input != NULL; i++) {
	input_end = test_data[i].input + test_data[i].input_len;
	cp = sudo_strsplit(test_data[i].input, input_end, " \t", &ep);
	for (j = 0; test_data[i].output[j] != NULL; j++) {
	    ntests++;
	    len = strlen(test_data[i].output[j]);
	    if ((size_t)(ep - cp) != len) {
		sudo_warnx_nodebug("failed test #%d: bad length, expected "
		    "%zu, got %zu", ntests, len, (size_t)(ep - cp));
		errors++;
		continue;
	    }
	    ntests++;
	    if (strncmp(cp, test_data[i].output[j], len) != 0) {
		sudo_warnx_nodebug("failed test #%d: expected %s, got %.*s",
		    ntests, test_data[i].output[j], (int)(ep - cp), cp);
		errors++;
		continue;
	    }
	    cp = sudo_strsplit(NULL, input_end, " \t", &ep);
	}
	ntests++;
	if (cp != NULL) {
	    sudo_warnx_nodebug("failed test #%d: extra tokens \"%.*s\"",
		ntests, (int)(input_end - cp), cp);
	    errors++;
	}
    }
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
