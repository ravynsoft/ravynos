/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudo_compat.h>
#include <sudo_util.h>

/* From parse.h */
extern size_t base64_decode(const char *str, unsigned char *dst, size_t dsize);
extern size_t base64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_len);

sudo_dso_public int main(int argc, char *argv[]);

static unsigned char bstring1[] = { 0xea, 0xb8, 0xa2, 0x71, 0xef, 0x67, 0xc1, 0xcd, 0x0d, 0xd9, 0xa6, 0xaa, 0xa8, 0x24, 0x77, 0x2a, 0xfc, 0x6f, 0x76, 0x37, 0x1b, 0xed, 0x9e, 0x1a, 0x90, 0x5f, 0xcf, 0xbc, 0x00 };

struct base64_test {
    const char *ascii;
    const char *encoded;
} test_strings[] = {
    {
	(char *)bstring1,
	"6riice9nwc0N2aaqqCR3Kvxvdjcb7Z4akF/PvA=="
    },
    {
	"any carnal pleasure.",
	"YW55IGNhcm5hbCBwbGVhc3VyZS4="
    },
    {
	"any carnal pleasure",
	"YW55IGNhcm5hbCBwbGVhc3VyZQ=="
    },
    {
	"any carnal pleasur",
	"YW55IGNhcm5hbCBwbGVhc3Vy"
    },
    {
	"any carnal pleasu",
	"YW55IGNhcm5hbCBwbGVhc3U="
    },
    {
	"any carnal pleas",
	"YW55IGNhcm5hbCBwbGVhcw=="
    }
};

int
main(int argc, char *argv[])
{
    int ch, ntests = nitems(test_strings);
    int i, errors = 0;
    unsigned char buf[64];
    size_t len;

    initprogname(argc > 0 ? argv[0] : "check_base64");

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

    for (i = 0; i < ntests; i++) {
	/* Test decode. */
	len = base64_decode(test_strings[i].encoded, buf, sizeof(buf));
	if (len == (size_t)-1) {
	    fprintf(stderr, "check_base64: failed to decode %s\n",
		test_strings[i].encoded);
	    errors++;
	} else {
	    buf[len] = '\0';
	    if (strcmp(test_strings[i].ascii, (char *)buf) != 0) {
		fprintf(stderr, "check_base64: expected %s, got %s\n",
		    test_strings[i].ascii, buf);
		errors++;
	    }
	}

	/* Test encode. */
	len = base64_encode((unsigned char *)test_strings[i].ascii,
	    strlen(test_strings[i].ascii), (char *)buf, sizeof(buf));
	if (len == (size_t)-1) {
	    fprintf(stderr, "check_base64: failed to encode %s\n",
		test_strings[i].ascii);
	    errors++;
	} else {
	    if (strcmp(test_strings[i].encoded, (char *)buf) != 0) {
		fprintf(stderr, "check_base64: expected %s, got %s\n",
		    test_strings[i].encoded, buf);
		errors++;
	    }
	}
    }
    ntests *= 2;	/* we test in both directions */

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    exit(errors);
}
