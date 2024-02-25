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
#include <assert.h>
#include <string.h>
#include "wayland-util.h"
#include "wayland-private.h"
#include "test-runner.h"

TEST(array_init)
{
	struct wl_array array;

	/* fill with garbage to emulate uninitialized memory */
	memset(&array, 0x57, sizeof array);

	wl_array_init(&array);
	assert(array.size == 0);
	assert(array.alloc == 0);
	assert(array.data == 0);
}

TEST(array_release)
{
	struct wl_array array;
	void *ptr;

	wl_array_init(&array);
	ptr = wl_array_add(&array, 1);
	assert(ptr != NULL);
	assert(array.data != NULL);

	wl_array_release(&array);
	assert(array.data == WL_ARRAY_POISON_PTR);
}

TEST(array_add)
{
	struct mydata {
		unsigned int a;
		unsigned int b;
		double c;
		double d;
	};

	const unsigned int iterations = 1321; /* this is arbitrary */
	const int datasize = sizeof(struct mydata);
	struct wl_array array;
	size_t i;

	wl_array_init(&array);

	/* add some data */
	for (i = 0; i < iterations; i++) {
		struct mydata* ptr = wl_array_add(&array, datasize);
		assert(ptr);
		assert((i + 1) * datasize == array.size);

		ptr->a = i * 3;
		ptr->b = 20000 - i;
		ptr->c = (double)(i);
		ptr->d = (double)(i / 2.);
	}

	/* verify the data */
	for (i = 0; i < iterations; ++i) {
		struct mydata* check = (struct mydata*)array.data + i;

		assert(check->a == i * 3);
		assert(check->b == 20000 - i);
		assert(check->c == (double)(i));
		assert(check->d == (double)(i/2.));
	}

	wl_array_release(&array);
}

TEST(array_copy)
{
	const int iterations = 1529; /* this is arbitrary */
	struct wl_array source;
	struct wl_array copy;
	int i;

	wl_array_init(&source);

	/* add some data */
	for (i = 0; i < iterations; i++) {
		int *p = wl_array_add(&source, sizeof(int));
		assert(p);
		*p = i * 2 + i;
	}

	/* copy the array */
	wl_array_init(&copy);
	wl_array_copy(&copy, &source);

	/* check the copy */
	for (i = 0; i < iterations; i++) {
		int *s = (int *)source.data + i;
		int *c = (int *)copy.data + i;

		assert(*s == *c); /* verify the values are the same */
		assert(s != c); /* ensure the addresses aren't the same */
		assert(*s == i * 2 + i); /* sanity check */
	}

	wl_array_release(&source);
	wl_array_release(&copy);
}

TEST(array_for_each)
{
	static const int elements[] = { 77, 12, 45192, 53280, 334455 };
	struct wl_array array;
	int *p;
	int i;

	wl_array_init(&array);
	for (i = 0; i < 5; i++) {
		p = wl_array_add(&array, sizeof *p);
		assert(p);
		*p = elements[i];
	}

	i = 0;
	wl_array_for_each(p, &array) {
		assert(*p == elements[i]);
		i++;
	}
	assert(i == 5);

	wl_array_release(&array);
}
