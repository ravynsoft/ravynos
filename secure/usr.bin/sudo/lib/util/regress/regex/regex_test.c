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

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_fatal.h>
#include <sudo_util.h>

sudo_dso_public int main(int argc, char *argv[]);

struct regex_test {
    const char *pattern;
    bool result;
};

static struct regex_test test_data[] = {
    { "ab++", false },
    { "ab\\++", true },
    { "ab+\\+", true },
    { "ab**", false },
    { "ab\\**", true },
    { "ab*\\*", true },
    { "ab??", false },
    { "ab\\??", true },
    { "ab?\\?", true },
    { "ab{1}{1}", false },
    { "ab{1}{1,1}", false },
    { "ab{1}{,1}", false },
    { "ab{1}{1,}", false },
    { "ab{1}\\{1}", true },
    { "ab{1}\\{1,1}", true },
    { "ab{1}\\{,1}", true },
    { "ab{1}\\{1,}", true },
    { "ab+*", false },
    { "ab\\+*", true },
    { "ab+\\*", true },
    { "ab*+", false },
    { "ab\\*+", true },
    { "ab*\\+", true },
    { "ab?*", false },
    { "ab\\?*", true },
    { "ab?\\*", true },
    { "ab{1}*", false },
    { "ab\\{1}*", true },
    { "ab{1}\\*", true },
    { "ab{256}", false },
    { "ab{,256}", false },
    { "ab{256,}", false },
    { "ab{1,256}", false },
    { "ab{1,\\256}", false },
    { "ab{1,2\\56}", false },
    { "ab{256,1}", false },
    { "ab{\\256,1}", false },
    { "ab{2\\56,1}", false },
    { NULL }
};

int
main(int argc, char *argv[])
{
    struct regex_test *td;
    const char *errstr;
    int errors = 0, ntests = 0;
    bool result;
    int ch;

    initprogname(argc > 0 ? argv[0] : "regex_test");

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

    for (td = test_data; td->pattern != NULL; td++) {
	ntests++;
	result = sudo_regex_compile(NULL, td->pattern, &errstr);
	if (result != td->result) {
	    sudo_warnx("%s: expected %d, got %d", td->pattern, (int)td->result,
		(int)result);
	    errors++;
	}
    }

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
