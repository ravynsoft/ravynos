/*
 * Copyright © 2018 Red Hat, Inc.
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
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>

#include "libinput-util.h"
#include "evdev-tablet.h"
#include "litest.h"
#include "util-input-event.h"

START_TEST(totem_type)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;
	struct libinput_tablet_tool *tool;

	litest_drain_events(li);

	litest_tablet_proximity_in(dev, 50, 50, NULL);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	tool = libinput_event_tablet_tool_get_tool(t);

	ck_assert_int_eq(libinput_tablet_tool_get_type(tool),
			 LIBINPUT_TABLET_TOOL_TYPE_TOTEM);
	libinput_event_destroy(event);
}
END_TEST

START_TEST(totem_axes)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;
	struct libinput_tablet_tool *tool;

	litest_drain_events(li);

	litest_tablet_proximity_in(dev, 50, 50, NULL);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	tool = libinput_event_tablet_tool_get_tool(t);

	ck_assert(libinput_tablet_tool_has_rotation(tool));
	ck_assert(libinput_tablet_tool_has_size(tool));
	ck_assert(libinput_tablet_tool_has_button(tool, BTN_0));

	libinput_event_destroy(event);
}
END_TEST

START_TEST(totem_proximity_in_out)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;

	litest_drain_events(li);

	litest_tablet_proximity_in(dev, 50, 50, NULL);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	ck_assert_int_eq(libinput_event_tablet_tool_get_proximity_state(t),
			 LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_TIP);
	ck_assert_int_eq(libinput_event_tablet_tool_get_tip_state(t),
			 LIBINPUT_TABLET_TOOL_TIP_DOWN);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);
	litest_tablet_proximity_out(dev);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_TIP);
	ck_assert_int_eq(libinput_event_tablet_tool_get_tip_state(t),
			 LIBINPUT_TABLET_TOOL_TIP_UP);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	ck_assert_int_eq(libinput_event_tablet_tool_get_proximity_state(t),
			 LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT);
	libinput_event_destroy(event);
}
END_TEST

START_TEST(totem_proximity_in_on_init)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;
	const char *devnode;
	double x, y;
	double w, h;
	const struct input_absinfo *abs;

	abs = libevdev_get_abs_info(dev->evdev, ABS_MT_POSITION_X);
	w = absinfo_range(abs)/abs->resolution;
	abs = libevdev_get_abs_info(dev->evdev, ABS_MT_POSITION_Y);
	h = absinfo_range(abs)/abs->resolution;

	litest_tablet_proximity_in(dev, 50, 50, NULL);

	/* for simplicity, we create a new litest context */
	devnode = libevdev_uinput_get_devnode(dev->uinput);
	li = litest_create_context();
	libinput_path_add_device(li, devnode);
	libinput_dispatch(li);

	litest_wait_for_event_of_type(li,
				      LIBINPUT_EVENT_DEVICE_ADDED,
				      -1);
	event = libinput_get_event(li);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	ck_assert_int_eq(libinput_event_tablet_tool_get_proximity_state(t),
			 LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN);
	x = libinput_event_tablet_tool_get_x(t);
	y = libinput_event_tablet_tool_get_y(t);

	ck_assert_double_gt(x, w/2 - 1);
	ck_assert_double_lt(x, w/2 + 1);
	ck_assert_double_gt(y, h/2 - 1);
	ck_assert_double_lt(y, h/2 + 1);

	libinput_event_destroy(event);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_TIP);
	ck_assert_int_eq(libinput_event_tablet_tool_get_tip_state(t),
			 LIBINPUT_TABLET_TOOL_TIP_DOWN);
	x = libinput_event_tablet_tool_get_x(t);
	y = libinput_event_tablet_tool_get_y(t);

	ck_assert_double_gt(x, w/2 - 1);
	ck_assert_double_lt(x, w/2 + 1);
	ck_assert_double_gt(y, h/2 - 1);
	ck_assert_double_lt(y, h/2 + 1);

	libinput_event_destroy(event);

	litest_assert_empty_queue(li);

	litest_destroy_context(li);
}
END_TEST

START_TEST(totem_proximity_out_on_suspend)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;
	const char *devnode;

	/* for simplicity, we create a new litest context */
	devnode = libevdev_uinput_get_devnode(dev->uinput);
	li = litest_create_context();
	libinput_path_add_device(li, devnode);

	litest_tablet_proximity_in(dev, 50, 50, NULL);
	litest_drain_events(li);

	libinput_suspend(li);

	libinput_dispatch(li);
	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_TIP);
	ck_assert_int_eq(libinput_event_tablet_tool_get_tip_state(t),
			 LIBINPUT_TABLET_TOOL_TIP_UP);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	ck_assert_int_eq(libinput_event_tablet_tool_get_proximity_state(t),
			 LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT);
	libinput_event_destroy(event);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_DEVICE_REMOVED);
	litest_destroy_context(li);
}
END_TEST

START_TEST(totem_motion)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	double x = 50, y = 50;
	double current_x, current_y, old_x, old_y;

	litest_tablet_proximity_in(dev, x, y, NULL);
	litest_drain_events(li);

	for (int i = 0; i < 30; i++, x++, y--) {
		struct libinput_event_tablet_tool *t;

		litest_tablet_motion(dev, x + 1, y + 1, NULL);
		libinput_dispatch(li);

		event = libinput_get_event(li);
		t = litest_is_tablet_event(event, LIBINPUT_EVENT_TABLET_TOOL_AXIS);

		ck_assert(libinput_event_tablet_tool_x_has_changed(t));
		ck_assert(libinput_event_tablet_tool_y_has_changed(t));

		current_x = libinput_event_tablet_tool_get_x(t);
		current_y = libinput_event_tablet_tool_get_y(t);
		if (i != 0) {
			ck_assert_double_gt(current_x, old_x);
			ck_assert_double_lt(current_y, old_y);
		}
		old_x = current_x;
		old_y = current_y;

		libinput_event_destroy(event);
	}
}
END_TEST

START_TEST(totem_rotation)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	double r, old_r;
	struct axis_replacement axes[] = {
		{ ABS_MT_ORIENTATION, 50 }, /* mid-point is 0 */
		{ -1, -1 }
	};

	litest_tablet_proximity_in(dev, 50, 50, axes);
	litest_drain_events(li);

	old_r = 360;

	for (int i = 1; i < 30; i++) {
		struct libinput_event_tablet_tool *t;


		litest_axis_set_value(axes, ABS_MT_ORIENTATION, 50 + i);
		litest_tablet_motion(dev, 50, 50, axes);
		libinput_dispatch(li);

		event = libinput_get_event(li);
		t = litest_is_tablet_event(event, LIBINPUT_EVENT_TABLET_TOOL_AXIS);

		ck_assert(!libinput_event_tablet_tool_x_has_changed(t));
		ck_assert(!libinput_event_tablet_tool_y_has_changed(t));
		ck_assert(libinput_event_tablet_tool_rotation_has_changed(t));

		r = libinput_event_tablet_tool_get_rotation(t);
		ck_assert_double_lt(r, old_r);
		old_r = r;

		libinput_event_destroy(event);
	}

	old_r = 0;

	for (int i = 1; i < 30; i++) {
		struct libinput_event_tablet_tool *t;


		litest_axis_set_value(axes, ABS_MT_ORIENTATION, 50 - i);
		litest_tablet_motion(dev, 50, 50, axes);
		libinput_dispatch(li);

		event = libinput_get_event(li);
		t = litest_is_tablet_event(event, LIBINPUT_EVENT_TABLET_TOOL_AXIS);

		ck_assert(!libinput_event_tablet_tool_x_has_changed(t));
		ck_assert(!libinput_event_tablet_tool_y_has_changed(t));
		ck_assert(libinput_event_tablet_tool_rotation_has_changed(t));

		r = libinput_event_tablet_tool_get_rotation(t);
		ck_assert_double_gt(r, old_r);
		old_r = r;

		libinput_event_destroy(event);
	}
}
END_TEST

START_TEST(totem_size)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;
	double smin, smaj;

	litest_drain_events(li);

	litest_tablet_proximity_in(dev, 50, 50, NULL);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event, LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	ck_assert(libinput_event_tablet_tool_size_major_has_changed(t));
	ck_assert(libinput_event_tablet_tool_size_minor_has_changed(t));
	smaj = libinput_event_tablet_tool_get_size_major(t);
	smin = libinput_event_tablet_tool_get_size_minor(t);
	libinput_event_destroy(event);

	ck_assert_double_eq(smaj, 71.8);
	ck_assert_double_eq(smin, 71.8);

	litest_drain_events(li);
}
END_TEST

START_TEST(totem_button)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;

	litest_tablet_proximity_in(dev, 30, 40, NULL);
	litest_drain_events(li);

	litest_button_click(dev, BTN_0, true);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	t = litest_is_tablet_event(event, LIBINPUT_EVENT_TABLET_TOOL_BUTTON);
	ck_assert_int_eq(libinput_event_tablet_tool_get_button(t), BTN_0);
	ck_assert_int_eq(libinput_event_tablet_tool_get_button_state(t),
			 LIBINPUT_BUTTON_STATE_PRESSED);
	ck_assert_int_eq(libinput_event_tablet_tool_get_tip_state(t),
			 LIBINPUT_TABLET_TOOL_TIP_DOWN);
	libinput_event_destroy(event);

	litest_button_click(dev, BTN_0, false);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event, LIBINPUT_EVENT_TABLET_TOOL_BUTTON);
	ck_assert_int_eq(libinput_event_tablet_tool_get_button(t), BTN_0);
	ck_assert_int_eq(libinput_event_tablet_tool_get_button_state(t),
			 LIBINPUT_BUTTON_STATE_RELEASED);
	ck_assert_int_eq(libinput_event_tablet_tool_get_tip_state(t),
			 LIBINPUT_TABLET_TOOL_TIP_DOWN);
	libinput_event_destroy(event);
}
END_TEST

START_TEST(totem_button_down_on_init)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li;
	struct libinput_event *event;
	struct libinput_event_tablet_tool *t;
	const char *devnode;

	litest_tablet_proximity_in(dev, 50, 50, NULL);
	litest_button_click(dev, BTN_0, true);

	/* for simplicity, we create a new litest context */
	devnode = libevdev_uinput_get_devnode(dev->uinput);
	li = litest_create_context();
	libinput_path_add_device(li, devnode);
	libinput_dispatch(li);

	litest_wait_for_event_of_type(li,
				      LIBINPUT_EVENT_DEVICE_ADDED,
				      -1);
	event = libinput_get_event(li);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
	ck_assert_int_eq(libinput_event_tablet_tool_get_proximity_state(t),
			 LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN);

	libinput_event_destroy(event);

	event = libinput_get_event(li);
	t = litest_is_tablet_event(event,
				   LIBINPUT_EVENT_TABLET_TOOL_TIP);
	ck_assert_int_eq(libinput_event_tablet_tool_get_tip_state(t),
			 LIBINPUT_TABLET_TOOL_TIP_DOWN);

	libinput_event_destroy(event);

	/* The button is down on init but we don't expect an event */
	litest_assert_empty_queue(li);

	litest_button_click(dev, BTN_0, false);
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

	/* but buttons after this should be sent */
	litest_button_click(dev, BTN_0, true);
	libinput_dispatch(li);
	litest_assert_tablet_button_event(li, BTN_0, LIBINPUT_BUTTON_STATE_PRESSED);
	litest_button_click(dev, BTN_0, false);
	libinput_dispatch(li);
	litest_assert_tablet_button_event(li, BTN_0, LIBINPUT_BUTTON_STATE_RELEASED);

	litest_destroy_context(li);
}
END_TEST

START_TEST(totem_button_up_on_delete)
{
	struct libinput *li = litest_create_context();
	struct litest_device *dev = litest_add_device(li, LITEST_DELL_CANVAS_TOTEM);
	struct libevdev *evdev = libevdev_new();

	litest_tablet_proximity_in(dev, 10, 10, NULL);
	litest_drain_events(li);

	litest_button_click(dev, BTN_0, true);
	litest_drain_events(li);

	litest_delete_device(dev);
	libinput_dispatch(li);

	litest_assert_tablet_button_event(li,
					  BTN_0,
					  LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_tablet_tip_event(li, LIBINPUT_TABLET_TOOL_TIP_UP);
	litest_assert_tablet_proximity_event(li,
					     LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT);
	libevdev_free(evdev);
	litest_destroy_context(li);
}
END_TEST

START_TEST(totem_arbitration_below)
{
	struct litest_device *totem = litest_current_device();
	struct litest_device *touch;
	struct libinput *li = totem->libinput;

	touch = litest_add_device(li, LITEST_DELL_CANVAS_TOTEM_TOUCH);
	litest_drain_events(li);

	/* touches below the totem, cancelled once the totem is down */
	litest_touch_down(touch, 0, 50, 50);
	libinput_dispatch(li);
	litest_assert_touch_down_frame(li);
	litest_touch_move_to(touch, 0, 50, 50, 50, 70, 10);
	libinput_dispatch(li);
	while (libinput_next_event_type(li)) {
		litest_assert_touch_motion_frame(li);
	}

	litest_tablet_proximity_in(totem, 50, 70, NULL);
	libinput_dispatch(li);

	litest_assert_tablet_proximity_event(li, LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN);
	litest_assert_tablet_tip_event(li, LIBINPUT_TABLET_TOOL_TIP_DOWN);
	litest_assert_touch_cancel(li);

	litest_touch_move_to(touch, 0, 50, 70, 20, 50, 10);
	litest_assert_empty_queue(li);

	litest_tablet_motion(totem, 20, 50, NULL);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_TABLET_TOOL_AXIS);

	litest_touch_up(touch, 0);
	litest_assert_empty_queue(li);

	litest_delete_device(touch);
}
END_TEST

START_TEST(totem_arbitration_during)
{
	struct litest_device *totem = litest_current_device();
	struct litest_device *touch;
	struct libinput *li = totem->libinput;

	touch = litest_add_device(li, LITEST_DELL_CANVAS_TOTEM_TOUCH);
	litest_drain_events(li);

	litest_tablet_proximity_in(totem, 50, 50, NULL);
	libinput_dispatch(li);

	litest_drain_events(li);

	for (int i = 0; i < 3; i++) {
		litest_touch_down(touch, 0, 51, 51);
		litest_touch_move_to(touch, 0, 51, 50, 90, 80, 10);
		litest_touch_up(touch, 0);

		litest_assert_empty_queue(li);
	}

	litest_delete_device(touch);
}
END_TEST

START_TEST(totem_arbitration_outside_rect)
{
	struct litest_device *totem = litest_current_device();
	struct litest_device *touch;
	struct libinput *li = totem->libinput;

	touch = litest_add_device(li, LITEST_DELL_CANVAS_TOTEM_TOUCH);
	litest_drain_events(li);

	litest_tablet_proximity_in(totem, 50, 50, NULL);
	libinput_dispatch(li);

	litest_drain_events(li);

	for (int i = 0; i < 3; i++) {
		litest_touch_down(touch, 0, 81, 51);
		litest_touch_move_to(touch, 0, 81, 50, 90, 80, 10);
		litest_touch_up(touch, 0);
		libinput_dispatch(li);

		litest_assert_touch_sequence(li);
	}

	/* moving onto the totem is fine */
	litest_touch_down(touch, 0, 81, 51);
	litest_touch_move_to(touch, 0, 81, 50, 50, 50, 10);
	litest_touch_up(touch, 0);
	libinput_dispatch(li);

	litest_assert_touch_sequence(li);

	litest_delete_device(touch);
}
END_TEST

TEST_COLLECTION(totem)
{
	litest_add(totem_type, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_axes, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_proximity_in_out, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_proximity_in_on_init, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_proximity_out_on_suspend, LITEST_TOTEM, LITEST_ANY);

	litest_add(totem_motion, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_rotation, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_size, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_button, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_button_down_on_init, LITEST_TOTEM, LITEST_ANY);
	litest_add_no_device(totem_button_up_on_delete);

	litest_add(totem_arbitration_below, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_arbitration_during, LITEST_TOTEM, LITEST_ANY);
	litest_add(totem_arbitration_outside_rect, LITEST_TOTEM, LITEST_ANY);
}
