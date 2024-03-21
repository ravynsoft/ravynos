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
#include <stdint.h>
#include <assert.h>
#include "wayland-private.h"
#include "test-runner.h"

TEST(map_insert_new)
{
	struct wl_map map;
	uint32_t i, j, k, a, b, c;

	wl_map_init(&map, WL_MAP_SERVER_SIDE);
	i = wl_map_insert_new(&map, 0, &a);
	j = wl_map_insert_new(&map, 0, &b);
	k = wl_map_insert_new(&map, 0, &c);
	assert(i == WL_SERVER_ID_START);
	assert(j == WL_SERVER_ID_START + 1);
	assert(k == WL_SERVER_ID_START + 2);

	assert(wl_map_lookup(&map, i) == &a);
	assert(wl_map_lookup(&map, j) == &b);
	assert(wl_map_lookup(&map, k) == &c);
    wl_map_release(&map);

	wl_map_init(&map, WL_MAP_CLIENT_SIDE);
	i = wl_map_insert_new(&map, 0, &a);
	assert(i == 0);
	assert(wl_map_lookup(&map, i) == &a);

	wl_map_release(&map);
}

TEST(map_insert_at)
{
	struct wl_map map;
	uint32_t a, b, c;

	wl_map_init(&map, WL_MAP_CLIENT_SIDE);
	assert(wl_map_insert_at(&map, 0, WL_SERVER_ID_START, &a) == 0);
	assert(wl_map_insert_at(&map, 0, WL_SERVER_ID_START + 3, &b) == -1);
	assert(wl_map_insert_at(&map, 0, WL_SERVER_ID_START + 1, &c) == 0);

	assert(wl_map_lookup(&map, WL_SERVER_ID_START) == &a);
	assert(wl_map_lookup(&map, WL_SERVER_ID_START + 1) == &c);

	wl_map_release(&map);
}

TEST(map_remove)
{
	struct wl_map map;
	uint32_t i, j, k, l, a, b, c, d;

	wl_map_init(&map, WL_MAP_SERVER_SIDE);
	i = wl_map_insert_new(&map, 0, &a);
	j = wl_map_insert_new(&map, 0, &b);
	k = wl_map_insert_new(&map, 0, &c);
	assert(i == WL_SERVER_ID_START);
	assert(j == WL_SERVER_ID_START + 1);
	assert(k == WL_SERVER_ID_START + 2);

	assert(wl_map_lookup(&map, i) == &a);
	assert(wl_map_lookup(&map, j) == &b);
	assert(wl_map_lookup(&map, k) == &c);

	wl_map_remove(&map, j);
	assert(wl_map_lookup(&map, j) == NULL);

	/* Verify that we insert d at the hole left by removing b */
	l = wl_map_insert_new(&map, 0, &d);
	assert(l == WL_SERVER_ID_START + 1);
	assert(wl_map_lookup(&map, l) == &d);

	wl_map_release(&map);
}

TEST(map_flags)
{
	struct wl_map map;
	uint32_t i, j, a, b;

	wl_map_init(&map, WL_MAP_SERVER_SIDE);
	i = wl_map_insert_new(&map, 0, &a);
	j = wl_map_insert_new(&map, 1, &b);
	assert(i == WL_SERVER_ID_START);
	assert(j == WL_SERVER_ID_START + 1);

	assert(wl_map_lookup(&map, i) == &a);
	assert(wl_map_lookup(&map, j) == &b);

    assert(wl_map_lookup_flags(&map, i) == 0);
    assert(wl_map_lookup_flags(&map, j) == 1);

	wl_map_release(&map);
}

static enum wl_iterator_result never_run(void *element, void *data, uint32_t flags)
{
	assert(0);
}

TEST(map_iter_empty)
{
	struct wl_map map;

	wl_map_init(&map, WL_MAP_SERVER_SIDE);

	wl_map_for_each(&map, never_run, NULL);

	wl_map_release(&map);
}
