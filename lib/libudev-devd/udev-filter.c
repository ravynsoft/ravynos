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

#include <sys/types.h>
#include <sys/queue.h>

#include <fnmatch.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libudev.h"
#include "udev-device.h"
#include "udev-utils.h"
#include "udev-filter.h"

struct udev_filter_entry {
	int type;
	int neg;
	STAILQ_ENTRY(udev_filter_entry) next;
	char *value;
	char expr[];
};

void
udev_filter_init(struct udev_filter_head *ufh)
{

	STAILQ_INIT(ufh);
}

int
udev_filter_add(struct udev_filter_head *ufh, int type, int neg,
    const char *expr, const char *value)
{
	struct udev_filter_entry *ufe;
	size_t exprlen, valuelen;

	exprlen = strlen(expr) + 1;
	valuelen = value == NULL ? 0 : strlen(value) + 1;

	ufe = calloc
	    (1, offsetof(struct udev_filter_entry, expr) + exprlen + valuelen);
	if (ufe == NULL)
		return (-1);

	ufe->type = type;
	ufe->neg = neg;
	strcpy(ufe->expr, expr);
	ufe->value = NULL;
	if (value != NULL) {
		ufe->value = ufe->expr + exprlen;
		strcpy(ufe->value, value);
	}
	STAILQ_INSERT_TAIL(ufh, ufe, next);
	return (0);
}

void
udev_filter_free(struct udev_filter_head *ufh)
{
	struct udev_filter_entry *ufe1, *ufe2;

	ufe1 = STAILQ_FIRST(ufh);
	while (ufe1 != NULL) {
		ufe2 = STAILQ_NEXT(ufe1, next);
		free(ufe1);
		ufe1 = ufe2;
	}
	STAILQ_INIT(ufh);
}

static bool
fnmatch_list(struct udev_list *list, struct udev_filter_entry *ufe)
{
	struct udev_list_entry *entry;
	const char *key, *value;

	udev_list_entry_foreach(entry, udev_list_entry_get_first(list)) {
		key = _udev_list_entry_get_name(entry);
		if (fnmatch(ufe->expr, key, 0) == 0) {
			value = _udev_list_entry_get_value(entry);
			if (ufe->value == NULL && value == NULL)
				return (true);
			if (ufe->value != NULL && value != NULL &&
			    fnmatch(ufe->value, value, 0) == 0)
				return (true);
		}
	}
	return (false);
}

bool
udev_filter_match(struct udev *udev, struct udev_filter_head *ufh,
    const char *syspath)
{
	struct udev_filter_entry *ufe;
	struct udev_device *ud = NULL;
	const char *subsystem, *sysname;
	int ret;

	subsystem = get_subsystem_by_syspath(syspath);
	if (strcmp(subsystem, UNKNOWN_SUBSYSTEM) == 0)
		return (0);

	sysname = get_sysname_by_syspath(syspath);
	/* An empty filter list accepts everything. */
	ret = STAILQ_EMPTY(ufh);

	STAILQ_FOREACH(ufe, ufh, next) {
		if (ufe->type == UDEV_FILTER_TYPE_SUBSYSTEM &&
		    ufe->neg == 0 &&
		    fnmatch(ufe->expr, subsystem, 0) == 0) {
			ret = true;
			break;
		}
		if (ufe->type == UDEV_FILTER_TYPE_SYSNAME &&
		    ufe->neg == 0 &&
		    fnmatch(ufe->expr, sysname, 0) == 0) {
			ret = true;
			break;
		}
		if (ufe->type == UDEV_FILTER_TYPE_PROPERTY && ufe->neg == 0) {
			ud = udev_device_new_common(udev, syspath, UD_ACTION_NONE);
			if (ud == NULL)
				break;
			if (fnmatch_list(
			    udev_device_get_properties_list(ud), ufe)) {
				ret = true;
				break;
			}
		}
		if (ufe->type == UDEV_FILTER_TYPE_SYSATTR && ufe->neg == 0) {
			if (ud == NULL)
				ud = udev_device_new_common(udev, syspath,
				    UD_ACTION_NONE);
			if (ud == NULL)
				break;
			if (fnmatch_list(
			    udev_device_get_sysattr_list(ud), ufe)) {
				ret = true;
				break;
			}
		}
	}

	if (!ret)
		goto out;

	STAILQ_FOREACH(ufe, ufh, next) {
		if (ufe->type == UDEV_FILTER_TYPE_SUBSYSTEM &&
		    ufe->neg == 1 &&
		    fnmatch(ufe->expr, subsystem, 0) == 0) {
			ret = false;
			break;
		}
		if (ufe->type == UDEV_FILTER_TYPE_SYSNAME &&
		    ufe->neg == 1 &&
		    fnmatch(ufe->expr, sysname, 0) == 0) {
			ret = false;
			break;
		}
		if (ufe->type == UDEV_FILTER_TYPE_SYSATTR && ufe->neg == 1) {
			if (ud == NULL)
				ud = udev_device_new_common(udev, sysname,
				    UD_ACTION_NONE);
			if (ud == NULL)
				break;
			if (fnmatch_list(
			    udev_device_get_sysattr_list(ud), ufe)) {
				ret = false;
				break;
			}
		}
	}

out:
	if (ud != NULL)
		udev_device_unref(ud);

	return (ret);
}

/*
 * Returns true if the given @p subsystem is accepted by the
 * filters applied to the enumerator @p ue.
 */
bool
udev_filter_match_subsystem(struct udev_filter_head *ufh, const char *subsystem)
{
	if (!subsystem)
		return false;

	if (STAILQ_EMPTY(ufh))
		return true;

	struct udev_filter_entry *ufe;

	/* Scan for negative matches */
	STAILQ_FOREACH(ufe, ufh, next) {
		if (ufe->type == UDEV_FILTER_TYPE_SUBSYSTEM &&
			ufe->neg != 0 &&
			fnmatch(ufe->expr, subsystem, 0) == 0) {
			return false;
		}
	}

	/* Not empty, scan for positive matches */
	STAILQ_FOREACH(ufe, ufh, next) {
		if (ufe->type == UDEV_FILTER_TYPE_SUBSYSTEM &&
			ufe->neg == 0 &&
			fnmatch(ufe->expr, subsystem, 0) == 0) {
			return true;
		}
	}

	/* Not empty, matched nothing */
	return false;
}
