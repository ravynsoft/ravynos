/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>

sudo_dso_public int main(int argc, char *argv[]);

/* sudo_strtobool() tests */
static struct strtobool_data {
    const char *bool_str;
    int value;
} strtobool_data[] = {
    { "true", true },
    { "false", false },
    { "TrUe", true },
    { "fAlSe", false },
    { "1", true },
    { "0", false },
    { "on", true },
    { "off", false },
    { "yes", true },
    { "no", false },
    { "nope", -1 },
    { "10", -1 },
    { "one", -1 },
    { "zero", -1 },
    { NULL, 0 }
};

/*
 * Simple tests for sudo_strtobool()
 */
int
main(int argc, char *argv[])
{
    struct strtobool_data *d;
    int errors = 0, ntests = 0;
    int ch, value;

    initprogname(argc > 0 ? argv[0] : "strtobool_test");

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

    for (d = strtobool_data; d->bool_str != NULL; d++) {
	ntests++;
	value = sudo_strtobool(d->bool_str);
	if (value != d->value) {
	    sudo_warnx_nodebug("FAIL: %s != %d", d->bool_str, d->value);
	    errors++;
	}
    }

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    return errors;
}
