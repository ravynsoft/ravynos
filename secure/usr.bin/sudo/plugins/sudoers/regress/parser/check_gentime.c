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
#include <time.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudoers_debug.h>
#include <parse.h>

sudo_dso_public int main(int argc, char *argv[]);

const struct gentime_test {
    const char *gentime;
    time_t unixtime;
} tests[] = {
    { "199412161032ZZ", -1 },
    { "199412161032Z", 787573920 },
    { "199412160532-0500", 787573920 },
    { "199412160532-05000", -1 },
    { "199412160532", 787573920 },		/* local time is EST */
    { "20170214083000-0500", 1487079000 },
    { "201702140830-0500", 1487079000 },
    { "201702140830", 1487079000 },		/* local time is EST */
    { "201702140830.3-0500", 1487079018 },
    { "201702140830,3-0500", 1487079018 },
    { "20170214083000.5Z", 1487061000 },
    { "20170214083000,5Z", 1487061000 },
    { "201702142359.4Z", 1487116764 },
    { "201702142359,4Z", 1487116764 },
    { "2017021408.5Z", 1487061000 },
    { "2017021408,5Z", 1487061000 },
    { "20170214Z", -1 },
};

int
main(int argc, char *argv[])
{
    const int ntests = nitems(tests);
    int ch, i, errors = 0;
    time_t result;

    initprogname(argc > 0 ? argv[0] : "check_gentime");

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

    /* Do local time tests in Eastern Standard Time. */
    putenv((char *)"TZ=EST5EST5");
    tzset();

    for (i = 0; i < ntests; i++) {
	result = parse_gentime(tests[i].gentime);
	if (result != tests[i].unixtime) {
	    fprintf(stderr, "check_gentime[%d]: %s: expected %lld, got %lld\n",
		i, tests[i].gentime,
		(long long)tests[i].unixtime, (long long)result);
	    errors++;
	}
    }
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    exit(errors);
}
