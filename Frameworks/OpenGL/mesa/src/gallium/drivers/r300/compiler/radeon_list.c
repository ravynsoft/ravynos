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

#include "radeon_list.h"

#include <stdlib.h>
#include <stdio.h>

#include "memory_pool.h"

struct rc_list * rc_list(struct memory_pool * pool, void * item)
{
	struct rc_list * new = memory_pool_malloc(pool, sizeof(struct rc_list));
	new->Item = item;
	new->Next = NULL;
	new->Prev = NULL;

	return new;
}

void rc_list_add(struct rc_list ** list, struct rc_list * new_value)
{
	struct rc_list * temp;

	if (*list == NULL) {
		*list = new_value;
		return;
	}

	for (temp = *list; temp->Next; temp = temp->Next);

	temp->Next = new_value;
	new_value->Prev = temp;
}

void rc_list_remove(struct rc_list ** list, struct rc_list * rm_value)
{
	if (*list == rm_value) {
		*list = rm_value->Next;
		return;
	}

	rm_value->Prev->Next = rm_value->Next;
	if (rm_value->Next) {
		rm_value->Next->Prev = rm_value->Prev;
	}
}

unsigned int rc_list_count(struct rc_list * list)
{
	unsigned int count = 0;
	while (list) {
		count++;
		list = list->Next;
	}
	return count;
}

void rc_list_print(struct rc_list * list)
{
	while(list) {
		fprintf(stderr, "%p->", list->Item);
		list = list->Next;
	}
	fprintf(stderr, "\n");
}
