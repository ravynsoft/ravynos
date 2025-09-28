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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUDO_ERROR_WRAP 0

#include <sudoers.h>

sudo_dso_public int main(int argc, char *argv[]);

static void
test_serialize_list(int *ntests_out, int *errors_out)
{
    int ntests = *ntests_out;
    int errors = *errors_out;
    const char *expected = "myvar=a value with spaces,this\\,and\\,that,\\,";
    struct list_members members = SLIST_HEAD_INITIALIZER(members);
    struct list_member lm1, lm2, lm3;
    char *result;

    lm1.value = (char *)"a value with spaces";
    lm2.value = (char *)"this,and,that";
    lm3.value = (char *)",";
    SLIST_INSERT_HEAD(&members, &lm3, entries);
    SLIST_INSERT_HEAD(&members, &lm2, entries);
    SLIST_INSERT_HEAD(&members, &lm1, entries);

    ntests++;
    result = serialize_list("myvar", &members);
    if (result == NULL) {
	sudo_warnx("serialize_list returns NULL");
	++errors;
	goto done;
    }
    ntests++;
    if (strcmp(result, expected) != 0) {
	sudo_warnx("got \"%s\", expected \"%s\"", result, expected);
	++errors;
	goto done;
    }

done:
    free(result);
    *ntests_out = ntests;
    *errors_out = errors;
}

int
main(int argc, char *argv[])
{
    int ch, ntests = 0, errors = 0;

    initprogname(argc > 0 ? argv[0] : "check_serialize_list");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignored */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    test_serialize_list(&ntests, &errors);

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    exit(errors);
}
