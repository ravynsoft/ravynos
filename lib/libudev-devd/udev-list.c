/*
 * Copyright (c) 2015, 2021 Vladimir Kondratyev <vladimir@kondratyev.su>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libudev.h"
#include "udev-list.h"
#include "udev-utils.h"
#include "utils.h"

struct udev_list_entry {
	struct udev_list *list;
	RB_ENTRY(udev_list_entry) link;
	char *value;
	char name[];
};

void udev_list_entry_free(struct udev_list_entry *ule);

RB_PROTOTYPE(udev_list, udev_list_entry, link, udev_list_entry_cmp);

void
udev_list_init(struct udev_list *ul)
{

	RB_INIT(ul);
}

int
udev_list_insert(struct udev_list *ul, char const *name, char const *value)
{
	struct udev_list_entry *ule, *old_ule;
	size_t namelen, valuelen;

	namelen = strlen(name) + 1;
	valuelen = value == NULL ? 0 : strlen(value) + 1;
	ule = calloc
	    (1, offsetof(struct udev_list_entry, name) + namelen + valuelen);
	if (!ule)
		return (-1);

	ule->list = ul;
	strcpy(ule->name, name);
	ule->value = NULL;
	if (value != NULL) {
		ule->value = ule->name + namelen;
		strcpy(ule->value, value);
	}

	old_ule = RB_FIND(udev_list, ul, ule);
	if (old_ule != NULL) {
		RB_REMOVE(udev_list, ul, old_ule);
		udev_list_entry_free(old_ule);
	}

	RB_INSERT(udev_list, ul, ule);
	return (0);
}

void
udev_list_free(struct udev_list *ul)
{
	struct udev_list_entry *ule1, *ule2;

	RB_FOREACH_SAFE (ule1, udev_list, ul, ule2) {
		RB_REMOVE(udev_list, ul, ule1);
		udev_list_entry_free(ule1);
	}

	RB_INIT(ul);
}

void
udev_list_entry_free(struct udev_list_entry *ule)
{

	free(ule);
}

struct udev_list_entry *
udev_list_entry_get_first(struct udev_list *ul)
{

	return (RB_MIN(udev_list, ul));
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_list_entry_get_next(struct udev_list_entry *ule)
{

	return (RB_NEXT(udev_list,, ule));
}

const char *
_udev_list_entry_get_name(struct udev_list_entry *ule)
{

	return (ule->name);
}

LIBUDEV_EXPORT const char *
udev_list_entry_get_name(struct udev_list_entry *ule)
{
	const char *name;

	name = _udev_list_entry_get_name(ule);
	TRC("() %s", name);
	return (name);
}

const char *
_udev_list_entry_get_value(struct udev_list_entry *ule)
{

	return (ule->value);
}

LIBUDEV_EXPORT const char *
udev_list_entry_get_value(struct udev_list_entry *ule)
{
	const char *value;

	value = _udev_list_entry_get_value(ule);
	TRC("() %s", value);
	return (value);
}

static int
udev_list_entry_cmp (struct udev_list_entry *le1, struct udev_list_entry *le2)
{

	return (strcmp(le1->name, le2->name));
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_list_entry_get_by_name(struct udev_list_entry *ule, const char *name)
{
	struct udev_list_entry *find, *ret;

	find = calloc
	    (1, offsetof(struct udev_list_entry, name) + strlen(name) + 1);

	ret = RB_FIND(udev_list, ule->list, find);
	udev_list_entry_free(find);

	return (ret);
}

RB_GENERATE(udev_list, udev_list_entry, link, udev_list_entry_cmp);
