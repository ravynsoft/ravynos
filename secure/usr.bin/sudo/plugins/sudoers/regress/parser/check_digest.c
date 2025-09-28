/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sudo_queue.h>
#include <sudo_digest.h>
#include <sudo_util.h>
#include <parse.h>

sudo_dso_public int main(int argc, char *argv[]);

#define NUM_TESTS	8
static const char *test_strings[NUM_TESTS] = {
    "",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789"
	"012345678901234567890",
};

static unsigned char *
check_digest(unsigned int digest_type, const char *buf, size_t buflen,
    size_t *digest_len)
{
    char tfile[] = "digest.XXXXXX";
    unsigned char *digest = NULL;
    int tfd;

    /* Write test data to temporary file. */
    tfd = mkstemp(tfile);
    if (tfd == -1) {
	sudo_warn_nodebug("mkstemp");
	goto done;
    }
    if ((size_t)write(tfd, buf, buflen) != buflen) {
	sudo_warn_nodebug("write");
	goto done;
    }
    lseek(tfd, 0, SEEK_SET);

    /* Get file digest. */
    digest = sudo_filedigest(tfd, tfile, digest_type, digest_len);
    if (digest == NULL) {
	/* Warning (if any) printed by sudo_filedigest() */
	goto done;
    }
done:
    if (tfd != -1) {
	close(tfd);
	unlink(tfile);
    }
    return digest;
}

int
main(int argc, char *argv[])
{
    static const char hex[] = "0123456789abcdef";
    char buf[1000 * 1000];
    unsigned char *digest;
    unsigned int i, j;
    size_t digest_len;
    int ch;
    unsigned int digest_type;

    initprogname(argc > 0 ? argv[0] : "check_digest");

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

    for (digest_type = 0; digest_type < SUDO_DIGEST_INVALID; digest_type++) {
	for (i = 0; i < NUM_TESTS; i++) {
	    digest = check_digest(digest_type, test_strings[i],
		strlen(test_strings[i]), &digest_len);
	    if (digest != NULL) {
		printf("%s (\"%s\") = ", digest_type_to_name(digest_type),
		    test_strings[i]);
		for (j = 0; j < digest_len; j++) {
		    putchar(hex[digest[j] >> 4]);
		    putchar(hex[digest[j] & 0x0f]);
		}
		putchar('\n');
		free(digest);
	    }
	}

	/* Simulate a string of a million 'a' characters. */
	memset(buf, 'a', sizeof(buf));
	digest = check_digest(digest_type, buf, sizeof(buf), &digest_len);
	if (digest != NULL) {
	    printf("%s (one million 'a' characters) = ",
		digest_type_to_name(digest_type));
	    for (j = 0; j < digest_len; j++) {
		putchar(hex[digest[j] >> 4]);
		putchar(hex[digest[j] & 0x0f]);
	    }
	    putchar('\n');
	    free(digest);
	}
    }

    return 0;
}
