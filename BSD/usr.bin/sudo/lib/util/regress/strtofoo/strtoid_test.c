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
#include <errno.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>

sudo_dso_public int main(int argc, char *argv[]);

/* sudo_strtoidx() tests */
static struct strtoidx_data {
    const char *idstr;
    id_t id;
    const char *sep;
    const char *ep;
    int errnum;
} strtoidx_data[] = {
    { "0,1", 0, ",", ",", 0 },
    { "10", 10, NULL, NULL, 0 },
    { "-1", 0, NULL, NULL, EINVAL },
    { "4294967295", 0, NULL, NULL, EINVAL },
    { "4294967296", 0, NULL, NULL, ERANGE },
    { "-2147483649", 0, NULL, NULL, ERANGE },
    { "-2", (id_t)-2, NULL, NULL, 0 },
#if SIZEOF_ID_T != SIZEOF_LONG_LONG
    { "-2", (id_t)4294967294U, NULL, NULL, 0 },
#endif
    { "4294967294", (id_t)4294967294U, NULL, NULL, 0 },
    { NULL, 0, NULL, NULL, 0 }
};

/*
 * Simple tests for sudo_strtoidx()
 */
int
main(int argc, char *argv[])
{
    int ch, errors = 0, ntests = 0;
    struct strtoidx_data *d;
    const char *errstr;
    char *ep;
    id_t value;

    initprogname(argc > 0 ? argv[0] : "strtoid_test");

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

    for (d = strtoidx_data; d->idstr != NULL; d++) {
	ntests++;
	errstr = "some error";
	value = sudo_strtoidx(d->idstr, d->sep, &ep, &errstr);
	if (d->errnum != 0) {
	    if (errstr == NULL) {
		sudo_warnx_nodebug("FAIL: %s: missing errstr for errno %d",
		    d->idstr, d->errnum);
		errors++;
	    } else if (value != 0) {
		sudo_warnx_nodebug("FAIL: %s should return 0 on error",
		    d->idstr);
		errors++;
	    } else if (errno != d->errnum) {
		sudo_warnx_nodebug("FAIL: %s: errno mismatch, %d != %d",
		    d->idstr, errno, d->errnum);
		errors++;
	    }
	} else if (errstr != NULL) {
	    sudo_warnx_nodebug("FAIL: %s: %s", d->idstr, errstr);
	    errors++;
	} else if (value != d->id) {
	    sudo_warnx_nodebug("FAIL: %s != %u", d->idstr, (unsigned int)d->id);
	    errors++;
	} else if (d->ep != NULL && ep[0] != d->ep[0]) {
	    sudo_warnx_nodebug("FAIL: ep[0] %d != %d", (int)(unsigned char)ep[0],
		(int)(unsigned char)d->ep[0]);
	    errors++;
	}
    }

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    return errors;
}
