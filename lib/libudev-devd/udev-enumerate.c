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
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libudev.h"
#include "udev-filter.h"
#include "udev-list.h"
#include "udev-utils.h"
#include "utils.h"

struct udev_enumerate {
	int refcount;
	struct udev_filter_head filters;
	struct udev_list dev_list;
	struct udev *udev;
};

LIBUDEV_EXPORT struct udev_enumerate *
udev_enumerate_new(struct udev *udev)
{
	struct udev_enumerate *ue;

	TRC();
	ue = calloc(1, sizeof(struct udev_enumerate));
	if (ue == NULL)
		return (NULL);

	ue->udev = udev;
	udev_ref(udev);
	ue->refcount = 1;
	udev_filter_init(&ue->filters);
	udev_list_init(&ue->dev_list);

	return (ue);
}

LIBUDEV_EXPORT struct udev_enumerate *
udev_enumerate_ref(struct udev_enumerate *ue)
{

	TRC("(%p) refcount=%d", ue, ue->refcount);
	++ue->refcount;
	return (ue);
}

LIBUDEV_EXPORT void
udev_enumerate_unref(struct udev_enumerate *ue)
{

	TRC("(%p) refcount=%d", ue, ue->refcount);
	if (--ue->refcount == 0) {
		udev_filter_free(&ue->filters);
		udev_list_free(&ue->dev_list);
		udev_unref(ue->udev);
		free(ue);
	}
}

LIBUDEV_EXPORT int
udev_enumerate_add_match_subsystem(struct udev_enumerate *ue,
    const char *subsystem)
{

	TRC("(%p, %s)", ue, subsystem);
	return (udev_filter_add(&ue->filters, UDEV_FILTER_TYPE_SUBSYSTEM, 0,
	    subsystem, NULL));
}

LIBUDEV_EXPORT int
udev_enumerate_add_nomatch_subsystem(struct udev_enumerate *ue,
    const char *subsystem)
{

	TRC("(%p, %s)", ue, subsystem);
	return (udev_filter_add(&ue->filters, UDEV_FILTER_TYPE_SUBSYSTEM, 1,
	    subsystem, NULL));
}

LIBUDEV_EXPORT int
udev_enumerate_add_match_sysname(struct udev_enumerate *ue,
    const char *sysname)
{

	TRC("(%p, %s)", ue, sysname);
	return (udev_filter_add(&ue->filters, UDEV_FILTER_TYPE_SYSNAME, 0,
	     sysname, NULL));
}

LIBUDEV_EXPORT int
udev_enumerate_add_match_sysattr(struct udev_enumerate *ue,
    const char *sysattr, const char *value)
{

	TRC("(%p, %s, %s)", ue, sysattr, value);
	return (udev_filter_add(&ue->filters, UDEV_FILTER_TYPE_SYSATTR, 0,
	    sysattr, value));
}

LIBUDEV_EXPORT int
udev_enumerate_add_nomatch_sysattr(struct udev_enumerate *ue,
    const char *sysattr, const char *value)
{

	TRC("(%p, %s, %s)", ue, sysattr, value);
	return (udev_filter_add(&ue->filters, UDEV_FILTER_TYPE_SYSATTR, 1,
	    sysattr, value));
}


LIBUDEV_EXPORT int
udev_enumerate_add_match_property(struct udev_enumerate *ue,
    const char *property, const char *value)
{

	TRC("(%p, %s, %s)", ue, property, value);
	return (udev_filter_add(&ue->filters, UDEV_FILTER_TYPE_PROPERTY, 0,
	    property, value));
}

LIBUDEV_EXPORT int
udev_enumerate_add_match_tag(struct udev_enumerate *ue, const char *tag)
{

	TRC("(%p, %s)", ue, tag);
	return (udev_filter_add(&ue->filters, UDEV_FILTER_TYPE_TAG, 0, tag,
	    NULL));
}

LIBUDEV_EXPORT int
udev_enumerate_add_match_parent(struct udev_enumerate *ue,
    struct udev_device *parent)
{
	TRC("(%p, %p)", ue, parent);
	UNIMPL();
	return (0);
}
LIBUDEV_EXPORT int
udev_enumerate_add_match_is_initialized(struct udev_enumerate *ue)
{

	TRC("(%p)", ue);
	UNIMPL();
	return (0);
}

static int
enumerate_cb(const char *path, mode_t type, void *arg)
{
	struct udev_enumerate *ue = arg;
	const char *syspath;

	if (S_ISLNK(type) || S_ISCHR(type)) {
		syspath = get_syspath_by_devpath(path);
		if (udev_filter_match(ue->udev, &ue->filters, syspath) &&
		    udev_list_insert(&ue->dev_list, syspath, NULL) == -1)
			return (-1);
	}
	return (0);
}

LIBUDEV_EXPORT int
udev_enumerate_scan_devices(struct udev_enumerate *ue)
{
	struct scan_ctx ctx;
	char path[DEV_PATH_MAX] = DEV_PATH_ROOT "/";
	int ret;

	TRC("(%p)", ue);

	udev_list_free(&ue->dev_list);
	ctx = (struct scan_ctx) {
		.recursive = true,
		.cb = enumerate_cb,
		.args = ue,
	};

	ret = scandir_recursive(path, sizeof(path), &ctx);
#ifdef HAVE_DEVINFO_H
	if (ret == 0)
		ret = scandev_recursive(&ctx);
#endif
	if (ret == -1)
		udev_list_free(&ue->dev_list);
	return ret;
}

LIBUDEV_EXPORT int
udev_enumerate_scan_subsystems(struct udev_enumerate *ue)
{

	TRC("(%p)", ue);
	UNIMPL();
	return (0);
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_enumerate_get_list_entry(struct udev_enumerate *ue)
{

	TRC("(%p)", ue);
	return (udev_list_entry_get_first(&ue->dev_list));
}

LIBUDEV_EXPORT struct udev *
udev_enumerate_get_udev(struct udev_enumerate *ue)
{

	TRC("(%p)", ue);
	return (ue->udev);
}

LIBUDEV_EXPORT int
udev_enumerate_add_syspath(struct udev_enumerate *ue, const char *syspath)
{

	TRC("(%p, %s)", ue, syspath);
	return (udev_list_insert(&ue->dev_list, syspath, NULL));
}
