/*
 * Copyright © 2012 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include "test-runner.h"

static int
parse_count(const char *str, int *value)
{
	char *end;
	long v;

	errno = 0;
	v = strtol(str, &end, 10);
	if ((errno == ERANGE && (v == LONG_MAX || v == LONG_MIN)) ||
	    (errno != 0 && v == 0) ||
	    (end == str) ||
	    (*end != '\0')) {
		return -1;
	}

	if (v < 0 || v > INT_MAX) {
		return -1;
	}

	*value = v;
	return 0;
}

int main(int argc, char *argv[])
{
	int expected;

	if (argc != 2)
		goto help_out;

	if (parse_count(argv[1], &expected) < 0)
		goto help_out;

	if (count_open_fds() == expected)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;

help_out:
	fprintf(stderr, "Usage: %s N\n"
		"where N is the expected number of open file descriptors.\n"
		"This program exits with a failure if the number "
		"does not match exactly.\n", argv[0]);

	return EXIT_FAILURE;
}
