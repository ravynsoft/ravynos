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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "wayland-private.h"
#include "test-runner.h"

TEST(fixed_double_conversions)
{
	wl_fixed_t f;
	double d;

	d = 62.125;
	f = wl_fixed_from_double(d);
	fprintf(stderr, "double %lf to fixed %x\n", d, f);
	assert(f == 0x3e20);

	d = -1200.625;
	f = wl_fixed_from_double(d);
	fprintf(stderr, "double %lf to fixed %x\n", d, f);
	assert(f == -0x4b0a0);

	f = random();
	d = wl_fixed_to_double(f);
	fprintf(stderr, "fixed %x to double %lf\n", f, d);
	assert(d == f / 256.0);

	f = 0x012030;
	d = wl_fixed_to_double(f);
	fprintf(stderr, "fixed %x to double %lf\n", f, d);
	assert(d == 288.1875);

	f = 0x70000000;
	d = wl_fixed_to_double(f);
	fprintf(stderr, "fixed %x to double %lf\n", f, d);
	assert(d == f / 256);

	f = -0x012030;
	d = wl_fixed_to_double(f);
	fprintf(stderr, "fixed %x to double %lf\n", f, d);
	assert(d == -288.1875);

	f = 0x80000000;
	d = wl_fixed_to_double(f);
	fprintf(stderr, "fixed %x to double %lf\n", f, d);
	assert(d == f / 256);
}

TEST(fixed_int_conversions)
{
	wl_fixed_t f;
	int i;

	i = 62;
	f = wl_fixed_from_int(i);
	assert(f == 62 * 256);

	i = -2080;
	f = wl_fixed_from_int(i);
	assert(f == -2080 * 256);

	f = 0x277013;
	i = wl_fixed_to_int(f);
	assert(i == 0x2770);

	f = -0x5044;
	i = wl_fixed_to_int(f);
	assert(i == -0x50);
}
