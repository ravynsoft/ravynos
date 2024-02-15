/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014-2015, 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

sudo_dso_public int main(int argc, char *argv[]);

struct hexchar_test {
    char hex[3];
    int value;
};

int
main(int argc, char *argv[])
{
    struct hexchar_test *test_data;
    int i, ntests, result, errors = 0;
    static const char xdigs_lower[] = "0123456789abcdef";
    static const char xdigs_upper[] = "0123456789ABCDEF";

    initprogname(argc > 0 ? argv[0] : "hexchar_test");

    /* Build up test data. */
    ntests = 256 + 256 + 3;
    test_data = calloc((size_t)ntests, sizeof(*test_data));
    for (i = 0; i < 256; i++) {
	/* lower case */
	test_data[i].value = i;
	test_data[i].hex[1] = xdigs_lower[ (i & 0x0f)];
	test_data[i].hex[0] = xdigs_lower[((i & 0xf0) >> 4)];
	/* upper case */
	test_data[i + 256].value = i;
	test_data[i + 256].hex[1] = xdigs_upper[ (i & 0x0f)];
	test_data[i + 256].hex[0] = xdigs_upper[((i & 0xf0) >> 4)];
    }
    /* Also test invalid data */
    test_data[ntests - 3].hex[0] = '\0';
    test_data[ntests - 3].value = -1;
    strlcpy(test_data[ntests - 2].hex, "AG", sizeof(test_data[ntests - 2].hex));
    test_data[ntests - 2].value = -1;
    strlcpy(test_data[ntests - 1].hex, "-1", sizeof(test_data[ntests - 1].hex));
    test_data[ntests - 1].value = -1;

    for (i = 0; i < ntests; i++) {
	result = sudo_hexchar(test_data[i].hex);
	if (result != test_data[i].value) {
	    fprintf(stderr, "%s: expected %d, got %d\n", getprogname(),
		test_data[i].value, result);
	    errors++;
	}
    }
    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }
    return errors;
}
