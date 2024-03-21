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
#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libudev.h"
#include "udev.h"
#include "udev-device.h"
#include "udev-filter.h"
#include "udev-list.h"
#include "udev-utils.h"
#include "utils.h"

struct udev_device {
	int refcount;
	struct {
		unsigned int action : 2;
		unsigned int is_parent : 1;
	} flags;
	struct udev_list prop_list;
	struct udev_list sysattr_list;
	struct udev_list tag_list;
	struct udev_list devlink_list;
	struct udev *udev;
	struct udev_device *parent;
	char syspath[];
};

LIBUDEV_EXPORT struct udev_device *
udev_device_new_from_syspath(struct udev *udev, const char *syspath)
{

	TRC("(%s)", syspath);
	return (udev_device_new_common(udev, syspath, UD_ACTION_NONE));
}

LIBUDEV_EXPORT struct udev_device *
udev_device_new_from_devnum(struct udev *udev, char type, dev_t devnum)
{
	const char *syspath;
	struct udev_device *device;

	syspath = get_syspath_by_devnum(devnum);
	TRC("(%d) -> %s", (int)devnum, syspath != NULL ? syspath : "not found");
	if (syspath == NULL)
		return (NULL);

	device = udev_device_new_common(udev, syspath, UD_ACTION_NONE);
	free((void *)syspath);

	return (device);
}

LIBUDEV_EXPORT struct udev_device *
udev_device_new_from_subsystem_sysname(struct udev *udev,
   const char *subsystem, const char *sysname)
{

	TRC("(%s, %s)", subsystem, sysname);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT struct udev_device *
udev_device_new_from_device_id(struct udev *udev, const char *id)
{
	TRC("(%s)", id);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT struct udev_device *
udev_device_new_from_environment(struct udev *udev)
{
	TRC("(%p)", ud);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT char const *
udev_device_get_devnode(struct udev_device *ud)
{
	const char *devpath;

	TRC("(%p) %s", ud, ud->syspath);
	devpath = get_devpath_by_syspath(ud->syspath);
	return (devpath);
}

LIBUDEV_EXPORT char const *
udev_device_get_devpath(struct udev_device *ud)
{
	TRC("(%p)", ud);
	UNIMPL();
	return (NULL);
}

struct udev_list *
udev_device_get_properties_list(struct udev_device *ud)
{

	return (&ud->prop_list);
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_device_get_properties_list_entry(struct udev_device *ud)
{

	TRC("(%p(%s))", ud, ud->syspath);
	return (udev_list_entry_get_first(udev_device_get_properties_list(ud)));
}

struct udev_list *
udev_device_get_sysattr_list(struct udev_device *ud)
{

	return (&ud->sysattr_list);
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_device_get_sysattr_list_entry(struct udev_device *ud)
{


	TRC("(%p(%s))", ud, ud->syspath);
	return (udev_list_entry_get_first(udev_device_get_sysattr_list(ud)));
}

struct udev_list *
udev_device_get_tags_list(struct udev_device *ud)
{

	return (&ud->tag_list);
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_device_get_tags_list_entry(struct udev_device *ud)
{

	TRC("(%p(%s))", ud, ud->syspath);
	return (udev_list_entry_get_first(udev_device_get_tags_list(ud)));
}

LIBUDEV_EXPORT int
udev_device_has_tag(struct udev_device *ud, const char *tag)
{
	struct udev_list_entry *ule;

	TRC("(%p, %s)", ud, tag);
	ule = udev_list_entry_get_first(udev_device_get_tags_list(ud));
	return (udev_list_entry_get_by_name(ule, tag) != NULL);
}

struct udev_list *
udev_device_get_devlinks_list(struct udev_device *ud)
{

	return (&ud->devlink_list);
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_device_get_devlinks_list_entry(struct udev_device *ud)
{

	TRC("(%p(%s))", ud, ud->syspath);
	return (udev_list_entry_get_first(udev_device_get_devlinks_list(ud)));
}

LIBUDEV_EXPORT char const *
udev_device_get_property_value(struct udev_device *ud, char const *property)
{
	char const *key, *value;
	struct udev_list_entry *entry;

	udev_list_entry_foreach(entry, udev_list_entry_get_first(&ud->prop_list)) {
		key = _udev_list_entry_get_name(entry);
		if (!key)
			continue;
		if (strcmp(key, property) == 0) {
			value = _udev_list_entry_get_value(entry);
			TRC("(%p(%s), %s) %s", ud, ud->syspath, property, value);
			return (value);
		}
	}
	TRC("(%p(%s), %s) NULL", ud, ud->syspath, property);
	return (NULL);
}

LIBUDEV_EXPORT char const *
udev_device_get_sysattr_value(struct udev_device *ud, const char *sysattr)
{
	char const *key, *value;
	struct udev_list_entry *entry;

	udev_list_entry_foreach(entry, udev_list_entry_get_first(&ud->sysattr_list)) {
		key = _udev_list_entry_get_name(entry);
		if (!key)
			continue;
		if (strcmp(key, sysattr) == 0) {
			value = _udev_list_entry_get_value(entry);
			TRC("(%p(%s), %s) %s", ud, ud->syspath, sysattr, value);
			return (value);
		}
	}
	TRC("(%p(%s), %s) NULL", ud, ud->syspath, sysattr);
	return (NULL);
}

LIBUDEV_EXPORT int
udev_device_set_sysattr_value(struct udev_device *ud, const char *sysattr, const char *value)
{
	struct udev_list_entry *entry;

	udev_list_entry_foreach(entry, udev_list_entry_get_first(&ud->sysattr_list)) {
		char const *key;

		key = _udev_list_entry_get_name(entry);
		if (key && strcmp(key, sysattr) == 0)
			return -1;
	}

	return udev_list_insert(&ud->sysattr_list, sysattr, value);
}

LIBUDEV_EXPORT struct udev *
udev_device_get_udev(struct udev_device *ud)
{

	TRC("(%p(%s))", ud, ud->syspath);
	return (ud->udev);
}

struct udev_device *
udev_device_new_common(struct udev *udev, const char *syspath, int action)
{
	struct udev_device *ud;

	ud = calloc
	    (1, offsetof(struct udev_device, syspath) + strlen(syspath) + 1);
	if (ud == NULL)
		return (NULL);

	_udev_ref(udev);
	ud->udev = udev;
	ud->flags.action = action;
	ud->parent = NULL;
	ud->refcount = 1;
	strcpy(ud->syspath, syspath);
	udev_list_init(&ud->prop_list);
	udev_list_init(&ud->sysattr_list);
	udev_list_init(&ud->tag_list);
	udev_list_init(&ud->devlink_list);
	if (action != UD_ACTION_REMOVE)
		invoke_create_handler(ud);

	return (ud);
}

LIBUDEV_EXPORT const char *
udev_device_get_syspath(struct udev_device *ud)
{

	TRC("(%p) %s", ud, ud->syspath);
	return (ud->syspath);
}

LIBUDEV_EXPORT const char *
udev_device_get_sysname(struct udev_device *ud)
{
	const char *sysname;

	sysname = get_sysname_by_syspath(ud->syspath);
	TRC("(%p(%s)) %s", ud, ud->syspath, sysname);
	return (sysname);
}

LIBUDEV_EXPORT const char *
udev_device_get_subsystem(struct udev_device *ud)
{
	const char *subsystem;

	subsystem = get_subsystem_by_syspath(udev_device_get_syspath(ud));
	TRC("(%p(%s)) %s", ud, ud->syspath, subsystem);
	return (subsystem);
}

LIBUDEV_EXPORT struct udev_device *
udev_device_ref(struct udev_device *ud)
{
	TRC("(%p/%s) %d", ud, ud->syspath, ud->refcount);

	if (!ud->flags.is_parent)
		++ud->refcount;
	return (ud);
}

static void
udev_device_free(struct udev_device *ud)
{

	udev_list_free(&ud->prop_list);
	udev_list_free(&ud->sysattr_list);
	udev_list_free(&ud->tag_list);
	udev_list_free(&ud->devlink_list);
	if (ud->parent != NULL)
		udev_device_free(ud->parent);
	_udev_unref(ud->udev);
	free(ud);
}

LIBUDEV_EXPORT void
udev_device_unref(struct udev_device *ud)
{

	TRC("(%p/%s) %d", ud, ud->syspath, ud->refcount);
	if (ud->flags.is_parent)
		return;
	if (--ud->refcount == 0)
		udev_device_free(ud);
}

LIBUDEV_EXPORT struct udev_device *
udev_device_get_parent(struct udev_device *ud)
{

	TRC("(%p/%s) %p", ud, ud->syspath, ud->parent);
	return (ud->parent);
}

LIBUDEV_EXPORT struct udev_device *
udev_device_get_parent_with_subsystem_devtype(struct udev_device *ud,
    const char *subsystem, const char *devtype)
{

	TRC("(%p/%s, %s, %s)", ud, ud->syspath, subsystem, devtype);
	UNIMPL();
	return (ud->parent);
}

void
udev_device_set_parent(struct udev_device *ud, struct udev_device *parent)
{

	parent->flags.is_parent = 1;
	ud->parent = parent;
}

LIBUDEV_EXPORT int
udev_device_get_is_initialized(struct udev_device *ud)
{

	TRC("(%p/%s)", ud, ud->syspath);
	return (1);
}

LIBUDEV_EXPORT const char *
udev_device_get_action(struct udev_device *ud)
{
	const char *action;

	switch(ud->flags.action) {
	case UD_ACTION_NONE:
		action = "none";
		break;
	case UD_ACTION_ADD:
		action = "add";
		break;
	case UD_ACTION_REMOVE:
		action = "remove";
		break;
	case UD_ACTION_HOTPLUG:
		action = "change";
		break;
	default:
		action = "unknown";
	}
	TRC("(%p/%s) %s", ud, ud->syspath, action);
	return (action);
}

LIBUDEV_EXPORT dev_t
udev_device_get_devnum(struct udev_device *ud)
{
	const char *devpath;
	struct stat st;

	TRC("(%p) %s", ud, ud->syspath);
	devpath = get_devpath_by_syspath(ud->syspath);
	if (devpath == NULL ||
	    stat(devpath, &st) < 0 ||
	    !S_ISCHR(st.st_mode))
		return (makedev(0, 0));

	return (st.ST_RDEV);
}

LIBUDEV_EXPORT const char *
udev_device_get_devtype(struct udev_device *ud)
{

	TRC("(%p) %s", ud, ud->syspath);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT const char *
udev_device_get_driver(struct udev_device *ud)
{

	TRC("(%p) %s", ud, ud->syspath);
	UNIMPL();
	return (NULL);
}

LIBUDEV_EXPORT const char *
udev_device_get_sysnum(struct udev_device *ud)
{

	TRC("(%p) %s", ud, ud->syspath);
	return (ud->syspath + syspathlen_wo_units(ud->syspath));
}

LIBUDEV_EXPORT unsigned long long int
udev_device_get_seqnum(struct udev_device *ud)
{

	TRC("(%p) %s", ud, ud->syspath);
	UNIMPL();
	return (0);
}

LIBUDEV_EXPORT unsigned long long int
udev_device_get_usec_since_initialized(struct udev_device *ud)
{

	TRC("(%p) %s", ud, ud->syspath);
	UNIMPL();
	return (0);
}
