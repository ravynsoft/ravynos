/*
 * Copyright © 2013 Red Hat, Inc.
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

#include <config.h>

#include <check.h>
#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "litest.h"
#include "libinput-util.h"

struct counter {
	int open_func_count;
	int close_func_count;
};

static int
open_restricted_count(const char *path, int flags, void *data)
{
	struct counter *c = data;
	int fd;

	c->open_func_count++;

	fd = open(path, flags);
	return fd < 0 ? -errno : fd;
}

static void
close_restricted_count(int fd, void *data)
{
	struct counter *c = data;

	c->close_func_count++;
	close(fd);
}

static const struct libinput_interface counting_interface = {
	.open_restricted = open_restricted_count,
	.close_restricted = close_restricted_count,
};

static int
open_restricted(const char *path, int flags, void *data)
{
	int fd = open(path, flags);
	return fd < 0 ? -errno : fd;
}

static void
close_restricted(int fd, void *data)
{
	close(fd);
}

static const struct libinput_interface simple_interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};

START_TEST(path_create_NULL)
{
	struct libinput *li;
	struct counter counter;

	counter.open_func_count = 0;
	counter.close_func_count = 0;

	li = libinput_path_create_context(NULL, NULL);
	ck_assert(li == NULL);
	li = libinput_path_create_context(&counting_interface, &counter);
	ck_assert_notnull(li);
	libinput_unref(li);

	ck_assert_int_eq(counter.open_func_count, 0);
	ck_assert_int_eq(counter.close_func_count, 0);
}
END_TEST

START_TEST(path_create_invalid)
{
	struct libinput *li;
	struct libinput_device *device;
	const char *path = "/tmp";
	struct counter counter;

	counter.open_func_count = 0;
	counter.close_func_count = 0;

	li = libinput_path_create_context(&counting_interface, &counter);
	ck_assert_notnull(li);

	litest_disable_log_handler(li);

	device = libinput_path_add_device(li, path);
	ck_assert(device == NULL);

	ck_assert_int_eq(counter.open_func_count, 0);
	ck_assert_int_eq(counter.close_func_count, 0);

	litest_restore_log_handler(li);
	libinput_unref(li);
	ck_assert_int_eq(counter.close_func_count, 0);
}
END_TEST

START_TEST(path_create_invalid_kerneldev)
{
	struct libinput *li;
	struct libinput_device *device;
	const char *path = "/dev/uinput";
	struct counter counter;

	counter.open_func_count = 0;
	counter.close_func_count = 0;

	li = libinput_path_create_context(&counting_interface, &counter);
	ck_assert_notnull(li);

	litest_disable_log_handler(li);

	device = libinput_path_add_device(li, path);
	ck_assert(device == NULL);

	ck_assert_int_eq(counter.open_func_count, 1);
	ck_assert_int_eq(counter.close_func_count, 1);

	litest_restore_log_handler(li);
	libinput_unref(li);
	ck_assert_int_eq(counter.close_func_count, 1);
}
END_TEST

START_TEST(path_create_invalid_file)
{
	struct libinput *li;
	struct libinput_device *device;
	char path[] = "/tmp/litest_path_XXXXXX";
	int fd;
	struct counter counter;

	umask(002);
	fd = mkstemp(path);
	ck_assert_int_ge(fd, 0);
	close(fd);

	counter.open_func_count = 0;
	counter.close_func_count = 0;

	li = libinput_path_create_context(&counting_interface, &counter);
	unlink(path);

	litest_disable_log_handler(li);

	ck_assert_notnull(li);
	device = libinput_path_add_device(li, path);
	ck_assert(device == NULL);

	ck_assert_int_eq(counter.open_func_count, 0);
	ck_assert_int_eq(counter.close_func_count, 0);

	litest_restore_log_handler(li);
	libinput_unref(li);
	ck_assert_int_eq(counter.close_func_count, 0);
}
END_TEST

START_TEST(path_create_pathmax_file)
{
	struct libinput *li;
	struct libinput_device *device;
	char *path;
	struct counter counter;

	path = zalloc(PATH_MAX * 2);
	memset(path, 'a', PATH_MAX * 2 - 1);

	counter.open_func_count = 0;
	counter.close_func_count = 0;

	li = libinput_path_create_context(&counting_interface, &counter);

	litest_set_log_handler_bug(li);
	ck_assert_notnull(li);
	device = libinput_path_add_device(li, path);
	ck_assert(device == NULL);

	ck_assert_int_eq(counter.open_func_count, 0);
	ck_assert_int_eq(counter.close_func_count, 0);

	litest_restore_log_handler(li);
	libinput_unref(li);
	ck_assert_int_eq(counter.close_func_count, 0);

	free(path);
}
END_TEST


START_TEST(path_create_destroy)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libevdev_uinput *uinput;
	struct counter counter;

	counter.open_func_count = 0;
	counter.close_func_count = 0;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);

	li = libinput_path_create_context(&counting_interface, &counter);
	ck_assert_notnull(li);

	litest_disable_log_handler(li);

	ck_assert(libinput_get_user_data(li) == &counter);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	ck_assert_int_eq(counter.open_func_count, 1);

	libevdev_uinput_destroy(uinput);
	libinput_unref(li);
	ck_assert_int_eq(counter.close_func_count, 1);
}
END_TEST

START_TEST(path_force_destroy)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li;
	struct libinput_device *device;

	li = libinput_path_create_context(&simple_interface, NULL);
	ck_assert_notnull(li);
	libinput_ref(li);
	device = libinput_path_add_device(li,
				  libevdev_uinput_get_devnode(dev->uinput));
	ck_assert_notnull(device);

	while (libinput_unref(li) != NULL)
		;
}
END_TEST

START_TEST(path_set_user_data)
{
	struct libinput *li;
	int data1, data2;

	li = libinput_path_create_context(&simple_interface, &data1);
	ck_assert_notnull(li);
	ck_assert(libinput_get_user_data(li) == &data1);
	libinput_set_user_data(li, &data2);
	ck_assert(libinput_get_user_data(li) == &data2);

	libinput_unref(li);
}
END_TEST

START_TEST(path_added_seat)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_device *device;
	struct libinput_seat *seat;
	const char *seat_name;
	enum libinput_event_type type;

	libinput_dispatch(li);

	event = libinput_get_event(li);
	ck_assert_notnull(event);

	type = libinput_event_get_type(event);
	ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);

	device = libinput_event_get_device(event);
	seat = libinput_device_get_seat(device);
	ck_assert_notnull(seat);

	seat_name = libinput_seat_get_logical_name(seat);
	ck_assert_str_eq(seat_name, "default");

	libinput_event_destroy(event);
}
END_TEST

START_TEST(path_seat_change)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_device *device;
	struct libinput_seat *seat1, *seat2;
	const char *seat1_name;
	const char *seat2_name = "new seat";
	int rc;

	libinput_dispatch(li);

	event = libinput_get_event(li);
	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_DEVICE_ADDED);

	device = libinput_event_get_device(event);
	libinput_device_ref(device);

	seat1 = libinput_device_get_seat(device);
	libinput_seat_ref(seat1);

	seat1_name = libinput_seat_get_logical_name(seat1);
	libinput_event_destroy(event);

	litest_drain_events(li);

	rc = libinput_device_set_seat_logical_name(device,
						   seat2_name);
	ck_assert_int_eq(rc, 0);

	libinput_dispatch(li);

	event = libinput_get_event(li);
	ck_assert_notnull(event);

	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_DEVICE_REMOVED);

	ck_assert(libinput_event_get_device(event) == device);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	ck_assert_notnull(event);
	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_DEVICE_ADDED);
	ck_assert(libinput_event_get_device(event) != device);
	libinput_device_unref(device);

	device = libinput_event_get_device(event);
	seat2 = libinput_device_get_seat(device);

	ck_assert_str_ne(libinput_seat_get_logical_name(seat2),
			 seat1_name);
	ck_assert_str_eq(libinput_seat_get_logical_name(seat2),
			 seat2_name);
	libinput_event_destroy(event);

	libinput_seat_unref(seat1);

	/* litest: swap the new device in, so cleanup works */
	libinput_device_unref(dev->libinput_device);
	libinput_device_ref(device);
	dev->libinput_device = device;
}
END_TEST

START_TEST(path_added_device)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_device *device;
	enum libinput_event_type type;

	libinput_dispatch(li);

	event = libinput_get_event(li);
	ck_assert_notnull(event);
	type = libinput_event_get_type(event);
	ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
	device = libinput_event_get_device(event);
	ck_assert_notnull(device);

	libinput_event_destroy(event);
}
END_TEST

START_TEST(path_add_device)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_device *device;
	char *sysname1 = NULL, *sysname2 = NULL;
	enum libinput_event_type type;

	libinput_dispatch(li);

	event = libinput_get_event(li);
	ck_assert_notnull(event);
	type = libinput_event_get_type(event);
	ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
	device = libinput_event_get_device(event);
	ck_assert_notnull(device);
	sysname1 = safe_strdup(libinput_device_get_sysname(device));
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(dev->uinput));
	ck_assert_notnull(device);

	libinput_dispatch(li);

	event = libinput_get_event(li);
	ck_assert_notnull(event);
	type = libinput_event_get_type(event);
	ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
	device = libinput_event_get_device(event);
	ck_assert_notnull(device);
	sysname2 = safe_strdup(libinput_device_get_sysname(device));
	libinput_event_destroy(event);

	ck_assert_str_eq(sysname1, sysname2);

	free(sysname1);
	free(sysname2);
}
END_TEST

START_TEST(path_add_invalid_path)
{
	struct libinput *li;
	struct libinput_device *device;

	li = litest_create_context();

	litest_disable_log_handler(li);
	device = libinput_path_add_device(li, "/tmp/");
	litest_restore_log_handler(li);
	ck_assert(device == NULL);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);

	litest_destroy_context(li);
}
END_TEST

START_TEST(path_device_sysname)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_event *ev;
	struct libinput_device *device;
	const char *sysname;
	enum libinput_event_type type;

	libinput_dispatch(dev->libinput);

	ev = libinput_get_event(dev->libinput);
	ck_assert_notnull(ev);
	type = libinput_event_get_type(ev);
	ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
	device = libinput_event_get_device(ev);
	ck_assert_notnull(device);
	sysname = libinput_device_get_sysname(device);

	ck_assert_notnull(sysname);
	ck_assert_int_gt(strlen(sysname), 1);
	ck_assert(strchr(sysname, '/') == NULL);
	ck_assert(strneq(sysname, "event", 5));

	libinput_event_destroy(ev);
}
END_TEST

START_TEST(path_remove_device)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_device *device;
	int remove_event = 0;

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(dev->uinput));
	ck_assert_notnull(device);
	litest_drain_events(li);

	libinput_path_remove_device(device);
	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_DEVICE_REMOVED)
			remove_event++;

		libinput_event_destroy(event);
	}

	ck_assert_int_eq(remove_event, 1);
}
END_TEST

START_TEST(path_double_remove_device)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_device *device;
	int remove_event = 0;

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(dev->uinput));
	ck_assert_notnull(device);
	litest_drain_events(li);

	libinput_path_remove_device(device);
	libinput_path_remove_device(device);
	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_DEVICE_REMOVED)
			remove_event++;

		libinput_event_destroy(event);
	}

	ck_assert_int_eq(remove_event, 1);
}
END_TEST

START_TEST(path_suspend)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libevdev_uinput *uinput;
	int rc;
	void *userdata = &rc;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);

	li = libinput_path_create_context(&simple_interface, userdata);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	libinput_suspend(li);
	libinput_resume(li);

	libevdev_uinput_destroy(uinput);
	libinput_unref(li);
}
END_TEST

START_TEST(path_double_suspend)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libevdev_uinput *uinput;
	int rc;
	void *userdata = &rc;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);

	li = libinput_path_create_context(&simple_interface, userdata);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	libinput_suspend(li);
	libinput_suspend(li);
	libinput_resume(li);

	libevdev_uinput_destroy(uinput);
	libinput_unref(li);
}
END_TEST

START_TEST(path_double_resume)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libevdev_uinput *uinput;
	int rc;
	void *userdata = &rc;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);

	li = libinput_path_create_context(&simple_interface, userdata);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	libinput_suspend(li);
	libinput_resume(li);
	libinput_resume(li);

	libevdev_uinput_destroy(uinput);
	libinput_unref(li);
}
END_TEST

START_TEST(path_add_device_suspend_resume)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libinput_event *event;
	struct libevdev_uinput *uinput1, *uinput2;
	int rc;
	int nevents;
	void *userdata = &rc;

	uinput1 = litest_create_uinput_device("test device", NULL,
					      EV_KEY, BTN_LEFT,
					      EV_KEY, BTN_RIGHT,
					      EV_REL, REL_X,
					      EV_REL, REL_Y,
					      -1);
	uinput2 = litest_create_uinput_device("test device 2", NULL,
					      EV_KEY, BTN_LEFT,
					      EV_KEY, BTN_RIGHT,
					      EV_REL, REL_X,
					      EV_REL, REL_Y,
					      -1);

	li = libinput_path_create_context(&simple_interface, userdata);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput1));
	ck_assert_notnull(device);
	libinput_path_add_device(li, libevdev_uinput_get_devnode(uinput2));

	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	libinput_suspend(li);
	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_REMOVED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	libinput_resume(li);
	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	libevdev_uinput_destroy(uinput1);
	libevdev_uinput_destroy(uinput2);
	libinput_unref(li);
}
END_TEST

START_TEST(path_add_device_suspend_resume_fail)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libinput_event *event;
	struct libevdev_uinput *uinput1, *uinput2;
	int rc;
	int nevents;
	void *userdata = &rc;

	uinput1 = litest_create_uinput_device("test device", NULL,
					      EV_KEY, BTN_LEFT,
					      EV_KEY, BTN_RIGHT,
					      EV_REL, REL_X,
					      EV_REL, REL_Y,
					      -1);
	uinput2 = litest_create_uinput_device("test device 2", NULL,
					      EV_KEY, BTN_LEFT,
					      EV_KEY, BTN_RIGHT,
					      EV_REL, REL_X,
					      EV_REL, REL_Y,
					      -1);

	li = libinput_path_create_context(&simple_interface, userdata);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput1));
	ck_assert_notnull(device);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput2));
	ck_assert_notnull(device);

	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	libinput_suspend(li);
	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_REMOVED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	/* now drop one of the devices */
	libevdev_uinput_destroy(uinput1);
	rc = libinput_resume(li);
	ck_assert_int_eq(rc, -1);

	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		/* We expect one device being added, second one fails,
		 * causing a removed event for the first one */
		if (type != LIBINPUT_EVENT_DEVICE_ADDED &&
		    type != LIBINPUT_EVENT_DEVICE_REMOVED)
			ck_abort();
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	libevdev_uinput_destroy(uinput2);
	libinput_unref(li);
}
END_TEST

START_TEST(path_add_device_suspend_resume_remove_device)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libinput_event *event;
	struct libevdev_uinput *uinput1, *uinput2;
	int rc;
	int nevents;
	void *userdata = &rc;

	uinput1 = litest_create_uinput_device("test device", NULL,
					      EV_KEY, BTN_LEFT,
					      EV_KEY, BTN_RIGHT,
					      EV_REL, REL_X,
					      EV_REL, REL_Y,
					      -1);
	uinput2 = litest_create_uinput_device("test device 2", NULL,
					      EV_KEY, BTN_LEFT,
					      EV_KEY, BTN_RIGHT,
					      EV_REL, REL_X,
					      EV_REL, REL_Y,
					      -1);

	li = libinput_path_create_context(&simple_interface, userdata);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput1));
	ck_assert_notnull(device);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput2));

	libinput_device_ref(device);
	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	libinput_suspend(li);
	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_REMOVED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 2);

	/* now drop and remove one of the devices */
	libevdev_uinput_destroy(uinput2);
	libinput_path_remove_device(device);
	libinput_device_unref(device);

	rc = libinput_resume(li);
	ck_assert_int_eq(rc, 0);

	libinput_dispatch(li);

	nevents = 0;
	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);
		ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
		libinput_event_destroy(event);
		nevents++;
	}

	ck_assert_int_eq(nevents, 1);

	libevdev_uinput_destroy(uinput1);
	libinput_unref(li);
}
END_TEST

START_TEST(path_device_gone)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libevdev_uinput *uinput;
	struct libinput_event *event;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);

	li = libinput_path_create_context(&simple_interface, NULL);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	litest_drain_events(li);

	libevdev_uinput_destroy(uinput);

	libinput_dispatch(li);

	event = libinput_get_event(li);
	ck_assert_notnull(event);
	litest_assert_event_type(event, LIBINPUT_EVENT_DEVICE_REMOVED);
	libinput_event_destroy(event);

	libinput_unref(li);
}
END_TEST

START_TEST(path_seat_recycle)
{
	struct libinput *li;
	struct libevdev_uinput *uinput;
	int rc;
	void *userdata = &rc;
	struct libinput_event *ev;
	struct libinput_device *device;
	struct libinput_seat *saved_seat = NULL;
	struct libinput_seat *seat;
	int data = 0;
	int found = 0;
	void *user_data;
	enum libinput_event_type type;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);

	li = libinput_path_create_context(&simple_interface, userdata);
	ck_assert_notnull(li);

	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	libinput_dispatch(li);

	ev = libinput_get_event(li);
	ck_assert_notnull(ev);
	type = libinput_event_get_type(ev);
	ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
	device = libinput_event_get_device(ev);
	ck_assert_notnull(device);
	saved_seat = libinput_device_get_seat(device);
	libinput_seat_set_user_data(saved_seat, &data);
	libinput_seat_ref(saved_seat);
	libinput_event_destroy(ev);
	ck_assert_notnull(saved_seat);

	litest_assert_empty_queue(li);

	libinput_suspend(li);

	litest_drain_events(li);

	libinput_resume(li);

	libinput_dispatch(li);
	ev = libinput_get_event(li);
	ck_assert_notnull(ev);
	type = libinput_event_get_type(ev);
	ck_assert_int_eq(type, LIBINPUT_EVENT_DEVICE_ADDED);
	device = libinput_event_get_device(ev);
	ck_assert_notnull(device);

	seat = libinput_device_get_seat(device);
	user_data = libinput_seat_get_user_data(seat);
	if (user_data == &data) {
		found = 1;
		ck_assert(seat == saved_seat);
	}

	libinput_event_destroy(ev);
	ck_assert(found == 1);

	libinput_unref(li);

	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(path_udev_assign_seat)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int rc;

	litest_set_log_handler_bug(li);
	rc = libinput_udev_assign_seat(li, "foo");
	ck_assert_int_eq(rc, -1);
	litest_restore_log_handler(li);
}
END_TEST

START_TEST(path_ignore_device)
{
	struct litest_device *dev;
	struct libinput *li;
	struct libinput_device *device;
	const char *path;

	dev = litest_create(LITEST_IGNORED_MOUSE, NULL, NULL, NULL, NULL);
	path = libevdev_uinput_get_devnode(dev->uinput);
	ck_assert_notnull(path);

	li = litest_create_context();
	device = libinput_path_add_device(li, path);
	ck_assert(device == NULL);

	litest_destroy_context(li);
	litest_delete_device(dev);
}
END_TEST

TEST_COLLECTION(path)
{
	litest_add_no_device(path_create_NULL);
	litest_add_no_device(path_create_invalid);
	litest_add_no_device(path_create_invalid_file);
	litest_add_no_device(path_create_invalid_kerneldev);
	litest_add_no_device(path_create_pathmax_file);
	litest_add_no_device(path_create_destroy);
	litest_add(path_force_destroy, LITEST_ANY, LITEST_ANY);
	litest_add_no_device(path_set_user_data);
	litest_add_no_device(path_suspend);
	litest_add_no_device(path_double_suspend);
	litest_add_no_device(path_double_resume);
	litest_add_no_device(path_add_device_suspend_resume);
	litest_add_no_device(path_add_device_suspend_resume_fail);
	litest_add_no_device(path_add_device_suspend_resume_remove_device);
	litest_add_for_device(path_added_seat, LITEST_SYNAPTICS_CLICKPAD_X220);
	litest_add_for_device(path_seat_change, LITEST_SYNAPTICS_CLICKPAD_X220);
	litest_add(path_added_device, LITEST_ANY, LITEST_ANY);
	litest_add(path_device_sysname, LITEST_ANY, LITEST_ANY);
	litest_add_for_device(path_add_device, LITEST_SYNAPTICS_CLICKPAD_X220);
	litest_add_no_device(path_add_invalid_path);
	litest_add_for_device(path_remove_device, LITEST_SYNAPTICS_CLICKPAD_X220);
	litest_add_for_device(path_double_remove_device, LITEST_SYNAPTICS_CLICKPAD_X220);
	litest_add_no_device(path_device_gone);
	litest_add_no_device(path_seat_recycle);
	litest_add_for_device(path_udev_assign_seat, LITEST_SYNAPTICS_CLICKPAD_X220);

	litest_add_no_device(path_ignore_device);
}
