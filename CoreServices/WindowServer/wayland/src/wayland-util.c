/*
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2011 Intel Corporation
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

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "wayland-util.h"
#include "wayland-private.h"

WL_EXPORT void
wl_list_init(struct wl_list *list)
{
	list->prev = list;
	list->next = list;
}

WL_EXPORT void
wl_list_insert(struct wl_list *list, struct wl_list *elm)
{
	elm->prev = list;
	elm->next = list->next;
	list->next = elm;
	elm->next->prev = elm;
}

WL_EXPORT void
wl_list_remove(struct wl_list *elm)
{
	elm->prev->next = elm->next;
	elm->next->prev = elm->prev;
	elm->next = NULL;
	elm->prev = NULL;
}

WL_EXPORT int
wl_list_length(const struct wl_list *list)
{
	struct wl_list *e;
	int count;

	count = 0;
	e = list->next;
	while (e != list) {
		e = e->next;
		count++;
	}

	return count;
}

WL_EXPORT int
wl_list_empty(const struct wl_list *list)
{
	return list->next == list;
}

WL_EXPORT void
wl_list_insert_list(struct wl_list *list, struct wl_list *other)
{
	if (wl_list_empty(other))
		return;

	other->next->prev = list;
	other->prev->next = list->next;
	list->next->prev = other->prev;
	list->next = other->next;
}

WL_EXPORT void
wl_array_init(struct wl_array *array)
{
	memset(array, 0, sizeof *array);
}

WL_EXPORT void
wl_array_release(struct wl_array *array)
{
	free(array->data);
	array->data = WL_ARRAY_POISON_PTR;
}

WL_EXPORT void *
wl_array_add(struct wl_array *array, size_t size)
{
	size_t alloc;
	void *data, *p;

	if (array->alloc > 0)
		alloc = array->alloc;
	else
		alloc = 16;

	while (alloc < array->size + size)
		alloc *= 2;

	if (array->alloc < alloc) {
		if (array->alloc > 0)
			data = realloc(array->data, alloc);
		else
			data = malloc(alloc);

		if (data == NULL)
			return NULL;
		array->data = data;
		array->alloc = alloc;
	}

	p = (char *)array->data + array->size;
	array->size += size;

	return p;
}

WL_EXPORT int
wl_array_copy(struct wl_array *array, struct wl_array *source)
{
	if (array->size < source->size) {
		if (!wl_array_add(array, source->size - array->size))
			return -1;
	} else {
		array->size = source->size;
	}

	if (source->size > 0)
		memcpy(array->data, source->data, source->size);

	return 0;
}

/** \cond */

int
wl_interface_equal(const struct wl_interface *a, const struct wl_interface *b)
{
	/* In most cases the pointer equality test is sufficient.
	 * However, in some cases, depending on how things are split
	 * across shared objects, we can end up with multiple
	 * instances of the interface metadata constants.  So if the
	 * pointers match, the interfaces are equal, if they don't
	 * match we have to compare the interface names.
	 */
	return a == b || strcmp(a->name, b->name) == 0;
}

union map_entry {
	uintptr_t next;
	void *data;
};

#define map_entry_is_free(entry) ((entry).next & 0x1)
#define map_entry_get_data(entry) ((void *)((entry).next & ~(uintptr_t)0x3))
#define map_entry_get_flags(entry) (((entry).next >> 1) & 0x1)

void
wl_map_init(struct wl_map *map, uint32_t side)
{
	memset(map, 0, sizeof *map);
	map->side = side;
}

void
wl_map_release(struct wl_map *map)
{
	wl_array_release(&map->client_entries);
	wl_array_release(&map->server_entries);
}

uint32_t
wl_map_insert_new(struct wl_map *map, uint32_t flags, void *data)
{
	union map_entry *start, *entry;
	struct wl_array *entries;
	uint32_t base;
	uint32_t count;

	if (map->side == WL_MAP_CLIENT_SIDE) {
		entries = &map->client_entries;
		base = 0;
	} else {
		entries = &map->server_entries;
		base = WL_SERVER_ID_START;
	}

	if (map->free_list) {
		start = entries->data;
		entry = &start[map->free_list >> 1];
		map->free_list = entry->next;
	} else {
		entry = wl_array_add(entries, sizeof *entry);
		if (!entry)
			return 0;
		start = entries->data;
	}

	/* wl_array only grows, so if we have too many objects at
	 * this point there's no way to clean up. We could be more
	 * pro-active about trying to avoid this allocation, but
	 * it doesn't really matter because at this point there is
	 * nothing to be done but disconnect the client and delete
	 * the whole array either way.
	 */
	count = entry - start;
	if (count > WL_MAP_MAX_OBJECTS) {
		/* entry->data is freshly malloced garbage, so we'd
		 * better make it a NULL so wl_map_for_each doesn't
		 * dereference it later. */
		entry->data = NULL;
		errno = ENOSPC;
		return 0;
	}
	entry->data = data;
	entry->next |= (flags & 0x1) << 1;

	return count + base;
}

int
wl_map_insert_at(struct wl_map *map, uint32_t flags, uint32_t i, void *data)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START) {
		entries = &map->client_entries;
	} else {
		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	if (i > WL_MAP_MAX_OBJECTS) {
		errno = ENOSPC;
		return -1;
	}

	count = entries->size / sizeof *start;
	if (count < i) {
		errno = EINVAL;
		return -1;
	}

	if (count == i) {
		if (!wl_array_add(entries, sizeof *start))
			return -1;
	}

	start = entries->data;
	start[i].data = data;
	start[i].next |= (flags & 0x1) << 1;

	return 0;
}

int
wl_map_reserve_new(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START) {
		if (map->side == WL_MAP_CLIENT_SIDE) {
			errno = EINVAL;
			return -1;
		}

		entries = &map->client_entries;
	} else {
		if (map->side == WL_MAP_SERVER_SIDE) {
			errno = EINVAL;
			return -1;
		}

		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	if (i > WL_MAP_MAX_OBJECTS) {
		errno = ENOSPC;
		return -1;
	}

	count = entries->size / sizeof *start;
	if (count < i) {
		errno = EINVAL;
		return -1;
	}

	if (count == i) {
		if (!wl_array_add(entries, sizeof *start))
			return -1;

		start = entries->data;
		start[i].data = NULL;
	} else {
		start = entries->data;
		if (start[i].data != NULL) {
			errno = EINVAL;
			return -1;
		}
	}

	return 0;
}

void
wl_map_remove(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START) {
		if (map->side == WL_MAP_SERVER_SIDE)
			return;

		entries = &map->client_entries;
	} else {
		if (map->side == WL_MAP_CLIENT_SIDE)
			return;

		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	start = entries->data;
	start[i].next = map->free_list;
	map->free_list = (i << 1) | 1;
}

void *
wl_map_lookup(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START) {
		entries = &map->client_entries;
	} else {
		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	start = entries->data;
	count = entries->size / sizeof *start;

	if (i < count && !map_entry_is_free(start[i]))
		return map_entry_get_data(start[i]);

	return NULL;
}

uint32_t
wl_map_lookup_flags(struct wl_map *map, uint32_t i)
{
	union map_entry *start;
	uint32_t count;
	struct wl_array *entries;

	if (i < WL_SERVER_ID_START) {
		entries = &map->client_entries;
	} else {
		entries = &map->server_entries;
		i -= WL_SERVER_ID_START;
	}

	start = entries->data;
	count = entries->size / sizeof *start;

	if (i < count && !map_entry_is_free(start[i]))
		return map_entry_get_flags(start[i]);

	return 0;
}

static enum wl_iterator_result
for_each_helper(struct wl_array *entries, wl_iterator_func_t func, void *data)
{
	enum wl_iterator_result ret = WL_ITERATOR_CONTINUE;
	union map_entry entry, *start;
	size_t count;

	start = (union map_entry *) entries->data;
	count = entries->size / sizeof(union map_entry);

	for (size_t idx = 0; idx < count; idx++) {
		entry = start[idx];
		if (entry.data && !map_entry_is_free(entry)) {
			ret = func(map_entry_get_data(entry), data, map_entry_get_flags(entry));
			if (ret != WL_ITERATOR_CONTINUE)
				break;
		}
	}

	return ret;
}

void
wl_map_for_each(struct wl_map *map, wl_iterator_func_t func, void *data)
{
	enum wl_iterator_result ret;

	ret = for_each_helper(&map->client_entries, func, data);
	if (ret == WL_ITERATOR_CONTINUE)
		for_each_helper(&map->server_entries, func, data);
}

static void
wl_log_stderr_handler(const char *fmt, va_list arg)
{
	vfprintf(stderr, fmt, arg);
}

wl_log_func_t wl_log_handler = wl_log_stderr_handler;

void
wl_log(const char *fmt, ...)
{
	va_list argp;

	va_start(argp, fmt);
	wl_log_handler(fmt, argp);
	va_end(argp);
}

void
wl_abort(const char *fmt, ...)
{
	va_list argp;

	va_start(argp, fmt);
	wl_log_handler(fmt, argp);
	va_end(argp);

	abort();
}

/** \endcond */
