/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <asl.h>
#include <asl_core.h>
#include <asl_msg.h>
#include <asl_msg_list.h>

asl_msg_list_t *
asl_msg_list_new(void)
{
	asl_msg_list_t *out = (asl_msg_list_t *)calloc(1, sizeof(asl_msg_list_t));
	if (out == NULL) return NULL;

	out->asl_type = ASL_TYPE_LIST;
	out->refcount = 1;

	return out;
}

asl_msg_list_t *
asl_msg_list_new_count(uint32_t n)
{
	asl_msg_list_t *out = (asl_msg_list_t *)calloc(1, sizeof(asl_msg_list_t));
	if (out == NULL) return NULL;

	out->asl_type = ASL_TYPE_LIST;
	out->refcount = 1;
	out->count = n;

	out->msg = (asl_msg_t **)reallocf(out->msg, out->count * sizeof(asl_msg_t *));
	if (out->msg == NULL)
	{
		free(out);
		return NULL;
	}

	return out;
}

asl_msg_list_t *
asl_msg_list_retain(asl_msg_list_t *list)
{
	if (list == NULL) return NULL;
	asl_retain((asl_object_t)list);
	return list;
}

void
asl_msg_list_release(asl_msg_list_t *list)
{
	if (list == NULL) return;
	asl_release((asl_object_t)list);
}

char *
asl_msg_list_to_string(asl_msg_list_t *list, uint32_t *len)
{
	uint32_t i;
	char tmp[16];
	char *out;
	asl_string_t *str;

	if (list == NULL) return NULL;
	if (list->count == 0) return NULL;
	if (list->msg == NULL) return NULL;

	str = asl_string_new(ASL_ENCODE_ASL);
	if (str == NULL) return NULL;

	snprintf(tmp, sizeof(tmp), "%u", list->count);
	asl_string_append(str, tmp);
	asl_string_append_char_no_encoding(str, '\n');

	for (i = 0; i < list->count; i++)
	{
		asl_string_append_asl_msg(str, list->msg[i]);
		asl_string_append_char_no_encoding(str, '\n');
	}

	*len = asl_string_length(str);
	out = asl_string_release_return_bytes(str);
	return out;
}

asl_string_t *
asl_msg_list_to_asl_string(asl_msg_list_t *list, uint32_t encoding)
{
	uint32_t i;
	char tmp[16];
	asl_string_t *str;

	if (list == NULL) return NULL;
	if (list->count == 0) return NULL;
	if (list->msg == NULL) return NULL;

	str = asl_string_new(encoding);
	if (str == NULL) return NULL;

	snprintf(tmp, sizeof(tmp), "%u", list->count);
	asl_string_append(str, tmp);
	asl_string_append_char_no_encoding(str, '\n');

	for (i = 0; i < list->count; i++)
	{
		asl_string_append_asl_msg(str, list->msg[i]);
		asl_string_append_char_no_encoding(str, '\n');
	}

	return str;
}

asl_msg_list_t *
asl_msg_list_from_string(const char *buf)
{
	uint32_t i, n;
	const char *p;
	asl_msg_list_t *out;
	asl_msg_t *m;

	if (buf == NULL) return NULL;
	p = buf;

	n = atoi(buf);
	if (n == 0) return NULL;

	out = asl_msg_list_new();
	if (out == NULL) return NULL;

	for (i = 0; i < n; i++)
	{
		p = strchr(p, '\n');
		if (p == NULL)
		{
			asl_msg_list_release(out);
			return NULL;
		}

		p++;

		m = asl_msg_from_string(p);
		if (m == NULL)
		{
			asl_msg_list_release(out);
			return NULL;
		}

		asl_msg_list_append(out, m);
		asl_msg_release(m);
	}

	return out;
}

void
asl_msg_list_insert(asl_msg_list_t *list, uint32_t x, void *obj)
{
	uint32_t i, j;
	asl_object_private_t *oo = (asl_object_private_t *)obj;

	if (list == NULL) return;
	if (obj == NULL) return;
	if (list->count == UINT32_MAX) return;

	if (x >= list->count) x = list->count;

	uint32_t type = asl_get_type((asl_object_t)oo);
	uint32_t count = 0;

	if ((type == ASL_TYPE_MSG) || (type == ASL_TYPE_QUERY)) count = 1;
	else count = asl_object_count(oo);

	if (count == 0) return;

	uint64_t check = list->count;
	check += count;
	if (check > UINT32_MAX) return;

	list->msg = (asl_msg_t **)reallocf(list->msg, (list->count + count) * sizeof(asl_msg_t *));
	if (list->msg == NULL)
	{
		list->count = 0;
		list->curr = 0;
		return;
	}

	for (i = list->count, j = i - 1; i > x; i--, j--) list->msg[i] = list->msg[j];

	asl_object_set_iteration_index(oo, 0);

	if ((type == ASL_TYPE_MSG) || (type == ASL_TYPE_QUERY))
	{
		list->msg[x] = (asl_msg_t *)asl_retain((asl_object_t)oo);
	}
	else
	{
		for (i = x, j = 0; j < count; i++, j++) list->msg[i] = (asl_msg_t *)asl_object_next(oo);
	}

	asl_object_set_iteration_index(oo, 0);

	list->count += count;
}

void
asl_msg_list_append(asl_msg_list_t *list, void *obj)
{
	asl_msg_list_insert(list, UINT32_MAX, obj);
}

void
asl_msg_list_prepend(asl_msg_list_t *list, void *obj)
{
	asl_msg_list_insert(list, 0, obj);
}

size_t
asl_msg_list_count(asl_msg_list_t *list)
{
	if (list == NULL) return 0;
	return list->count;
}

asl_msg_t *
asl_msg_list_get_index(asl_msg_list_t *list, size_t index)
{
	asl_msg_t *out;

	if (list == NULL) return NULL;
	if (index >= list->count) return NULL;
	if (list->msg == NULL)
	{
		list->curr = 0;
		list->count = 0;
		return NULL;
	}

	out = list->msg[index];
	return out;
}

void
asl_msg_list_remove_index(asl_msg_list_t *list, size_t index)
{
	uint32_t i, j;

	if (list == NULL) return;
	if (index >= list->count) return;
	if (list->msg == NULL)
	{
		list->curr = 0;
		list->count = 0;
		return;
	}

	asl_msg_release(list->msg[index]);

	for (i = index + 1, j = index; i < list->count; i++) list->msg[j] = list->msg[i];
	list->count--;

	list->msg = (asl_msg_t **)reallocf(list->msg, list->count * sizeof(asl_msg_t *));
	if (list->msg == NULL)
	{
		list->count = 0;
		list->curr = 0;
	}
}

asl_msg_t *
asl_msg_list_next(asl_msg_list_t *list)
{
	asl_msg_t *out;

	if (list == NULL) return NULL;
	if (list->curr >= list->count) return NULL;
	if (list->msg == NULL)
	{
		list->curr = 0;
		list->count = 0;
		return NULL;
	}

	out = list->msg[list->curr];
	list->curr++;
	return out;
}

asl_msg_t *
asl_msg_list_prev(asl_msg_list_t *list)
{
	asl_msg_t *out;

	if (list == NULL) return NULL;
	if (list->curr == 0) return NULL;
	if (list->msg == NULL)
	{
		list->curr = 0;
		list->count = 0;
		return NULL;
	}

	if (list->curr > list->count) list->curr = list->count;

	list->curr--;
	out = list->msg[list->curr];
	return out;
}

void
asl_msg_list_reset_iteration(asl_msg_list_t *list, size_t position)
{
	if (list == NULL) return;

	if (position > list->count) position = SIZE_MAX;
	list->curr = position;
}

asl_msg_list_t *
asl_msg_list_search(asl_msg_list_t *list, asl_msg_t *query)
{
	uint32_t i;
	asl_msg_list_t *out = NULL;

	if (list == NULL) return NULL;

	if (list->msg == NULL)
	{
		list->curr = 0;
		list->count = 0;
		return NULL;
	}

	for (i = 0; i < list->count; i++)
	{
		int match = 0;
		if (query == NULL) match = 1;
		else match = asl_msg_cmp(query, list->msg[i]);

		if (match != 0)
		{
			if (out == NULL) out = asl_msg_list_new();
			if (out == NULL) return NULL;
			asl_msg_list_append(out, list->msg[i]);
		}
	}

	return out;
}

asl_msg_list_t *
asl_msg_list_match(asl_msg_list_t *list, asl_msg_list_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t direction)
{
	uint32_t i, end, n = 0;
	struct timeval now, finish;
	asl_msg_list_t *out = NULL;

	if (list == NULL) return NULL;
	if (list->msg == NULL)
	{
		list->curr = 0;
		list->count = 0;
		return NULL;
	}

	/* start the timer if a timeout was specified */
	memset(&finish, 0, sizeof(struct timeval));
	if (duration != 0)
	{
		if (gettimeofday(&finish, NULL) == 0)
		{
			finish.tv_sec += (duration / USEC_PER_SEC);
			finish.tv_usec += (duration % USEC_PER_SEC);
			if (finish.tv_usec > USEC_PER_SEC)
			{
				finish.tv_usec -= USEC_PER_SEC;
				finish.tv_sec += 1;
			}
		}
		else
		{
			/* shouldn't happen, but if gettimeofday failed we just run without a timeout */
			memset(&finish, 0, sizeof(struct timeval));
		}
	}

	end = list->count - 1;
	if (direction >= 0)
	{
		if (start >= list->count)
		{
			if (last != NULL) *last = list->count;
			return 0;
		}

		direction = 1;
	}
	else
	{
		if (start >= list->count) start = list->count - 1;
		end = 0;
		direction = -1;
	}

	i = start;

	do
	{
		int match = 0;
		if (qlist == NULL) match = 1;
		else match = asl_msg_cmp_list(list->msg[i], qlist);

		if (last != NULL) *last = i;

		if (match != 0)
		{
			if (out == NULL) out = asl_msg_list_new();
			if (out == NULL) return NULL;

			asl_msg_list_append(out, list->msg[i]);
			n++;
		}

		if (n >= count) return n;

		/* check the timer */
		if ((finish.tv_sec != 0) && (gettimeofday(&now, NULL) == 0))
		{
			if ((now.tv_sec > finish.tv_sec) || ((now.tv_sec == finish.tv_sec) && (now.tv_usec > finish.tv_usec))) return n;
		}

		i += direction;
	} while (i != end);

	return out;
}

#pragma mark -
#pragma mark asl_object support

static asl_object_private_t *
_jump_alloc(uint32_t type)
{
	return (asl_object_private_t *)asl_msg_list_new();
}

static void
_jump_dealloc(asl_object_private_t *obj)
{
	asl_msg_list_t *list = (asl_msg_list_t *)obj;

	if (list == NULL) return;
	if (list->msg != NULL)
	{
		uint32_t i;
		for (i = 0; i < list->count; i++) asl_msg_release(list->msg[i]);
		free(list->msg);
	}

	free(list);
}

static size_t
_jump_count(asl_object_private_t *obj)
{
	return asl_msg_list_count((asl_msg_list_t *)obj);
}

static asl_object_private_t *
_jump_next(asl_object_private_t *obj)
{
	return (asl_object_private_t *)asl_msg_list_next((asl_msg_list_t *)obj);
}

static asl_object_private_t *
_jump_prev(asl_object_private_t *obj)
{
	return (asl_object_private_t *)asl_msg_list_prev((asl_msg_list_t *)obj);
}

static asl_object_private_t *
_jump_get_object_at_index(asl_object_private_t *obj, size_t n)
{
	return (asl_object_private_t *)asl_msg_list_get_index((asl_msg_list_t *)obj, n);
}

static void
_jump_set_iteration_index(asl_object_private_t *obj, size_t n)
{
	asl_msg_list_reset_iteration((asl_msg_list_t *)obj, n);
}

static void
_jump_remove_object_at_index(asl_object_private_t *obj, size_t n)
{
	asl_msg_list_remove_index((asl_msg_list_t *)obj, n);
}

static void
_jump_append(asl_object_private_t *obj, asl_object_private_t *newobj, void *addr)
{
	int type = asl_get_type((asl_object_t)newobj);
	if ((type != ASL_TYPE_QUERY) && (type != ASL_TYPE_MSG)) return;

	asl_msg_list_append((asl_msg_list_t *)obj, newobj);
}

static void
_jump_prepend(asl_object_private_t *obj, asl_object_private_t *newobj)
{
	int type = asl_get_type((asl_object_t)newobj);
	if ((type != ASL_TYPE_QUERY) && (type != ASL_TYPE_MSG)) return;

	asl_msg_list_prepend((asl_msg_list_t *)obj, newobj);
}

static asl_object_private_t *
_jump_search(asl_object_private_t *obj, asl_object_private_t *query)
{
	int type = asl_get_type((asl_object_t)query);
	
	if ((query != NULL) && (type != ASL_TYPE_QUERY) && (type != ASL_TYPE_MSG)) return NULL;

	asl_msg_list_t *out = asl_msg_list_search((asl_msg_list_t *)obj, (asl_msg_t *)query);
	if (out == NULL) return NULL;
	return (asl_object_private_t *)out;
}

static asl_object_private_t *
_jump_match(asl_object_private_t *obj, asl_object_private_t *qlist,  size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir)
{
	int type = asl_get_type((asl_object_t)qlist);

	if ((qlist != NULL) && (type != ASL_TYPE_LIST)) return NULL;

	return (asl_object_private_t *)asl_msg_list_match((asl_msg_list_t *)obj, (asl_msg_list_t *)qlist, last, start, count, duration, dir);
}

__private_extern__ const asl_jump_table_t *
asl_msg_list_jump_table()
{
	static const asl_jump_table_t jump =
	{
		.alloc = &_jump_alloc,
		.dealloc = &_jump_dealloc,
		.set_key_val_op = NULL,
		.unset_key = NULL,
		.get_val_op_for_key = NULL,
		.get_key_val_op_at_index = NULL,
		.count = &_jump_count,
		.next = &_jump_next,
		.prev = &_jump_prev,
		.get_object_at_index = &_jump_get_object_at_index,
		.set_iteration_index = &_jump_set_iteration_index,
		.remove_object_at_index = &_jump_remove_object_at_index,
		.append = &_jump_append,
		.prepend = &_jump_prepend,
		.search = &_jump_search,
		.match = &_jump_match
	};

	return &jump;
}
