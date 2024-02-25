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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "wayland-private.h"
#include "test-runner.h"

TEST(list_init)
{
	struct wl_list list;

	wl_list_init(&list);
	assert(list.next == &list);
	assert(list.prev == &list);
	assert(wl_list_empty(&list));
}

struct element {
	int i;
	struct wl_list link;
};

TEST(list_insert)
{
	struct wl_list list;
	struct element e;

	wl_list_init(&list);
	wl_list_insert(&list, &e.link);
	assert(list.next == &e.link);
	assert(list.prev == &e.link);
	assert(e.link.next == &list);
	assert(e.link.prev == &list);
}

TEST(list_length)
{
	struct wl_list list;
	struct element e;

	wl_list_init(&list);
	assert(wl_list_length(&list) == 0);
	wl_list_insert(&list, &e.link);
	assert(wl_list_length(&list) == 1);
	wl_list_remove(&e.link);
	assert(wl_list_length(&list) == 0);
}

TEST(list_iterator)
{
	struct wl_list list;
	struct element e1, e2, e3, e4, *e;
	int reference[] = { 708090, 102030, 5588, 12 };
	unsigned int i;

	e1.i = 708090;
	e2.i = 102030;
	e3.i = 5588;
	e4.i = 12;

	wl_list_init(&list);
	wl_list_insert(list.prev, &e1.link);
	wl_list_insert(list.prev, &e2.link);
	wl_list_insert(list.prev, &e3.link);
	wl_list_insert(list.prev, &e4.link);

	i = 0;
	wl_list_for_each(e, &list, link) {
		assert(i < ARRAY_LENGTH(reference));
		assert(e->i == reference[i]);
		i++;
	}
	assert(i == ARRAY_LENGTH(reference));

	i = 0;
	wl_list_for_each_reverse(e, &list, link) {
		assert(i < ARRAY_LENGTH(reference));
		assert(e->i == reference[ARRAY_LENGTH(reference) - i - 1]);
		i++;
	}
	assert(i == ARRAY_LENGTH(reference));
}

static int
validate_list(struct wl_list *list, int *reference, int length)
{
	struct element *e;
	int i;

	i = 0;
	wl_list_for_each(e, list, link) {
		if (i >= length)
			return 0;
		if (e->i != reference[i])
			return 0;
		i++;
	}

	if (i != length)
		return 0;

	return 1;
}

TEST(list_remove)
{
	struct wl_list list;
	struct element e1, e2, e3;
	int reference1[] = { 17, 8888, 1000 }, reference2[] = { 17, 1000 };

	e1.i = 17;
	e2.i = 8888;
	e3.i = 1000;

	wl_list_init(&list);
	wl_list_insert(&list, &e1.link);
	wl_list_insert(list.prev, &e2.link);
	wl_list_insert(list.prev, &e3.link);
	assert(validate_list(&list, reference1, ARRAY_LENGTH(reference1)));

	wl_list_remove(&e2.link);
	assert(validate_list(&list, reference2, ARRAY_LENGTH(reference2)));
}

TEST(list_insert_list)
{
	struct wl_list list, other;
	struct element e1, e2, e3, e4, e5, e6;
	int reference1[] = { 17, 8888, 1000 };
	int reference2[] = { 76543, 1, -500 };
	int reference3[] = { 17, 76543, 1, -500, 8888, 1000 };

	e1.i = 17;
	e2.i = 8888;
	e3.i = 1000;

	wl_list_init(&list);
	wl_list_insert(&list, &e1.link);
	wl_list_insert(list.prev, &e2.link);
	wl_list_insert(list.prev, &e3.link);
	assert(validate_list(&list, reference1, ARRAY_LENGTH(reference1)));

	e4.i = 76543;
	e5.i = 1;
	e6.i = -500;

	wl_list_init(&other);
	wl_list_insert(&other, &e4.link);
	wl_list_insert(other.prev, &e5.link);
	wl_list_insert(other.prev, &e6.link);
	assert(validate_list(&other, reference2, ARRAY_LENGTH(reference2)));

	wl_list_insert_list(list.next, &other);
	assert(validate_list(&list, reference3, ARRAY_LENGTH(reference3)));
}
