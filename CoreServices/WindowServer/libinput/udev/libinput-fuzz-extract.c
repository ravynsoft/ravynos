/*
 * Copyright © 2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <libudev.h>
#include <linux/input.h>
#include <libevdev/libevdev.h>

#include "util-prop-parsers.h"
#include "util-macros.h"

/**
 * For a non-zero fuzz on the x/y axes, print that fuzz as property and
 * reset the kernel's fuzz to 0.
 * https://bugs.freedesktop.org/show_bug.cgi?id=105202
 */
static void
handle_absfuzz(struct udev_device *device)
{
	const char *devnode;
	struct libevdev *evdev = NULL;
	int fd = -1;
	int rc;
	unsigned int axes[] = {ABS_X,
			       ABS_Y,
			       ABS_MT_POSITION_X,
			       ABS_MT_POSITION_Y};

	devnode = udev_device_get_devnode(device);
	if (!devnode)
		goto out;

	fd = open(devnode, O_RDONLY);
	if (fd < 0)
		goto out;

	rc = libevdev_new_from_fd(fd, &evdev);
	if (rc != 0)
		goto out;

	if (!libevdev_has_event_type(evdev, EV_ABS))
		goto out;

	ARRAY_FOR_EACH(axes, code) {
		int fuzz;

		fuzz = libevdev_get_abs_fuzz(evdev, *code);
		if (fuzz)
			printf("LIBINPUT_FUZZ_%02x=%d\n", *code, fuzz);
	}

out:
	close(fd);
	libevdev_free(evdev);
}

/**
 * Where a device has EVDEV_ABS_... set with a fuzz, that fuzz hasn't been
 * applied to the kernel yet. So we need to extract it ourselves **and**
 * update the property so the kernel won't actually set it later.
 */
static void
handle_evdev_abs(struct udev_device *device)
{
	unsigned int axes[] = {ABS_X,
			       ABS_Y,
			       ABS_MT_POSITION_X,
			       ABS_MT_POSITION_Y};

	ARRAY_FOR_EACH(axes, code) {
		const char *prop;
		char name[64];
		uint32_t mask;
		struct input_absinfo abs;

		snprintf(name, sizeof(name), "EVDEV_ABS_%02X", *code);
		prop = udev_device_get_property_value(device, name);
		if (!prop)
			continue;

		mask = parse_evdev_abs_prop(prop, &abs);
		if (mask & ABS_MASK_FUZZ)
			printf("LIBINPUT_FUZZ_%02x=%d\n", *code, abs.fuzz);
	}
}

int main(int argc, char **argv)
{
	int rc = 1;
	struct udev *udev = NULL;
	struct udev_device *device = NULL;
	const char *syspath;

	if (argc != 2)
		return 1;

	syspath = argv[1];

	udev = udev_new();
	if (!udev)
		goto out;

	device = udev_device_new_from_syspath(udev, syspath);
	if (!device)
		goto out;

	handle_absfuzz(device);
	handle_evdev_abs(device);

	rc = 0;

out:
	if (device)
		udev_device_unref(device);
	if (udev)
		udev_unref(udev);

	return rc;
}
