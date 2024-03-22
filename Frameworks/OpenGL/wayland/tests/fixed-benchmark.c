/*
 * Copyright © 2012 Intel Corporation
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
#include <string.h>
#include <time.h>
#include <assert.h>
#include "wayland-private.h"

volatile double global_d;

static void
noop_conversion(void)
{
	wl_fixed_t f;
	union {
		int64_t i;
		double d;
	} u;

	for (f = 0; f < INT32_MAX; f++) {
		u.i = f;
		global_d = u.d;
	}
}

static void
magic_conversion(void)
{
	wl_fixed_t f;

	for (f = 0; f < INT32_MAX; f++)
		global_d = wl_fixed_to_double(f);
}

static void
mul_conversion(void)
{
	wl_fixed_t f;

	/* This will get optimized into multiplication by 1/256 */
	for (f = 0; f < INT32_MAX; f++)
		global_d = f / 256.0;
}

double factor = 256.0;

static void
div_conversion(void)
{
	wl_fixed_t f;

	for (f = 0; f < INT32_MAX; f++)
		global_d = f / factor;
}

static void
benchmark(const char *s, void (*f)(void))
{
	struct timespec start, stop, elapsed;

	clock_gettime(CLOCK_MONOTONIC, &start);
	f();
	clock_gettime(CLOCK_MONOTONIC, &stop);

	elapsed.tv_sec = stop.tv_sec - start.tv_sec;
	elapsed.tv_nsec = stop.tv_nsec - start.tv_nsec;
	if (elapsed.tv_nsec < 0) {
		elapsed.tv_nsec += 1000000000;
		elapsed.tv_sec--;
	}
	printf("benchmarked %s:\t%ld.%09lds\n",
	       s, elapsed.tv_sec, elapsed.tv_nsec);
}

int main(void)
{
	benchmark("noop", noop_conversion);
	benchmark("magic", magic_conversion);
	benchmark("div", div_conversion);
	benchmark("mul", mul_conversion);

	return 0;
}
