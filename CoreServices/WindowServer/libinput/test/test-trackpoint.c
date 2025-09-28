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
#include <unistd.h>

#include "libinput-util.h"
#include "litest.h"

static inline bool
has_disable_while_trackpointing(struct litest_device *device)
{
	return libinput_device_config_dwtp_is_available(device->libinput_device);
}

START_TEST(trackpoint_middlebutton)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint64_t ptime, rtime;

	litest_drain_events(li);

	/* A quick middle button click should get reported normally */
	litest_button_click_debounced(dev, li, BTN_MIDDLE, 1);
	msleep(2);
	litest_button_click_debounced(dev, li, BTN_MIDDLE, 0);

	litest_wait_for_event(li);

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_MIDDLE,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	ptime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_MIDDLE,
				       LIBINPUT_BUTTON_STATE_RELEASED);
	rtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);

	ck_assert_int_lt(ptime, rtime);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(trackpoint_scroll)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_button_scroll(dev, BTN_MIDDLE, 1, 6);
	litest_assert_scroll(li,
			     LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			     LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
			     6);
	litest_button_scroll(dev, BTN_MIDDLE, 1, -7);
	litest_assert_scroll(li,
			     LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			     LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
			     -7);
	litest_button_scroll(dev, BTN_MIDDLE, 8, 1);
	litest_assert_scroll(li,
			     LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			     LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
			     8);
	litest_button_scroll(dev, BTN_MIDDLE, -9, 1);
	litest_assert_scroll(li,
			     LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
			     LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
			     -9);

	/* scroll smaller than the threshold should not generate axis events */
	litest_button_scroll(dev, BTN_MIDDLE, 1, 1);

	litest_button_scroll(dev, BTN_MIDDLE, 0, 0);
	litest_assert_button_event(li, BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(trackpoint_middlebutton_noscroll)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;

	/* Disable middle button scrolling */
	libinput_device_config_scroll_set_method(dev->libinput_device,
					LIBINPUT_CONFIG_SCROLL_NO_SCROLL);

	litest_drain_events(li);

	/* A long middle button click + motion should get reported normally now */
	litest_button_scroll(dev, BTN_MIDDLE, 0, 10);

	litest_assert_button_event(li, BTN_MIDDLE, 1);

	event = libinput_get_event(li);
	ck_assert_notnull(event);
	ck_assert_int_eq(libinput_event_get_type(event), LIBINPUT_EVENT_POINTER_MOTION);
	libinput_event_destroy(event);

	litest_assert_button_event(li, BTN_MIDDLE, 0);

	litest_assert_empty_queue(li);

	/* Restore default scroll behavior */
	libinput_device_config_scroll_set_method(dev->libinput_device,
		libinput_device_config_scroll_get_default_method(
			dev->libinput_device));
}
END_TEST

START_TEST(trackpoint_scroll_source)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;

	litest_drain_events(li);

	litest_button_scroll(dev, BTN_MIDDLE, 0, 6);
	litest_wait_for_event_of_type(li, LIBINPUT_EVENT_POINTER_AXIS, -1);

	while ((event = libinput_get_event(li))) {
		ptrev = libinput_event_get_pointer_event(event);

		ck_assert_int_eq(litest_event_pointer_get_axis_source(ptrev),
				 LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS);

		libinput_event_destroy(event);
	}
}
END_TEST

START_TEST(trackpoint_topsoftbuttons_left_handed_trackpoint)
{
	struct litest_device *touchpad = litest_current_device();
	struct litest_device *trackpoint;
	struct libinput *li = touchpad->libinput;
	enum libinput_config_status status;
	struct libinput_event *event;
	struct libinput_device *device;

	litest_disable_hold_gestures(touchpad->libinput_device);

	trackpoint = litest_add_device(li, LITEST_TRACKPOINT);
	litest_drain_events(li);
	/* touchpad right-handed, trackpoint left-handed */
	status = libinput_device_config_left_handed_set(
					trackpoint->libinput_device, 1);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_touch_down(touchpad, 0, 5, 5);
	libinput_dispatch(li);
	litest_button_click_debounced(touchpad, li, BTN_LEFT, true);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_RIGHT,
			       LIBINPUT_BUTTON_STATE_PRESSED);
	device = libinput_event_get_device(event);
	ck_assert(device == trackpoint->libinput_device);
	libinput_event_destroy(event);

	litest_button_click_debounced(touchpad, li, BTN_LEFT, false);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_RIGHT,
			       LIBINPUT_BUTTON_STATE_RELEASED);
	device = libinput_event_get_device(event);
	ck_assert(device == trackpoint->libinput_device);
	libinput_event_destroy(event);

	litest_delete_device(trackpoint);
}
END_TEST

START_TEST(trackpoint_topsoftbuttons_left_handed_touchpad)
{
	struct litest_device *touchpad = litest_current_device();
	struct litest_device *trackpoint;
	struct libinput *li = touchpad->libinput;
	enum libinput_config_status status;
	struct libinput_event *event;
	struct libinput_device *device;

	litest_disable_hold_gestures(touchpad->libinput_device);

	trackpoint = litest_add_device(li, LITEST_TRACKPOINT);
	litest_drain_events(li);
	/* touchpad left-handed, trackpoint right-handed */
	status = libinput_device_config_left_handed_set(
					touchpad->libinput_device, 1);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_touch_down(touchpad, 0, 5, 5);
	libinput_dispatch(li);
	litest_button_click_debounced(touchpad, li, BTN_LEFT, true);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_button_event(event, BTN_LEFT, LIBINPUT_BUTTON_STATE_PRESSED);
	device = libinput_event_get_device(event);
	ck_assert(device == trackpoint->libinput_device);
	libinput_event_destroy(event);

	litest_button_click_debounced(touchpad, li, BTN_LEFT, false);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_LEFT,
			       LIBINPUT_BUTTON_STATE_RELEASED);
	device = libinput_event_get_device(event);
	ck_assert(device == trackpoint->libinput_device);
	libinput_event_destroy(event);

	litest_delete_device(trackpoint);
}
END_TEST

START_TEST(trackpoint_topsoftbuttons_left_handed_both)
{
	struct litest_device *touchpad = litest_current_device();
	struct litest_device *trackpoint;
	struct libinput *li = touchpad->libinput;
	enum libinput_config_status status;
	struct libinput_event *event;
	struct libinput_device *device;

	litest_disable_hold_gestures(touchpad->libinput_device);

	trackpoint = litest_add_device(li, LITEST_TRACKPOINT);
	litest_drain_events(li);
	/* touchpad left-handed, trackpoint left-handed */
	status = libinput_device_config_left_handed_set(
					touchpad->libinput_device, 1);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	status = libinput_device_config_left_handed_set(
					trackpoint->libinput_device, 1);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_touch_down(touchpad, 0, 5, 5);
	libinput_dispatch(li);
	litest_button_click_debounced(touchpad, li, BTN_LEFT, true);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_RIGHT,
			       LIBINPUT_BUTTON_STATE_PRESSED);
	device = libinput_event_get_device(event);
	ck_assert(device == trackpoint->libinput_device);
	libinput_event_destroy(event);

	litest_button_click_debounced(touchpad, li, BTN_LEFT, false);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_RIGHT,
			       LIBINPUT_BUTTON_STATE_RELEASED);
	device = libinput_event_get_device(event);
	ck_assert(device == trackpoint->libinput_device);
	libinput_event_destroy(event);

	litest_delete_device(trackpoint);
}
END_TEST

static inline void
enable_dwtp(struct litest_device *dev)
{
	enum libinput_config_status status,
				    expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_dwtp_set_enabled(dev->libinput_device,
						LIBINPUT_CONFIG_DWTP_ENABLED);
	litest_assert_int_eq(status, expected);
}

static inline void
disable_dwtp(struct litest_device *dev)
{
	enum libinput_config_status status,
				    expected = LIBINPUT_CONFIG_STATUS_SUCCESS;
	status = libinput_device_config_dwtp_set_enabled(dev->libinput_device,
						LIBINPUT_CONFIG_DWTP_DISABLED);
	litest_assert_int_eq(status, expected);
}


START_TEST(trackpoint_palmdetect)
{
	struct litest_device *trackpoint = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = trackpoint->libinput;
	int i;

	touchpad = litest_add_device(li, LITEST_SYNAPTICS_I2C);
	if (has_disable_while_trackpointing(touchpad))
		enable_dwtp(touchpad);

	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	for (i = 0; i < 10; i++) {
		litest_event(trackpoint, EV_REL, REL_X, 1);
		litest_event(trackpoint, EV_REL, REL_Y, 1);
		litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
	}
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 30, 30);
	litest_touch_move_to(touchpad, 0, 30, 30, 80, 80, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	litest_timeout_trackpoint();
	libinput_dispatch(li);

	litest_touch_down(touchpad, 0, 30, 30);
	litest_touch_move_to(touchpad, 0, 30, 30, 80, 80, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(trackpoint_palmdetect_dwtp_disabled)
{
	struct litest_device *trackpoint = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = trackpoint->libinput;
	int i;

	touchpad = litest_add_device(li, LITEST_SYNAPTICS_I2C);
	if (has_disable_while_trackpointing(touchpad))
		disable_dwtp(touchpad);

	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	for (i = 0; i < 10; i++) {
		litest_event(trackpoint, EV_REL, REL_X, 1);
		litest_event(trackpoint, EV_REL, REL_Y, 1);
		litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
	}
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 30, 30);
	litest_touch_move_to(touchpad, 0, 30, 30, 80, 80, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(trackpoint_palmdetect_resume_touch)
{
	struct litest_device *trackpoint = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = trackpoint->libinput;
	int i;

	touchpad = litest_add_device(li, LITEST_SYNAPTICS_I2C);

	if (has_disable_while_trackpointing(touchpad))
		enable_dwtp(touchpad);

	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	for (i = 0; i < 10; i++) {
		litest_event(trackpoint, EV_REL, REL_X, 1);
		litest_event(trackpoint, EV_REL, REL_Y, 1);
		litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
	}
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 30, 30);
	litest_touch_move_to(touchpad, 0, 30, 30, 80, 80, 10);
	litest_assert_empty_queue(li);

	litest_timeout_trackpoint();
	libinput_dispatch(li);

	/* touch started after last tp event, expect resume */
	litest_touch_move_to(touchpad, 0, 80, 80, 30, 30, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(trackpoint_palmdetect_require_min_events)
{
	struct litest_device *trackpoint = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = trackpoint->libinput;

	touchpad = litest_add_device(li, LITEST_SYNAPTICS_I2C);

	if (has_disable_while_trackpointing(touchpad))
		enable_dwtp(touchpad);

	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	/* A single event does not trigger palm detection */
	litest_event(trackpoint, EV_REL, REL_X, 1);
	litest_event(trackpoint, EV_REL, REL_Y, 1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 30, 30);
	litest_touch_move_to(touchpad, 0, 30, 30, 80, 80, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(trackpoint_palmdetect_require_min_events_timeout)
{
	struct litest_device *trackpoint = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = trackpoint->libinput;

	touchpad = litest_add_device(li, LITEST_SYNAPTICS_I2C);

	if (has_disable_while_trackpointing(touchpad))
		enable_dwtp(touchpad);

	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	for (int i = 0; i < 10; i++) {
		/* A single event does not trigger palm detection */
		litest_event(trackpoint, EV_REL, REL_X, 1);
		litest_event(trackpoint, EV_REL, REL_Y, 1);
		litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
		litest_drain_events(li);

		litest_touch_down(touchpad, 0, 30, 30);
		litest_touch_move_to(touchpad, 0, 30, 30, 80, 80, 10);
		litest_touch_up(touchpad, 0);
		litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

		litest_timeout_trackpoint();
	}

	litest_delete_device(touchpad);
}
END_TEST

TEST_COLLECTION(trackpoint)
{
	litest_add(trackpoint_middlebutton, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_middlebutton_noscroll, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_scroll, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_scroll_source, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_topsoftbuttons_left_handed_trackpoint, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add(trackpoint_topsoftbuttons_left_handed_touchpad, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add(trackpoint_topsoftbuttons_left_handed_both, LITEST_TOPBUTTONPAD, LITEST_ANY);

	litest_add(trackpoint_palmdetect, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_palmdetect_dwtp_disabled, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_palmdetect_resume_touch, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_palmdetect_require_min_events, LITEST_POINTINGSTICK, LITEST_ANY);
	litest_add(trackpoint_palmdetect_require_min_events_timeout, LITEST_POINTINGSTICK, LITEST_ANY);
}
