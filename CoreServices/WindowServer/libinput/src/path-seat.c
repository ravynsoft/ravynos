/*
 * Copyright © 2013-2015 Red Hat, Inc.
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

#include <string.h>
#include <sys/stat.h>
#include <libudev.h>

#include "evdev.h"

struct path_input {
	struct libinput base;
	struct udev *udev;
	struct list path_list;
};

struct path_device {
	struct list link;
	struct udev_device *udev_device;
};

struct path_seat {
	struct libinput_seat base;
};

static const char default_seat[] = "seat0";
static const char default_seat_name[] = "default";

static void
path_disable_device(struct evdev_device *device)
{
	struct libinput_seat *seat = device->base.seat;
	struct evdev_device *dev;

	list_for_each_safe(dev,
			   &seat->devices_list, base.link) {
		if (dev != device)
			continue;

		evdev_device_remove(device);
		break;
	}
}

static void
path_input_disable(struct libinput *libinput)
{
	struct path_input *input = (struct path_input*)libinput;
	struct path_seat *seat;
	struct evdev_device *device;

	list_for_each_safe(seat, &input->base.seat_list, base.link) {
		libinput_seat_ref(&seat->base);
		list_for_each_safe(device,
				   &seat->base.devices_list, base.link)
			path_disable_device(device);
		libinput_seat_unref(&seat->base);
	}
}

static void
path_seat_destroy(struct libinput_seat *seat)
{
	struct path_seat *pseat = (struct path_seat*)seat;
	free(pseat);
}

static struct path_seat*
path_seat_create(struct path_input *input,
		 const char *seat_name,
		 const char *seat_logical_name)
{
	struct path_seat *seat;

	seat = zalloc(sizeof(*seat));

	libinput_seat_init(&seat->base, &input->base, seat_name,
			   seat_logical_name, path_seat_destroy);

	return seat;
}

static struct path_seat*
path_seat_get_named(struct path_input *input,
		    const char *seat_name_physical,
		    const char *seat_name_logical)
{
	struct path_seat *seat;

	list_for_each(seat, &input->base.seat_list, base.link) {
		if (streq(seat->base.physical_name, seat_name_physical) &&
		    streq(seat->base.logical_name, seat_name_logical))
			return seat;
	}

	return NULL;
}

static struct path_seat *
path_seat_get_for_device(struct path_input *input,
			 struct udev_device *udev_device,
			 const char *seat_logical_name_override)
{
	struct path_seat *seat = NULL;
	char *seat_name = NULL, *seat_logical_name = NULL;
	const char *seat_prop;

	const char *devnode, *sysname;

	devnode = udev_device_get_devnode(udev_device);
	sysname = udev_device_get_sysname(udev_device);

	seat_prop = udev_device_get_property_value(udev_device, "ID_SEAT");
	seat_name = safe_strdup(seat_prop ? seat_prop : default_seat);

	if (seat_logical_name_override) {
		seat_logical_name = safe_strdup(seat_logical_name_override);
	} else {
		seat_prop = udev_device_get_property_value(udev_device, "WL_SEAT");
		seat_logical_name = safe_strdup(seat_prop ? seat_prop : default_seat_name);
	}

	if (!seat_logical_name) {
		log_error(&input->base,
			  "%s: failed to create seat name for device '%s'.\n",
			  sysname,
			  devnode);
		goto out;
	}

	seat = path_seat_get_named(input, seat_name, seat_logical_name);

	if (!seat)
		seat = path_seat_create(input, seat_name, seat_logical_name);
	if (!seat) {
		log_info(&input->base,
			 "%s: failed to create seat for device '%s'.\n",
			 sysname,
			 devnode);
		goto out;
	}

	libinput_seat_ref(&seat->base);
out:
	free(seat_name);
	free(seat_logical_name);

	return seat;
}

static struct libinput_device *
path_device_enable(struct path_input *input,
		   struct udev_device *udev_device,
		   const char *seat_logical_name_override)
{
	struct path_seat *seat;
	struct evdev_device *device = NULL;
	const char *output_name;
	const char *devnode, *sysname;

	devnode = udev_device_get_devnode(udev_device);
	sysname = udev_device_get_sysname(udev_device);

	seat = path_seat_get_for_device(input, udev_device, seat_logical_name_override);
	if (!seat)
		goto out;

	device = evdev_device_create(&seat->base, udev_device);
	libinput_seat_unref(&seat->base);

	if (device == EVDEV_UNHANDLED_DEVICE) {
		device = NULL;
		log_info(&input->base,
			 "%-7s - not using input device '%s'.\n",
			 sysname,
			 devnode);
		goto out;
	} else if (device == NULL) {
		log_info(&input->base,
			 "%-7s - failed to create input device '%s'.\n",
			 sysname,
			 devnode);
		goto out;
	}

	evdev_read_calibration_prop(device);
	output_name = udev_device_get_property_value(udev_device, "WL_OUTPUT");
	device->output_name = safe_strdup(output_name);

out:
	return device ? &device->base : NULL;
}

static int
path_input_enable(struct libinput *libinput)
{
	struct path_input *input = (struct path_input*)libinput;
	struct path_device *dev;

	list_for_each(dev, &input->path_list, link) {
		if (path_device_enable(input, dev->udev_device, NULL) == NULL) {
			path_input_disable(libinput);
			return -1;
		}
	}

	return 0;
}

static void
path_device_destroy(struct path_device *dev)
{
	list_remove(&dev->link);
	udev_device_unref(dev->udev_device);
	free(dev);
}

static void
path_input_destroy(struct libinput *input)
{
	struct path_input *path_input = (struct path_input*)input;
	struct path_device *dev;

	udev_unref(path_input->udev);

	list_for_each_safe(dev, &path_input->path_list, link)
		path_device_destroy(dev);

}

static struct libinput_device *
path_create_device(struct libinput *libinput,
		   struct udev_device *udev_device,
		   const char *seat_name)
{
	struct path_input *input = (struct path_input*)libinput;
	struct path_device *dev;
	struct libinput_device *device;

	dev = zalloc(sizeof *dev);
	dev->udev_device = udev_device_ref(udev_device);

	list_insert(&input->path_list, &dev->link);

	device = path_device_enable(input, udev_device, seat_name);

	if (!device)
		path_device_destroy(dev);

	return device;
}

static int
path_device_change_seat(struct libinput_device *device,
			const char *seat_name)
{
	struct libinput *libinput = device->seat->libinput;
	struct evdev_device *evdev = evdev_device(device);
	struct udev_device *udev_device = NULL;
	int rc = -1;

	udev_device = evdev->udev_device;
	udev_device_ref(udev_device);
	libinput_path_remove_device(device);

	if (path_create_device(libinput, udev_device, seat_name) != NULL)
		rc = 0;
	udev_device_unref(udev_device);
	return rc;
}

static const struct libinput_interface_backend interface_backend = {
	.resume = path_input_enable,
	.suspend = path_input_disable,
	.destroy = path_input_destroy,
	.device_change_seat = path_device_change_seat,
};

LIBINPUT_EXPORT struct libinput *
libinput_path_create_context(const struct libinput_interface *interface,
			     void *user_data)
{
	struct path_input *input;
	struct udev *udev;

	if (!interface)
		return NULL;

	udev = udev_new();
	if (!udev)
		return NULL;

	input = zalloc(sizeof *input);
	if (libinput_init(&input->base, interface,
			  &interface_backend, user_data) != 0) {
		udev_unref(udev);
		free(input);
		return NULL;
	}

	input->udev = udev;
	list_init(&input->path_list);

	return &input->base;
}

static inline struct udev_device *
udev_device_from_devnode(struct libinput *libinput,
			 struct udev *udev,
			 const char *devnode)
{
	struct udev_device *dev;
	struct stat st;
	size_t count = 0;

	if (stat(devnode, &st) < 0)
		return NULL;

	dev = udev_device_new_from_devnum(udev, 'c', st.st_rdev);

	while (dev && !udev_device_get_is_initialized(dev)) {
		udev_device_unref(dev);
		count++;
		if (count > 200) {
			log_bug_libinput(libinput,
					"udev device never initialized (%s)\n",
					devnode);
			return NULL;
		}
		msleep(10);
		dev = udev_device_new_from_devnum(udev, 'c', st.st_rdev);
	}

	return dev;
}

LIBINPUT_EXPORT struct libinput_device *
libinput_path_add_device(struct libinput *libinput,
			 const char *path)
{
	struct path_input *input = (struct path_input *)libinput;
	struct udev *udev = input->udev;
	struct udev_device *udev_device;
	struct libinput_device *device;

	if (strlen(path) > PATH_MAX) {
		log_bug_client(libinput,
			       "Unexpected path, limited to %d characters.\n",
			       PATH_MAX);
		return NULL;
	}

	if (libinput->interface_backend != &interface_backend) {
		log_bug_client(libinput, "Mismatching backends.\n");
		return NULL;
	}

	udev_device = udev_device_from_devnode(libinput, udev, path);
	if (!udev_device) {
		log_bug_client(libinput, "Invalid path %s\n", path);
		return NULL;
	}

	if (ignore_litest_test_suite_device(udev_device)) {
		udev_device_unref(udev_device);
		return NULL;
	}

	/* We cannot do this during path_create_context because the log
	 * handler isn't set up there but we really want to log to the right
	 * place if the quirks run into parser errors. So we have to do it
	 * on the first call to add_device.
	 */
	libinput_init_quirks(libinput);

	device = path_create_device(libinput, udev_device, NULL);
	udev_device_unref(udev_device);
	return device;
}

LIBINPUT_EXPORT void
libinput_path_remove_device(struct libinput_device *device)
{
	struct libinput *libinput = device->seat->libinput;
	struct path_input *input = (struct path_input*)libinput;
	struct libinput_seat *seat;
	struct evdev_device *evdev = evdev_device(device);
	struct path_device *dev;

	if (libinput->interface_backend != &interface_backend) {
		log_bug_client(libinput, "Mismatching backends.\n");
		return;
	}

	list_for_each_safe(dev, &input->path_list, link) {
		if (dev->udev_device == evdev->udev_device) {
			path_device_destroy(dev);
			break;
		}
	}

	seat = device->seat;
	libinput_seat_ref(seat);
	path_disable_device(evdev);
	libinput_seat_unref(seat);
}
