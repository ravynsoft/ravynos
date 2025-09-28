/*
 * Copyright © 2014 Red Hat, Inc.
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
#include <libinput-util.h>
#include <unistd.h>
#include <stdarg.h>

#include "litest.h"
#include "libinput-util.h"

static int open_restricted(const char *path, int flags, void *data)
{
	int fd = open(path, flags);
	return fd < 0 ? -errno : fd;
}
static void close_restricted(int fd, void *data)
{
	close(fd);
}

static const struct libinput_interface simple_interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};

static struct libevdev_uinput *
create_simple_test_device(const char *name, ...)
{
	va_list args;
	struct libevdev_uinput *uinput;
	struct libevdev *evdev;
	unsigned int type, code;
	int rc;
	struct input_absinfo abs = {
		.value = -1,
		.minimum = 0,
		.maximum = 100,
		.fuzz = 0,
		.flat = 0,
		.resolution = 100,
	};

	evdev = libevdev_new();
	litest_assert_notnull(evdev);
	libevdev_set_name(evdev, name);

	va_start(args, name);

	while ((type = va_arg(args, unsigned int)) != (unsigned int)-1 &&
	       (code = va_arg(args, unsigned int)) != (unsigned int)-1) {
		const struct input_absinfo *a = NULL;
		if (type == EV_ABS)
			a = &abs;
		libevdev_enable_event_code(evdev, type, code, a);
	}

	va_end(args);

	rc = libevdev_uinput_create_from_device(evdev,
						LIBEVDEV_UINPUT_OPEN_MANAGED,
						&uinput);
	litest_assert_int_eq(rc, 0);
	libevdev_free(evdev);

	return uinput;
}

START_TEST(event_conversion_device_notify)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_event *event;
	int device_added = 0, device_removed = 0;

	uinput = create_simple_test_device("litest test device",
					   EV_REL, REL_X,
					   EV_REL, REL_Y,
					   EV_KEY, BTN_LEFT,
					   EV_KEY, BTN_MIDDLE,
					   EV_KEY, BTN_LEFT,
					   -1, -1);
	li = litest_create_context();
	litest_restore_log_handler(li); /* use the default litest handler */
	libinput_path_add_device(li, libevdev_uinput_get_devnode(uinput));

	libinput_dispatch(li);
	libinput_suspend(li);
	libinput_resume(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_DEVICE_ADDED ||
		    type == LIBINPUT_EVENT_DEVICE_REMOVED) {
			struct libinput_event_device_notify *dn;
			struct libinput_event *base;
			dn = libinput_event_get_device_notify_event(event);
			base = libinput_event_device_notify_get_base_event(dn);
			ck_assert(event == base);

			if (type == LIBINPUT_EVENT_DEVICE_ADDED)
				device_added++;
			else if (type == LIBINPUT_EVENT_DEVICE_REMOVED)
				device_removed++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_pointer_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_gesture_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_tool_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}

		libinput_event_destroy(event);
	}

	litest_destroy_context(li);
	libevdev_uinput_destroy(uinput);

	ck_assert_int_gt(device_added, 0);
	ck_assert_int_gt(device_removed, 0);
}
END_TEST

START_TEST(event_conversion_pointer)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int motion = 0, button = 0;

	/* Queue at least two relative motion events as the first one may
	 * be absorbed by the pointer acceleration filter. */
	litest_event(dev, EV_REL, REL_X, -1);
	litest_event(dev, EV_REL, REL_Y, -1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_REL, REL_X, -1);
	litest_event(dev, EV_REL, REL_Y, -1);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_POINTER_MOTION ||
		    type == LIBINPUT_EVENT_POINTER_BUTTON) {
			struct libinput_event_pointer *p;
			struct libinput_event *base;
			p = libinput_event_get_pointer_event(event);
			base = libinput_event_pointer_get_base_event(p);
			ck_assert(event == base);

			if (type == LIBINPUT_EVENT_POINTER_MOTION)
				motion++;
			else if (type == LIBINPUT_EVENT_POINTER_BUTTON)
				button++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_gesture_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_tool_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(motion, 0);
	ck_assert_int_gt(button, 0);
}
END_TEST

START_TEST(event_conversion_pointer_abs)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int motion = 0, button = 0;

	litest_event(dev, EV_ABS, ABS_X, 10);
	litest_event(dev, EV_ABS, ABS_Y, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_ABS, ABS_X, 30);
	litest_event(dev, EV_ABS, ABS_Y, 30);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE ||
		    type == LIBINPUT_EVENT_POINTER_BUTTON) {
			struct libinput_event_pointer *p;
			struct libinput_event *base;
			p = libinput_event_get_pointer_event(event);
			base = libinput_event_pointer_get_base_event(p);
			ck_assert(event == base);

			if (type == LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE)
				motion++;
			else if (type == LIBINPUT_EVENT_POINTER_BUTTON)
				button++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_gesture_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_tool_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(motion, 0);
	ck_assert_int_gt(button, 0);
}
END_TEST

START_TEST(event_conversion_key)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int key = 0;

	litest_event(dev, EV_KEY, KEY_A, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, KEY_A, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_KEYBOARD_KEY) {
			struct libinput_event_keyboard *k;
			struct libinput_event *base;
			k = libinput_event_get_keyboard_event(event);
			base = libinput_event_keyboard_get_base_event(k);
			ck_assert(event == base);

			key++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_pointer_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_gesture_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_tool_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(key, 0);
}
END_TEST

START_TEST(event_conversion_touch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int touch = 0;

	libinput_dispatch(li);

	litest_event(dev, EV_KEY, BTN_TOOL_FINGER, 1);
	litest_event(dev, EV_KEY, BTN_TOUCH, 1);
	litest_event(dev, EV_ABS, ABS_X, 10);
	litest_event(dev, EV_ABS, ABS_Y, 10);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, 1);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_X, 10);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_Y, 10);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type >= LIBINPUT_EVENT_TOUCH_DOWN &&
		    type <= LIBINPUT_EVENT_TOUCH_FRAME) {
			struct libinput_event_touch *t;
			struct libinput_event *base;
			t = libinput_event_get_touch_event(event);
			base = libinput_event_touch_get_base_event(t);
			ck_assert(event == base);

			touch++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_pointer_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_gesture_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_tool_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(touch, 0);
}
END_TEST

START_TEST(event_conversion_gesture)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int gestures = 0;
	int i;

	libinput_dispatch(li);

	litest_touch_down(dev, 0, 70, 30);
	litest_touch_down(dev, 1, 30, 70);
	libinput_dispatch(li);
	litest_timeout_gesture_hold();

	for (i = 0; i < 8; i++) {
		litest_push_event_frame(dev);
		litest_touch_move(dev, 0, 70 - i * 5, 30 + i * 5);
		litest_touch_move(dev, 1, 30 + i * 5, 70 - i * 5);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type >= LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN &&
		    type <= LIBINPUT_EVENT_GESTURE_HOLD_END) {
			struct libinput_event_gesture *g;
			struct libinput_event *base;
			g = libinput_event_get_gesture_event(event);
			base = libinput_event_gesture_get_base_event(g);
			ck_assert(event == base);

			gestures++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_pointer_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(gestures, 0);
}
END_TEST

START_TEST(event_conversion_tablet)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int events = 0;
	struct axis_replacement axes[] = {
		{ ABS_DISTANCE, 10 },
		{ -1, -1 }
	};

	litest_tablet_proximity_in(dev, 50, 50, axes);
	litest_tablet_motion(dev, 60, 50, axes);
	litest_button_click(dev, BTN_STYLUS, true);
	litest_button_click(dev, BTN_STYLUS, false);

	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type >= LIBINPUT_EVENT_TABLET_TOOL_AXIS &&
		    type <= LIBINPUT_EVENT_TABLET_TOOL_BUTTON) {
			struct libinput_event_tablet_tool *t;
			struct libinput_event *base;
			t = libinput_event_get_tablet_tool_event(event);
			base = libinput_event_tablet_tool_get_base_event(t);
			ck_assert(event == base);

			events++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_pointer_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(events, 0);
}
END_TEST

START_TEST(event_conversion_tablet_pad)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int events = 0;

	litest_button_click(dev, BTN_0, true);
	litest_pad_ring_start(dev, 10);
	litest_pad_ring_end(dev);

	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type >= LIBINPUT_EVENT_TABLET_PAD_BUTTON &&
		    type <= LIBINPUT_EVENT_TABLET_PAD_STRIP) {
			struct libinput_event_tablet_pad *p;
			struct libinput_event *base;

			p = libinput_event_get_tablet_pad_event(event);
			base = libinput_event_tablet_pad_get_base_event(p);
			ck_assert(event == base);

			events++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_pointer_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_tool_event(event) == NULL);
			ck_assert(libinput_event_get_switch_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(events, 0);
}
END_TEST

START_TEST(event_conversion_switch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int sw = 0;

	litest_switch_action(dev,
			     LIBINPUT_SWITCH_LID,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_switch_action(dev,
			     LIBINPUT_SWITCH_LID,
			     LIBINPUT_SWITCH_STATE_OFF);
	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_SWITCH_TOGGLE) {
			struct libinput_event_switch *s;
			struct libinput_event *base;
			s = libinput_event_get_switch_event(event);
			base = libinput_event_switch_get_base_event(s);
			ck_assert(event == base);

			sw++;

			litest_disable_log_handler(li);
			ck_assert(libinput_event_get_device_notify_event(event) == NULL);
			ck_assert(libinput_event_get_keyboard_event(event) == NULL);
			ck_assert(libinput_event_get_pointer_event(event) == NULL);
			ck_assert(libinput_event_get_touch_event(event) == NULL);
			ck_assert(libinput_event_get_gesture_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_tool_event(event) == NULL);
			ck_assert(libinput_event_get_tablet_pad_event(event) == NULL);
			litest_restore_log_handler(li);
		}
		libinput_event_destroy(event);
	}

	ck_assert_int_gt(sw, 0);
}
END_TEST

START_TEST(context_ref_counting)
{
	struct libinput *li;

	/* These tests rely on valgrind to detect memory leak and use after
	 * free errors. */

	li = libinput_path_create_context(&simple_interface, NULL);
	ck_assert_notnull(li);
	ck_assert_ptr_eq(libinput_unref(li), NULL);

	li = libinput_path_create_context(&simple_interface, NULL);
	ck_assert_notnull(li);
	ck_assert_ptr_eq(libinput_ref(li), li);
	ck_assert_ptr_eq(libinput_unref(li), li);
	ck_assert_ptr_eq(libinput_unref(li), NULL);
}
END_TEST

START_TEST(config_status_string)
{
	const char *strs[3];
	const char *invalid;
	size_t i, j;

	strs[0] = libinput_config_status_to_str(LIBINPUT_CONFIG_STATUS_SUCCESS);
	strs[1] = libinput_config_status_to_str(LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	strs[2] = libinput_config_status_to_str(LIBINPUT_CONFIG_STATUS_INVALID);

	for (i = 0; i < ARRAY_LENGTH(strs) - 1; i++)
		for (j = i + 1; j < ARRAY_LENGTH(strs); j++)
			ck_assert_str_ne(strs[i], strs[j]);

	invalid = libinput_config_status_to_str(LIBINPUT_CONFIG_STATUS_INVALID + 1);
	ck_assert(invalid == NULL);
	invalid = libinput_config_status_to_str(LIBINPUT_CONFIG_STATUS_SUCCESS - 1);
	ck_assert(invalid == NULL);
}
END_TEST

static int open_restricted_leak(const char *path, int flags, void *data)
{
	return *(int*)data;
}

static void close_restricted_leak(int fd, void *data)
{
	/* noop */
}

const struct libinput_interface leak_interface = {
	.open_restricted = open_restricted_leak,
	.close_restricted = close_restricted_leak,
};

START_TEST(fd_no_event_leak)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;
	int fd = -1;
	const char *path;
	struct libinput_event *event;

	uinput = create_simple_test_device("litest test device",
					   EV_REL, REL_X,
					   EV_REL, REL_Y,
					   EV_KEY, BTN_LEFT,
					   EV_KEY, BTN_MIDDLE,
					   EV_KEY, BTN_LEFT,
					   -1, -1);
	path = libevdev_uinput_get_devnode(uinput);

	fd = open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC);
	ck_assert_int_gt(fd, -1);

	li = libinput_path_create_context(&leak_interface, &fd);
	litest_restore_log_handler(li); /* use the default litest handler */

	/* Add the device, trigger an event, then remove it again.
	 * Without it, we get a SYN_DROPPED immediately and no events.
	 */
	device = libinput_path_add_device(li, path);
	libevdev_uinput_write_event(uinput, EV_REL, REL_X, 1);
	libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0);
	libinput_path_remove_device(device);
	libinput_dispatch(li);
	litest_drain_events(li);

	/* Device is removed, but fd is still open. Queue an event, add a
	 * new device with the same fd, the queued event must be discarded
	 * by libinput */
	libevdev_uinput_write_event(uinput, EV_REL, REL_Y, 1);
	libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	libinput_path_add_device(li, path);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_DEVICE_ADDED);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);

	close(fd);
	libinput_unref(li);
	libevdev_uinput_destroy(uinput);
}
END_TEST

static void timer_offset_warning(struct libinput *libinput,
				 enum libinput_log_priority priority,
				 const char *format,
				 va_list args)
{
	struct litest_user_data *user_data = libinput_get_user_data(libinput);
	int *warning_triggered = user_data->private;

	if (priority == LIBINPUT_LOG_PRIORITY_ERROR &&
	    strstr(format, "scheduled expiry is in the past"))
		(*warning_triggered)++;
}

START_TEST(timer_offset_bug_warning)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int warning_triggered = 0;
	struct litest_user_data *user_data = libinput_get_user_data(li);

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);

	litest_timeout_tap();

	user_data->private = &warning_triggered;
	libinput_log_set_handler(li, timer_offset_warning);
	libinput_dispatch(li);

	/* triggered for touch down and touch up */
	ck_assert_int_eq(warning_triggered, 2);
	litest_restore_log_handler(li);
}
END_TEST

static void timer_delay_warning(struct libinput *libinput,
				enum libinput_log_priority priority,
				const char *format,
				va_list args)
{
	struct litest_user_data *user_data = libinput_get_user_data(libinput);
	int *warning_triggered = user_data->private;

	if (priority == LIBINPUT_LOG_PRIORITY_ERROR &&
	    strstr(format, "event processing lagging behind by"))
		(*warning_triggered)++;
}

START_TEST(timer_delay_bug_warning)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int warning_triggered = 0;
	struct litest_user_data *user_data = libinput_get_user_data(li);

	litest_drain_events(li);

	user_data->private = &warning_triggered;
	libinput_log_set_handler(li, timer_delay_warning);

	for (int i = 0; i < 20; i++) {
		litest_event(dev, EV_REL, REL_X, -1);
		litest_event(dev, EV_SYN, SYN_REPORT, 0);
		msleep(21);
		libinput_dispatch(li);
	}

	ck_assert_int_ge(warning_triggered, 1);
	litest_restore_log_handler(li);
}
END_TEST

START_TEST(timer_flush)
{
	struct libinput *li;
	struct litest_device *keyboard, *touchpad;

	li = litest_create_context();

	touchpad = litest_add_device(li, LITEST_SYNAPTICS_TOUCHPAD);
	litest_enable_tap(touchpad->libinput_device);
	libinput_dispatch(li);
	keyboard = litest_add_device(li, LITEST_KEYBOARD);
	libinput_dispatch(li);
	litest_drain_events(li);

	/* make sure tapping works */
	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_up(touchpad, 0);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_empty_queue(li);

	/* make sure dwt-tap is ignored */
	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	libinput_dispatch(li);
	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_up(touchpad, 0);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_KEYBOARD_KEY);

	/* Ignore 'timer offset negative' warnings */
	litest_disable_log_handler(li);

	/* now mess with the timing
	   - send a key event
	   - expire dwt
	   - send a tap
	   and then call libinput_dispatch(). libinput should notice that
	   the tap event came in after the timeout and thus acknowledge the
	   tap.
	 */
	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_timeout_dwt_long();
	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_up(touchpad, 0);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);
	litest_restore_log_handler(li);

	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_PRESSED);
	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_RELEASED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_delete_device(keyboard);
	litest_delete_device(touchpad);

	litest_destroy_context(li);
}
END_TEST

START_TEST(udev_absinfo_override)
{
	struct litest_device *dev = litest_current_device();
	struct libevdev *evdev = dev->evdev;
	const struct input_absinfo *abs;
	struct udev_device *ud;
	struct udev_list_entry *entry;
	bool found_x = false, found_y = false,
	     found_mt_x = false, found_mt_y = false;

	ud = libinput_device_get_udev_device(dev->libinput_device);
	ck_assert_notnull(ud);

	/* Custom checks for this special litest device only */

	entry = udev_device_get_properties_list_entry(ud);
	while (entry) {
		const char *key, *value;

		key = udev_list_entry_get_name(entry);
		value = udev_list_entry_get_value(entry);

		if (streq(key, "EVDEV_ABS_00")) {
			found_x = true;
			ck_assert(streq(value, "1:1000:100:10"));
		}
		if (streq(key, "EVDEV_ABS_01")) {
			found_y = true;
			ck_assert(streq(value, "2:2000:200:20"));
		}
		if (streq(key, "EVDEV_ABS_35")) {
			found_mt_x = true;
			ck_assert(streq(value, "3:3000:300:30"));
		}
		if (streq(key, "EVDEV_ABS_36")) {
			found_mt_y = true;
			ck_assert(streq(value, "4:4000:400:40"));
		}

		entry = udev_list_entry_get_next(entry);
	}
	udev_device_unref(ud);

	ck_assert(found_x);
	ck_assert(found_y);
	ck_assert(found_mt_x);
	ck_assert(found_mt_y);

	abs = libevdev_get_abs_info(evdev, ABS_X);
	ck_assert_int_eq(abs->minimum, 1);
	ck_assert_int_eq(abs->maximum, 1000);
	ck_assert_int_eq(abs->resolution, 100);
	/* if everything goes well, we override the fuzz to 0 */
	ck_assert_int_eq(abs->fuzz, 0);

	abs = libevdev_get_abs_info(evdev, ABS_Y);
	ck_assert_int_eq(abs->minimum, 2);
	ck_assert_int_eq(abs->maximum, 2000);
	ck_assert_int_eq(abs->resolution, 200);
	/* if everything goes well, we override the fuzz to 0 */
	ck_assert_int_eq(abs->fuzz, 0);

	abs = libevdev_get_abs_info(evdev, ABS_MT_POSITION_X);
	ck_assert_int_eq(abs->minimum, 3);
	ck_assert_int_eq(abs->maximum, 3000);
	ck_assert_int_eq(abs->resolution, 300);
	/* if everything goes well, we override the fuzz to 0 */
	ck_assert_int_eq(abs->fuzz, 0);

	abs = libevdev_get_abs_info(evdev, ABS_MT_POSITION_Y);
	ck_assert_int_eq(abs->minimum, 4);
	ck_assert_int_eq(abs->maximum, 4000);
	ck_assert_int_eq(abs->resolution, 400);
	/* if everything goes well, we override the fuzz to 0 */
	ck_assert_int_eq(abs->fuzz, 0);
}
END_TEST

TEST_COLLECTION(misc)
{
	litest_add_no_device(event_conversion_device_notify);
	litest_add_for_device(event_conversion_pointer, LITEST_MOUSE);
	litest_add_for_device(event_conversion_pointer_abs, LITEST_XEN_VIRTUAL_POINTER);
	litest_add_for_device(event_conversion_key, LITEST_KEYBOARD);
	litest_add_for_device(event_conversion_touch, LITEST_WACOM_TOUCH);
	litest_add_for_device(event_conversion_gesture, LITEST_BCM5974);
	litest_add_for_device(event_conversion_tablet, LITEST_WACOM_CINTIQ);
	litest_add_for_device(event_conversion_tablet_pad, LITEST_WACOM_INTUOS5_PAD);
	litest_add_for_device(event_conversion_switch, LITEST_LID_SWITCH);

	litest_add_deviceless(context_ref_counting);
	litest_add_deviceless(config_status_string);

	litest_add_for_device(timer_offset_bug_warning, LITEST_SYNAPTICS_TOUCHPAD);
	litest_add_for_device(timer_delay_bug_warning, LITEST_MOUSE);
	litest_add_no_device(timer_flush);

	litest_add_no_device(fd_no_event_leak);

	litest_add_for_device(udev_absinfo_override, LITEST_ABSINFO_OVERRIDE);
}
