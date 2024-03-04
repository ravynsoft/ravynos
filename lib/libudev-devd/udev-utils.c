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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYSCTLBYNAME
#include <sys/sysctl.h>
#endif

#include <assert.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_DEV_HID_HIDRAW_H
#include <dev/hid/hidraw.h>
#endif
#ifdef HAVE_LINUX_INPUT_H
#include <linux/input.h>
#endif

#include "libudev.h"
#include "udev-device.h"
#include "udev-list.h"
#include "udev-utils.h"
#include "utils.h"
#ifdef ENABLE_GPL
#include "utils-gpl.h"
#endif

#ifdef HAVE_LINUX_INPUT_H
#ifndef	BTN_DPAD_UP
#define	BTN_DPAD_UP	0x220
#endif
#ifndef	BTN_DPAD_RIGHT
#define	BTN_DPAD_RIGHT	0x223
#endif
#ifndef	BTN_SOUTH
#define	BTN_SOUTH	0x130
#endif
#else	/* !HAVE_LINUX_INPUT_H */
#define	BUS_PCI		0x01
#define	BUS_USB		0x03
#define	BUS_VIRTUAL	0x06
#define	BUS_ISA		0x10
#define	BUS_I8042	0x11
#endif	/* !HAVE_LINUX_INPUT_H */

#define	PS2_KEYBOARD_VENDOR		0x001
#define	PS2_KEYBOARD_PRODUCT		0x001
#define	PS2_MOUSE_VENDOR		0x002
#define	PS2_MOUSE_GENERIC_PRODUCT	0x001

typedef void (create_node_handler_t)(struct udev_device *udev_device);

#if defined(HAVE_LINUX_INPUT_H) || defined(HAVE_DEV_HID_HIDRAW_H)
static const char *virtual_sysname = "uinput";
#endif

#ifdef HAVE_LINUX_INPUT_H
static create_node_handler_t	create_evdev_handler;
#endif
static create_node_handler_t	create_keyboard_handler;
static create_node_handler_t	create_mouse_handler;
static create_node_handler_t	create_joystick_handler;
static create_node_handler_t	create_touchpad_handler;
static create_node_handler_t	create_touchscreen_handler;
static create_node_handler_t	create_sysmouse_handler;
static create_node_handler_t	create_kbdmux_handler;
static create_node_handler_t	create_drm_handler;
#ifdef HAVE_DEV_HID_HIDRAW_H
static create_node_handler_t	create_hidraw_handler;
#endif

struct devnum_scan_args {
	dev_t	devnum;
	char *	pattern;
	char *	path;
	size_t	len;
};

struct subsystem_config {
	char *subsystem;
	char *syspath;
	char *symlink; /* If syspath is symlink, path it refers to */
	int flags; /* See SCFLAG_* below. */
	create_node_handler_t *create_handler;
};

enum {
	IT_NONE,
	IT_KEYBOARD,
	IT_MOUSE,
	IT_TOUCHPAD,
	IT_TOUCHSCREEN,
	IT_JOYSTICK,
	IT_TABLET,
	IT_ACCELEROMETER,
	IT_SWITCH,
};

/* Flag which in indicates a device should be skipped because it's
 * already exposed through EVDEV when it's enabled. */
#define	SCFLAG_SKIP_IF_EVDEV	0x01

static const struct subsystem_config subsystems[] = {
	{
#ifdef HAVE_LINUX_INPUT_H
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/input/event[0-9]*",
		.create_handler = create_evdev_handler
	}, {
#endif
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/ukbd[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_keyboard_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/atkbd[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_keyboard_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/kbdmux[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_kbdmux_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/ums[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_mouse_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/psm[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_mouse_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/joy[0-9]*",
		.create_handler = create_joystick_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/atp[0-9]*",
		.create_handler = create_touchpad_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/wsp[0-9]*",
		.create_handler = create_touchpad_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/uep[0-9]*",
		.create_handler = create_touchscreen_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/sysmouse",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_sysmouse_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/vboxguest",
		.create_handler = create_mouse_handler,
	}, {
		.subsystem = "drm",
		.syspath = DEV_PATH_ROOT "/dri/card[0-9]*",
		.symlink = DEV_PATH_ROOT "/drm/[0-9]*",
		.create_handler = create_drm_handler,
	},
#ifdef HAVE_DEV_HID_HIDRAW_H
	{
		.subsystem = "hidraw",
		.syspath = DEV_PATH_ROOT "/hidraw[0-9]*",
		.create_handler = create_hidraw_handler,
	},
#endif
};

static const struct subsystem_config *
get_subsystem_config_by_syspath(const char *path)
{
	size_t i;

	for (i = 0; i < nitems(subsystems); i++)
		if (fnmatch(subsystems[i].syspath, path, 0) == 0)
			return (&subsystems[i]);

	return (NULL);
}

static bool
kernel_has_evdev_enabled()
{
	static int enabled = -1;
#ifdef HAVE_SYSCTLBYNAME
	size_t len;
#endif

	if (enabled != -1)
		return (enabled);

#ifdef HAVE_SYSCTLBYNAME
	if (sysctlbyname("kern.features.evdev_support", &enabled, &len, NULL, 0) < 0)
		return (0);
#else
	enabled = 1;
#endif

	TRC("() EVDEV enabled: %s", enabled ? "true" : "false");
	return (enabled);
}

const char *
get_subsystem_by_syspath(const char *syspath)
{
	const struct subsystem_config *sc;

	sc = get_subsystem_config_by_syspath(syspath);
	if (sc == NULL)
		return (UNKNOWN_SUBSYSTEM);
	if (sc->flags & SCFLAG_SKIP_IF_EVDEV && kernel_has_evdev_enabled()) {
		TRC("(%s) EVDEV enabled -> skipping device", syspath);
		return (UNKNOWN_SUBSYSTEM);
	}

	return (sc->subsystem);
}

const char *
get_sysname_by_syspath(const char *syspath)
{

	return (strbase(syspath));
}

const char *
get_devpath_by_syspath(const char *syspath)
{

	return (syspath);
}

const char *
get_syspath_by_devpath(const char *devpath)
{

	return (devpath);
}

static int
get_syspath_by_devnum_cb(const char *path, mode_t type, void *args)
{
	struct devnum_scan_args *sa = args;
	struct stat st;

	if (S_ISLNK(type) &&
	    fnmatch(sa->pattern, path, 0) == 0 &&
	    stat(path, &st) == 0 &&
	    st.ST_RDEV == sa->devnum) {
		strlcpy(sa->path, path, sa->len);
		return (-1);
	}
	return (0);
}

const char *
get_syspath_by_devnum(dev_t devnum)
{
	char devpath[DEV_PATH_MAX] = DEV_PATH_ROOT "/";
	char linkdir[DEV_PATH_MAX];
	struct stat st;
	struct scan_ctx ctx;
	struct devnum_scan_args args;
	const char *linkbase;
	size_t dev_len, linkdir_len, i;

	dev_len = strlen(devpath);
	devname_r(devnum, S_IFCHR, devpath + dev_len, sizeof(devpath) - dev_len);
	/* Recheck path as devname_r returns zero-terminated garbage on error */
	if (stat(devpath, &st) != 0 || st.ST_RDEV != devnum) {
		TRC("(%d) -> failed", (int)devnum);
		return NULL;
	}
	TRC("(%d) -> %s", (int)devnum, devpath);

	/* Resolve symlink in reverse direction if necessary */
	for (i = 0; i < nitems(subsystems); i++) {
		if (subsystems[i].symlink != NULL &&
		    fnmatch(subsystems[i].symlink, devpath, 0) == 0) {
			linkbase = strbase(subsystems[i].syspath);
			assert(linkbase != NULL);
			linkdir_len = linkbase - subsystems[i].syspath;
			if (linkdir_len >= sizeof(linkdir))
				linkdir_len = sizeof(linkdir);
			strlcpy(linkdir, subsystems[i].syspath, linkdir_len + 1);
			args = (struct devnum_scan_args) {
				.devnum = devnum,
				.pattern = subsystems[i].syspath,
				.path = devpath,
				.len = sizeof(devpath),
			};
			ctx = (struct scan_ctx) {
				.recursive = false,
				.cb = get_syspath_by_devnum_cb,
				.args = &args,
			};
			if (scandir_recursive(linkdir, sizeof(linkdir), &ctx) == -1)
				break;
		}
	}

	return (strdup(devpath));
}

void
invoke_create_handler(struct udev_device *ud)
{
	const char *path;
	const struct subsystem_config *sc;

	path = udev_device_get_syspath(ud);
	sc = get_subsystem_config_by_syspath(path);
	if (sc == NULL || sc->create_handler == NULL)
		return;
	if (sc->flags & SCFLAG_SKIP_IF_EVDEV && kernel_has_evdev_enabled()) {
		TRC("(%p) EVDEV enabled -> skipping device", ud);
		return;
	}

	sc->create_handler(ud);
}

static int
set_input_device_type(struct udev_device *ud, int input_type)
{
	struct udev_list *ul;

	ul = udev_device_get_properties_list(ud);
	if (udev_list_insert(ul, "ID_INPUT", "1") < 0)
		return (-1);
	switch (input_type) {
	case IT_KEYBOARD:
		udev_list_insert(ul, "ID_INPUT_KEY", "1");
		udev_list_insert(ul, "ID_INPUT_KEYBOARD", "1");
		break;
	case IT_MOUSE:
		udev_list_insert(ul, "ID_INPUT_MOUSE", "1");
		break;
	case IT_TOUCHPAD:
		udev_list_insert(ul, "ID_INPUT_MOUSE", "1");
		udev_list_insert(ul, "ID_INPUT_TOUCHPAD", "1");
		break;
	case IT_TOUCHSCREEN:
		udev_list_insert(ul, "ID_INPUT_TOUCHSCREEN", "1");
		break;
	case IT_JOYSTICK:
		udev_list_insert(ul, "ID_INPUT_JOYSTICK", "1");
		break;
	case IT_TABLET:
		udev_list_insert(ul, "ID_INPUT_TABLET", "1");
		break;
	case IT_ACCELEROMETER:
		udev_list_insert(ul, "ID_INPUT_ACCELEROMETER", "1");
		break;
	case IT_SWITCH:
		udev_list_insert(ul, "ID_INPUT_SWITCH", "1");
		break;
	}
	return (0);
}

static struct udev_device *
create_xorg_parent(struct udev_device *ud, const char* sysname,
    const char *name, const char *product, const char *pnp_id)
{
	struct udev_device *parent;
	struct udev *udev;
	struct udev_list *props, *sysattrs;

	/* xorg-server gets device name and vendor string from parent device */
	udev = udev_device_get_udev(ud);
	parent = udev_device_new_common(udev, sysname, UD_ACTION_NONE);
	if (parent == NULL)
		return NULL;

	props = udev_device_get_properties_list(parent);
	sysattrs = udev_device_get_sysattr_list(parent);
	udev_list_insert(props, "NAME", name);
	udev_list_insert(sysattrs, "name", name);
	if (product != NULL)
		udev_list_insert(props, "PRODUCT", product);
	if (pnp_id != NULL)
		udev_list_insert(sysattrs, "id", product);

	return (parent);
}

#ifdef HAVE_LINUX_INPUT_H

#define	LONG_BITS	(sizeof(long) * 8)
#define	NLONGS(x)	(((x) + LONG_BITS - 1) / LONG_BITS)

static inline bool
bit_is_set(const unsigned long *array, int bit)
{
	return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}

static inline bool
bit_find(const unsigned long *array, int start, int stop)
{
	int i;

	for (i = start; i < stop; i++)
		if (bit_is_set(array, i))
			return true;

	return false;
}

static void
create_evdev_handler(struct udev_device *ud)
{
	struct udev_device *parent;
	const char *sysname;
	char name[80], product[80], phys[80];
	int fd = -1, input_type = IT_NONE;
	bool opened = false;
	bool has_keys, has_buttons, has_lmr, has_dpad, has_joy_axes;
	bool has_rel_axes, has_abs_axes, has_mt, has_switches;
	unsigned long key_bits[NLONGS(KEY_CNT)];
	unsigned long rel_bits[NLONGS(REL_CNT)];
	unsigned long abs_bits[NLONGS(ABS_CNT)];
	unsigned long sw_bits[NLONGS(SW_CNT)];
	unsigned long prp_bits[NLONGS(INPUT_PROP_CNT)];
	struct input_id id;
#ifdef HAVE_SYSCTLBYNAME
	const char *unit;
	char mib[32];
	size_t len;

	sysname = udev_device_get_sysname(ud);
	len = syspathlen_wo_units(sysname);
	unit = sysname + len;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.name", unit);
	len = sizeof(name);
	if (sysctlbyname(mib, name, &len, NULL, 0) < 0)
		goto use_ioctl;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.phys", unit);
	len = sizeof(phys);
	if (sysctlbyname(mib, phys, &len, NULL, 0) < 0)
		goto use_ioctl;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.id", unit);
	len = sizeof(id);
	if (sysctlbyname(mib, &id, &len, NULL, 0) < 0)
		goto use_ioctl;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.key_bits", unit);
	len = sizeof(key_bits);
	if (sysctlbyname(mib, key_bits, &len, NULL, 0) < 0)
		goto use_ioctl;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.rel_bits", unit);
	len = sizeof(rel_bits);
	if (sysctlbyname(mib, rel_bits, &len, NULL, 0) < 0)
		goto use_ioctl;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.abs_bits", unit);
	len = sizeof(abs_bits);
	if (sysctlbyname(mib, abs_bits, &len, NULL, 0) < 0)
		goto use_ioctl;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.sw_bits", unit);
	len = sizeof(sw_bits);
	if (sysctlbyname(mib, sw_bits, &len, NULL, 0) < 0)
		goto use_ioctl;

	snprintf(mib, sizeof(mib), "kern.evdev.input.%s.props", unit);
	len = sizeof(prp_bits);
	if (sysctlbyname(mib, prp_bits, &len, NULL, 0) < 0)
		goto use_ioctl;

	goto found_values;

use_ioctl:
	ERR("sysctl not found, opening device and using ioctl");
#endif

	fd = open(udev_device_get_devnode(ud), O_RDONLY | O_CLOEXEC);
	if (fd == -1) {
		fd = path_to_fd(udev_device_get_devnode(ud));
	} else {
		opened = true;
	}
	if (fd == -1)
		return;

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0 ||
	    (ioctl(fd, EVIOCGPHYS(sizeof(phys)), phys) < 0 && errno != ENOENT) ||
	    ioctl(fd, EVIOCGID, &id) < 0 ||
	    ioctl(fd, EVIOCGBIT(EV_REL, sizeof(rel_bits)), rel_bits) < 0 ||
	    ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) < 0 ||
	    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0 ||
	    ioctl(fd, EVIOCGBIT(EV_SW, sizeof(sw_bits)), sw_bits) < 0 ||
	    ioctl(fd, EVIOCGPROP(sizeof(prp_bits)), prp_bits) < 0) {
		ERR("could not query evdev");
		goto bail_out;
	}

#ifdef HAVE_SYSCTLBYNAME
found_values:
#endif
	/* Derived from EvdevProbe() function of xf86-input-evdev driver */
	has_keys = bit_find(key_bits, 0, BTN_MISC);
	has_buttons = bit_find(key_bits, BTN_MISC, BTN_JOYSTICK);
	has_lmr = bit_find(key_bits, BTN_LEFT, BTN_MIDDLE + 1);
	has_dpad = bit_find(key_bits, BTN_DPAD_UP, BTN_DPAD_RIGHT + 1);
	has_joy_axes = bit_find(abs_bits, ABS_RX, ABS_HAT3Y + 1);
	has_rel_axes = bit_find(rel_bits, 0, REL_CNT);
	has_abs_axes = bit_find(abs_bits, 0, ABS_CNT);
	has_switches = bit_find(sw_bits, 0, SW_CNT);
	has_mt = bit_find(abs_bits, ABS_MT_SLOT, ABS_CNT);

	if (has_abs_axes) {
		if (has_mt && !has_buttons) {
			if (bit_is_set(key_bits, BTN_JOYSTICK)) {
				input_type = IT_JOYSTICK;
				goto detected;
			} else {
				has_buttons = true;
			}
		}

		if (bit_is_set(abs_bits, ABS_X) &&
		    bit_is_set(abs_bits, ABS_Y)) {
			if (bit_is_set(key_bits, BTN_TOOL_PEN) ||
			    bit_is_set(key_bits, BTN_STYLUS) ||
			    bit_is_set(key_bits, BTN_STYLUS2)) {
				input_type = IT_TABLET;
				goto detected;
			} else if (has_joy_axes ||
				   bit_is_set(key_bits, BTN_JOYSTICK)) {
			        /* Device is a joystick */
				input_type = IT_JOYSTICK;
				goto detected;
			} else if (bit_is_set(key_bits, BTN_SOUTH) ||
			           has_dpad ||
			           bit_is_set(abs_bits, ABS_HAT0X) ||
			           bit_is_set(abs_bits, ABS_HAT0Y) ||
			           bit_is_set(key_bits, BTN_THUMBL) ||
			           bit_is_set(key_bits, BTN_THUMBR)) {
			        /* Device is a gamepad */
				input_type = IT_JOYSTICK;
				goto detected;
			} else if (bit_is_set(abs_bits, ABS_PRESSURE) ||
			           bit_is_set(key_bits, BTN_TOUCH)) {
				if (has_lmr ||
				    bit_is_set(key_bits, BTN_TOOL_FINGER)) {
					input_type = IT_TOUCHPAD;
				} else {
					input_type = IT_TOUCHSCREEN;
				}
				goto detected;
			} else if (!(bit_is_set(rel_bits, REL_X) &&
			             bit_is_set(rel_bits, REL_Y)) &&
			             has_lmr) {
				input_type = IT_MOUSE;
				goto detected;
			}
		}
	}

	if (bit_is_set(prp_bits, INPUT_PROP_ACCELEROMETER))
		input_type = IT_ACCELEROMETER;
	else if (has_keys)
		input_type = IT_KEYBOARD;
	else if (bit_is_set(prp_bits, INPUT_PROP_POINTER) || has_rel_axes || has_abs_axes || has_buttons)
		input_type = IT_MOUSE;
	else if (has_switches)
		input_type = IT_SWITCH;

	if (input_type == IT_NONE)
		goto bail_out;

detected:
	set_input_device_type(ud, input_type);

	sysname = phys[0] == 0 ? virtual_sysname : phys;

	*(strchrnul(name, ',')) = '\0';	/* strip name */

	snprintf(product, sizeof(product), "%x/%x/%x/%x",
	    id.bustype, id.vendor, id.product, id.version);

	parent = create_xorg_parent(ud, sysname, name, product, NULL);
	if (parent != NULL)
		udev_device_set_parent(ud, parent);

bail_out:
	if (opened)
		close(fd);
}
#endif

size_t
syspathlen_wo_units(const char *path) {
	size_t len;

	len = strlen(path);
	while (len > 0) {
		if (path[len-1] < '0' || path[len-1] > '9')
			break;
		--len;
	}
	return len;
}

static void
set_parent(struct udev_device *ud)
{
	struct udev_device *parent;
	const char *sysname;
	char product[80], name[80];
	char *pnp_id = NULL;
	size_t len;
	uint32_t bus = BUS_VIRTUAL, prod = 0, vendor = 0;
#ifdef HAVE_SYSCTLBYNAME
	char devname[DEV_PATH_MAX], mib[32], pnpinfo[1024], parentname[80];
	const char *unit, *vendorstr, *prodstr, *devicestr;
	size_t vendorlen, prodlen, devicelen, pnplen;
#endif

	sysname = udev_device_get_sysname(ud);
	len = syspathlen_wo_units(sysname);
	/* Check if device unit number found */
	if (strlen(sysname) == len)
		return;

#ifdef HAVE_SYSCTLBYNAME
	snprintf(devname, len + 1, "%s", sysname);
	unit = sysname + len;

	snprintf(mib, sizeof(mib), "dev.%.17s.%.3s.%%desc", devname, unit);
	len = sizeof(name);
	if (sysctlbyname(mib, name, &len, NULL, 0) < 0)
		return;
	*(strchrnul(name, ',')) = '\0';	/* strip name */

	snprintf(mib, sizeof(mib), "dev.%.14s.%.3s.%%pnpinfo", devname, unit);
	len = sizeof(pnpinfo);
	if (sysctlbyname(mib, pnpinfo, &len, NULL, 0) < 0)
		return;

	snprintf(mib, sizeof(mib), "dev.%.15s.%.3s.%%parent", devname, unit);
	len = sizeof(parentname);
	if (sysctlbyname(mib, parentname, &len, NULL, 0) < 0)
		return;

	vendorstr = get_kern_prop_value(pnpinfo, "vendor", &vendorlen);
	prodstr = get_kern_prop_value(pnpinfo, "product", &prodlen);
	devicestr = get_kern_prop_value(pnpinfo, "device", &devicelen);
	pnp_id = get_kern_prop_value(pnpinfo, "_HID", &pnplen);
	if (pnp_id != NULL && pnplen == 4 && strncmp(pnp_id, "none", 4) == 0)
		pnp_id = NULL;
	if (pnp_id != NULL)
		pnp_id[pnplen] = '\0';
	if (prodstr != NULL && vendorstr != NULL) {
		/* XXX: should parent be compared to uhub* to detect usb? */
		vendor = strtol(vendorstr, NULL, 0);
		prod = strtol(prodstr, NULL, 0);
		bus = BUS_USB;
	} else if (devicestr != NULL && vendorstr != NULL) {
		vendor = strtol(vendorstr, NULL, 0);
		prod = strtol(devicestr, NULL, 0);
		bus = BUS_PCI;
	} else if (strcmp(parentname, "atkbdc0") == 0) {
		if (strcmp(devname, "atkbd") == 0) {
			vendor = PS2_KEYBOARD_VENDOR;
			prod = PS2_KEYBOARD_PRODUCT;
		} else if (strcmp(devname, "psm") == 0) {
			vendor = PS2_MOUSE_VENDOR;
			prod = PS2_MOUSE_GENERIC_PRODUCT;
		} else {
			vendor = 0;
			prod = 0;
		}
		bus = BUS_I8042;
	}
#else
	strlcpy(name, sysname, sizeof(name));
#endif
	snprintf(product, sizeof(product), "%x/%x/%x/0", bus, vendor, prod);
	parent = create_xorg_parent(ud, sysname, name, product, pnp_id);
	if (parent != NULL)
		udev_device_set_parent(ud, parent);

	return;
}

static void
create_keyboard_handler(struct udev_device *ud)
{

	set_input_device_type(ud, IT_KEYBOARD);
	set_parent(ud);
}

static void
create_mouse_handler(struct udev_device *ud)
{

	set_input_device_type(ud, IT_MOUSE);
	set_parent(ud);
}

static void
create_kbdmux_handler(struct udev_device *ud)
{
	struct udev_device *parent;
	const char* sysname;

	set_input_device_type(ud, IT_KEYBOARD);
	sysname = udev_device_get_sysname(ud);
	parent = create_xorg_parent(ud, sysname,
	    "System keyboard multiplexor", "6/1/1/0", NULL);
	if (parent != NULL)
		udev_device_set_parent(ud, parent);
}

static void
create_sysmouse_handler(struct udev_device *ud)
{
	struct udev_device *parent;
	const char* sysname;

	set_input_device_type(ud, IT_MOUSE);
	sysname = udev_device_get_sysname(ud);
	parent = create_xorg_parent(ud, sysname,
	    "System mouse", "6/2/1/0", NULL);
	if (parent != NULL)
		udev_device_set_parent(ud, parent);
}

static void
create_joystick_handler(struct udev_device *ud)
{

	set_input_device_type(ud, IT_JOYSTICK);
	set_parent(ud);
}

static void
create_touchpad_handler(struct udev_device *ud)
{

	set_input_device_type(ud, IT_TOUCHPAD);
	set_parent(ud);
}

static void
create_touchscreen_handler(struct udev_device *ud)
{

	set_input_device_type(ud, IT_TOUCHSCREEN);
	set_parent(ud);
}

static void
create_drm_handler(struct udev_device *ud)
{
	const char *sysname, *devpath;
	struct udev_device *parent;
#ifdef HAVE_SYSCTLBYNAME
	char devbuf[PATH_MAX], buf[32], *devbufptr;
	size_t buflen = sizeof(devbuf);
#endif

	udev_list_insert(udev_device_get_properties_list(ud), "HOTPLUG", "1");
	devpath = udev_device_get_devnode(ud);
	if (devpath == NULL)
		return;

	sysname = udev_device_get_sysname(ud);
	parent = create_xorg_parent(ud, sysname, "drm parent", NULL, NULL);
	if (parent == NULL)
		return;

	udev_device_set_parent(ud, parent);

#ifdef HAVE_SYSCTLBYNAME
	realpath(devpath, devbuf);
	devbufptr = devbuf + 1;
	devbufptr = strchrnul(devbufptr, '/');
	while (*devbufptr != '\0') {
		*devbufptr = '.';
		devbufptr = strchrnul(devbufptr, '/');
	}
	snprintf(buf, sizeof(buf), "%.24s.PCI_ID", devbuf + 1);
	if (sysctlbyname(buf, devbuf, &buflen, NULL, 0) == 0){
		udev_list_insert(
		    udev_device_get_properties_list(parent), "PCI_ID", devbuf);}
#endif
}

#ifdef HAVE_DEV_HID_HIDRAW_H
static void
create_hidraw_handler(struct udev_device *ud)
{
	char name[80], phys[80], uniq[32];
	const char *sysname;
	char *uevent;
	struct hidraw_devinfo info;
	struct udev_device *parent;
	struct udev *udev;
	struct udev_list *sysattrs;
	int fd = -1;
	bool opened = false;

	fd = path_to_fd(udev_device_get_devnode(ud));
	if (fd == -1) {
		fd = open(udev_device_get_devnode(ud), O_RDONLY | O_CLOEXEC);
		opened = true;
	}
	if (fd == -1)
		return;

	if (ioctl(fd, HIDIOCGRAWNAME(sizeof(name)), name) < 0 ||
	    ioctl(fd, HIDIOCGRAWPHYS(sizeof(phys)), phys) < 0 ||
	    ioctl(fd, HIDIOCGRAWUNIQ(sizeof(uniq)), uniq) < 0 ||
	    ioctl(fd, HIDIOCGRAWINFO, &info) < 0) {
		ERR("could not query hidraw");
		goto bail_out;
	}

	sysname = phys[0] == 0 ? virtual_sysname : phys;
	udev = udev_device_get_udev(ud);
	parent = udev_device_new_common(udev, sysname, UD_ACTION_NONE);
	if (parent == NULL)
		goto bail_out;

	sysattrs = udev_device_get_sysattr_list(parent);
	asprintf(&uevent,
	    "HID_ID=%04X:%08X:%08X\nHID_NAME=%s\nHID_PHYS=%s\nHID_UNIQ=%s",
	    info.bustype, info.vendor, info.product, name, phys, uniq);
	udev_list_insert(sysattrs, "uevent", uevent);
	free(uevent);

bail_out:
	if (opened)
		close(fd);
}
#endif

LIBUDEV_EXPORT int
udev_util_encode_string(const char *str, char *str_enc, size_t len)
{
#ifdef ENABLE_GPL
	return (encode_devnode_name(str, str_enc, len));
#else
	return (strlcpy(str_enc, str, len) < len ? 0 : -EINVAL);
#endif
}
