/*
 * Copyright 2011 Tom Stellard <tstellar@gmail.com>
 *
 * All Rights Reserved.
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
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "unit_test.h"

unsigned run_tests(struct test tests[])
{
	int i;
	unsigned pass = 1;
	for (i = 0; tests[i].name; i++) {
		printf("Test %s\n", tests[i].name);
		memset(&tests[i].result, 0, sizeof(tests[i].result));
		tests[i].test_func(&tests[i].result);
		printf("Test %s (%d/%d) pass\n", tests[i].name,
			tests[i].result.pass, tests[i].result.test_count);
		if (tests[i].result.pass != tests[i].result.test_count) {
			pass = 0;
		}
	}
	return pass;
}

void test_begin(struct test_result * result)
{
	result->test_count++;
}

void test_check(struct test_result * result, int cond)
{
	printf("Subtest %u -> ", result->test_count);
	if (cond) {
		result->pass++;
		printf("Pass");
	} else {
		result->fail++;
		printf("Fail");
	}
	printf("\n");
}
