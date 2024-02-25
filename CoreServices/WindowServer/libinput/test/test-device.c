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
#include <libudev.h>
#include <unistd.h>

#include "litest.h"
#include "libinput-util.h"

START_TEST(device_sendevents_config)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device;
	uint32_t modes;

	device = dev->libinput_device;

	modes = libinput_device_config_send_events_get_modes(device);
	ck_assert_int_eq(modes,
			 LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
}
END_TEST

START_TEST(device_sendevents_config_invalid)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	status = libinput_device_config_send_events_set_mode(device,
			     LIBINPUT_CONFIG_SEND_EVENTS_DISABLED | bit(4));
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
}
END_TEST

START_TEST(device_sendevents_config_touchpad)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device;
	uint32_t modes, expected;

	expected = LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;

	/* The wacom devices in the test suite are external */
	if (libevdev_get_id_vendor(dev->evdev) != VENDOR_ID_WACOM &&
	    !litest_touchpad_is_external(dev))
		expected |=
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;

	device = dev->libinput_device;

	modes = libinput_device_config_send_events_get_modes(device);
	ck_assert_int_eq(modes, expected);
}
END_TEST

START_TEST(device_sendevents_config_touchpad_superset)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device;
	enum libinput_config_status status;
	uint32_t modes;

	/* The wacom devices in the test suite are external */
	if (libevdev_get_id_vendor(dev->evdev) == VENDOR_ID_WACOM ||
	    litest_touchpad_is_external(dev))
		return;

	device = dev->libinput_device;

	modes = LIBINPUT_CONFIG_SEND_EVENTS_DISABLED |
		LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;

	status = libinput_device_config_send_events_set_mode(device,
							     modes);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* DISABLED supersedes the rest, expect the rest to be dropped */
	modes = libinput_device_config_send_events_get_mode(device);
	ck_assert_int_eq(modes, LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
}
END_TEST

START_TEST(device_sendevents_config_default)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device;
	uint32_t mode;

	device = dev->libinput_device;

	mode = libinput_device_config_send_events_get_mode(device);
	ck_assert_int_eq(mode,
			 LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);

	mode = libinput_device_config_send_events_get_default_mode(device);
	ck_assert_int_eq(mode,
			 LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
}
END_TEST

START_TEST(device_disable)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;
	struct libinput_event *event;
	struct litest_device *tmp;

	device = dev->libinput_device;

	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* no event from disabling */
	litest_assert_empty_queue(li);

	/* no event from disabled device */
	litest_event(dev, EV_REL, REL_X, 10);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_assert_empty_queue(li);

	/* create a new device so the resumed fd isn't the same as the
	   suspended one */
	tmp = litest_add_device(li, LITEST_KEYBOARD);
	ck_assert_notnull(tmp);
	litest_drain_events(li);

	/* no event from resuming */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	litest_assert_empty_queue(li);

	/* event from re-enabled device */
	litest_event(dev, EV_REL, REL_X, 10);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	event = libinput_get_event(li);
	ck_assert_notnull(event);
	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_POINTER_MOTION);
	libinput_event_destroy(event);

	litest_delete_device(tmp);
}
END_TEST

START_TEST(device_disable_tablet)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;
	struct axis_replacement axes[] = {
		{ ABS_DISTANCE, 10 },
		{ ABS_PRESSURE, 0 },
		{ -1, -1 }
	};

	device = dev->libinput_device;

	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* no event from disabling */
	litest_assert_empty_queue(li);

	litest_tablet_proximity_in(dev, 60, 60, axes);
	for (int i = 60; i < 70; i++) {
		litest_tablet_motion(dev, i, i, axes);
		libinput_dispatch(li);
	}
	litest_tablet_proximity_out(dev);

	litest_assert_empty_queue(li);

	/* no event from resuming */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_disable_touchpad)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* no event from disabling */
	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 90, 90, 10);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);

	/* no event from resuming */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_disable_touch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* no event from disabling */
	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 90, 90, 10);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);

	/* no event from resuming */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_disable_touch_during_touch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;
	struct libinput_event *event;

	device = dev->libinput_device;

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 90, 90, 10);
	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* after disabling sendevents we require a touch up */
	libinput_dispatch(li);
	event = libinput_get_event(li);
	litest_is_touch_event(event, LIBINPUT_EVENT_TOUCH_CANCEL);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	litest_is_touch_event(event, LIBINPUT_EVENT_TOUCH_FRAME);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);

	litest_touch_move_to(dev, 0, 90, 90, 50, 50, 10);
	litest_touch_up(dev, 0);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 90, 90, 10);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);

	/* no event from resuming */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_disable_events_pending)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;
	struct libinput_event *event;
	int i;

	device = dev->libinput_device;

	litest_drain_events(li);

	/* put a couple of events in the queue, enough to
	   feed the ptraccel trackers */
	for (i = 0; i < 10; i++) {
		litest_event(dev, EV_REL, REL_X, 10);
		litest_event(dev, EV_SYN, SYN_REPORT, 0);
	}
	libinput_dispatch(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* expect above events */
	litest_wait_for_event(li);
	while ((event = libinput_get_event(li)) != NULL) {
	       ck_assert_int_eq(libinput_event_get_type(event),
				LIBINPUT_EVENT_POINTER_MOTION);
	       libinput_event_destroy(event);
       }
}
END_TEST

START_TEST(device_double_disable)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_double_enable)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_reenable_syspath_changed)
{
	struct libinput *li;
	struct litest_device *litest_device;
	struct libinput_device *device1;
	enum libinput_config_status status;
	struct libinput_event *event;

	li = litest_create_context();
	litest_device = litest_add_device(li, LITEST_MOUSE);
	device1 = litest_device->libinput_device;

	libinput_device_ref(device1);
	status = libinput_device_config_send_events_set_mode(device1,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_drain_events(li);

	litest_delete_device(litest_device);
	litest_drain_events(li);

	litest_device = litest_add_device(li, LITEST_MOUSE);

	status = libinput_device_config_send_events_set_mode(device1,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* can't really check for much here, other than that if we pump
	   events through libinput, none of them should be from the first
	   device */
	litest_event(litest_device, EV_REL, REL_X, 1);
	litest_event(litest_device, EV_REL, REL_Y, 1);
	litest_event(litest_device, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	while ((event = libinput_get_event(li))) {
		ck_assert(libinput_event_get_device(event) != device1);
		libinput_event_destroy(event);
	}

	litest_delete_device(litest_device);
	libinput_device_unref(device1);
	litest_destroy_context(li);
}
END_TEST

START_TEST(device_reenable_device_removed)
{
	struct libinput *li;
	struct litest_device *litest_device;
	struct libinput_device *device;
	enum libinput_config_status status;

	li = litest_create_context();
	litest_device = litest_add_device(li, LITEST_MOUSE);
	device = litest_device->libinput_device;

	libinput_device_ref(device);
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_drain_events(li);

	litest_delete_device(litest_device);
	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	/* can't really check for much here, this really just exercises the
	   code path. */
	litest_assert_empty_queue(li);

	libinput_device_unref(device);
	litest_destroy_context(li);
}
END_TEST

START_TEST(device_disable_release_buttons)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrevent;
	enum libinput_config_status status;

	device = dev->libinput_device;

	litest_button_click_debounced(dev, li, BTN_LEFT, true);
	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_wait_for_event(li);
	event = libinput_get_event(li);

	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_POINTER_BUTTON);
	ptrevent = libinput_event_get_pointer_event(event);
	ck_assert_int_eq(libinput_event_pointer_get_button(ptrevent),
			 BTN_LEFT);
	ck_assert_int_eq(libinput_event_pointer_get_button_state(ptrevent),
			 LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_event_destroy(event);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_disable_release_keys)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	struct libinput_event *event;
	struct libinput_event_keyboard *kbdevent;
	enum libinput_config_status status;

	device = dev->libinput_device;

	litest_keyboard_key(dev, KEY_A, true);
	litest_drain_events(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_wait_for_event(li);
	event = libinput_get_event(li);

	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_KEYBOARD_KEY);
	kbdevent = libinput_event_get_keyboard_event(event);
	ck_assert_int_eq(libinput_event_keyboard_get_key(kbdevent),
			 KEY_A);
	ck_assert_int_eq(libinput_event_keyboard_get_key_state(kbdevent),
			 LIBINPUT_KEY_STATE_RELEASED);

	libinput_event_destroy(event);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_disable_release_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	libinput_device_config_tap_set_enabled(device,
					       LIBINPUT_CONFIG_TAP_ENABLED);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	/* tap happened before suspending, so we still expect the event */

	litest_timeout_tap();

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);

	/* resume, make sure we don't get anything */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

}
END_TEST

START_TEST(device_disable_release_tap_n_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	libinput_device_config_tap_set_enabled(device,
					       LIBINPUT_CONFIG_TAP_ENABLED);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	libinput_dispatch(li);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_disable_release_softbutton)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	device = dev->libinput_device;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 90);
	litest_button_click_debounced(dev, li, BTN_LEFT, true);

	/* make sure softbutton works */
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	/* disable */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);

	litest_button_click_debounced(dev, li, BTN_LEFT, false);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);

	/* resume, make sure we don't get anything */
	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

}
END_TEST

START_TEST(device_disable_topsoftbutton)
{
	struct litest_device *dev = litest_current_device();
	struct litest_device *trackpoint;
	struct libinput *li = dev->libinput;
	struct libinput_device *device;
	enum libinput_config_status status;

	struct libinput_event *event;
	struct libinput_event_pointer *ptrevent;

	device = dev->libinput_device;

	trackpoint = litest_add_device(li, LITEST_TRACKPOINT);

	status = libinput_device_config_send_events_set_mode(device,
			LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 10);
	litest_button_click_debounced(dev, li, BTN_LEFT, true);
	litest_button_click_debounced(dev, li, BTN_LEFT, false);
	litest_touch_up(dev, 0);

	litest_wait_for_event(li);
	event = libinput_get_event(li);
	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_POINTER_BUTTON);
	ck_assert_ptr_eq(libinput_event_get_device(event),
			 trackpoint->libinput_device);
	ptrevent = libinput_event_get_pointer_event(event);
	ck_assert_int_eq(libinput_event_pointer_get_button(ptrevent),
			 BTN_RIGHT);
	ck_assert_int_eq(libinput_event_pointer_get_button_state(ptrevent),
			 LIBINPUT_BUTTON_STATE_PRESSED);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	ck_assert_int_eq(libinput_event_get_type(event),
			 LIBINPUT_EVENT_POINTER_BUTTON);
	ck_assert_ptr_eq(libinput_event_get_device(event),
			 trackpoint->libinput_device);
	ptrevent = libinput_event_get_pointer_event(event);
	ck_assert_int_eq(libinput_event_pointer_get_button(ptrevent),
			 BTN_RIGHT);
	ck_assert_int_eq(libinput_event_pointer_get_button_state(ptrevent),
			 LIBINPUT_BUTTON_STATE_RELEASED);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);

	litest_delete_device(trackpoint);
}
END_TEST

START_TEST(device_ids)
{
	struct litest_device *dev = litest_current_device();
	const char *name;
	unsigned int pid, vid;

	name = libevdev_get_name(dev->evdev);
	pid = libevdev_get_id_product(dev->evdev);
	vid = libevdev_get_id_vendor(dev->evdev);

	ck_assert_str_eq(name,
			 libinput_device_get_name(dev->libinput_device));
	ck_assert_int_eq(pid,
			 libinput_device_get_id_product(dev->libinput_device));
	ck_assert_int_eq(vid,
			 libinput_device_get_id_vendor(dev->libinput_device));
}
END_TEST

START_TEST(device_get_udev_handle)
{
	struct litest_device *dev = litest_current_device();
	struct udev_device *udev_device;

	udev_device = libinput_device_get_udev_device(dev->libinput_device);
	ck_assert_notnull(udev_device);
	udev_device_unref(udev_device);
}
END_TEST

START_TEST(device_context)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_seat *seat;

	ck_assert(dev->libinput == libinput_device_get_context(dev->libinput_device));
	seat = libinput_device_get_seat(dev->libinput_device);
	ck_assert(dev->libinput == libinput_seat_get_context(seat));
}
END_TEST

START_TEST(device_user_data)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	void *userdata = &dev; /* not referenced */

	ck_assert(libinput_device_get_user_data(device) == NULL);
	libinput_device_set_user_data(device, userdata);
	ck_assert_ptr_eq(libinput_device_get_user_data(device), userdata);
	libinput_device_set_user_data(device, NULL);
	ck_assert(libinput_device_get_user_data(device) == NULL);
}
END_TEST

START_TEST(device_group_get)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device_group *group;

	int userdata = 10;

	group = libinput_device_get_device_group(dev->libinput_device);
	ck_assert_notnull(group);

	libinput_device_group_ref(group);

	libinput_device_group_set_user_data(group, &userdata);
	ck_assert_ptr_eq(&userdata,
			 libinput_device_group_get_user_data(group));

	libinput_device_group_unref(group);
}
END_TEST

START_TEST(device_group_ref)
{
	struct libinput *li = litest_create_context();
	struct litest_device *dev = litest_add_device(li,
						      LITEST_MOUSE);
	struct libinput_device *device = dev->libinput_device;
	struct libinput_device_group *group;

	group = libinput_device_get_device_group(device);
	ck_assert_notnull(group);
	libinput_device_group_ref(group);

	libinput_device_ref(device);
	litest_drain_events(li);
	litest_delete_device(dev);
	litest_drain_events(li);

	/* make sure the device is dead but the group is still around */
	ck_assert(libinput_device_unref(device) == NULL);

	libinput_device_group_ref(group);
	ck_assert_notnull(libinput_device_group_unref(group));
	ck_assert(libinput_device_group_unref(group) == NULL);

	litest_destroy_context(li);
}
END_TEST

START_TEST(device_group_leak)
{
	struct libinput *li;
	struct libinput_device *device;
	struct libevdev_uinput *uinput;
	struct libinput_device_group *group;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);

	li = litest_create_context();
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));

	group = libinput_device_get_device_group(device);
	libinput_device_group_ref(group);

	libinput_path_remove_device(device);

	libevdev_uinput_destroy(uinput);
	litest_destroy_context(li);

	/* the device group leaks, check valgrind */
}
END_TEST

START_TEST(abs_device_no_absx)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_ABS, ABS_Y,
					     -1);
	li = litest_create_context();
	litest_disable_log_handler(li);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_restore_log_handler(li);
	ck_assert(device == NULL);
	litest_destroy_context(li);

	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(abs_device_no_absy)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_ABS, ABS_X,
					     -1);
	li = litest_create_context();
	litest_disable_log_handler(li);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_restore_log_handler(li);
	ck_assert(device == NULL);
	litest_destroy_context(li);

	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(abs_mt_device_no_absy)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_ABS, ABS_X,
					     EV_ABS, ABS_Y,
					     EV_ABS, ABS_MT_SLOT,
					     EV_ABS, ABS_MT_POSITION_X,
					     -1);
	li = litest_create_context();
	litest_disable_log_handler(li);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_restore_log_handler(li);
	ck_assert(device == NULL);
	litest_destroy_context(li);

	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(abs_mt_device_no_absx)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;

	uinput = litest_create_uinput_device("test device", NULL,
					     EV_KEY, BTN_LEFT,
					     EV_KEY, BTN_RIGHT,
					     EV_ABS, ABS_X,
					     EV_ABS, ABS_Y,
					     EV_ABS, ABS_MT_SLOT,
					     EV_ABS, ABS_MT_POSITION_Y,
					     -1);
	li = litest_create_context();
	litest_disable_log_handler(li);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_restore_log_handler(li);
	ck_assert(device == NULL);
	litest_destroy_context(li);

	libevdev_uinput_destroy(uinput);
}
END_TEST

static void
assert_device_ignored(struct libinput *li, struct input_absinfo *absinfo)
{
	struct libevdev_uinput *uinput;
	struct libinput_device *device;

	uinput = litest_create_uinput_abs_device("test device", NULL,
						 absinfo,
						 EV_KEY, BTN_LEFT,
						 EV_KEY, BTN_RIGHT,
						 -1);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_assert_ptr_null(device);
	libevdev_uinput_destroy(uinput);
}

START_TEST(abs_device_no_range)
{
	struct libinput *li;
	int code = _i; /* looped test */
	/* set x/y so libinput doesn't just reject for missing axes */
	struct input_absinfo absinfo[] = {
		{ ABS_X, 0, 10, 0, 0, 0 },
		{ ABS_Y, 0, 10, 0, 0, 0 },
		{ code, 0, 0, 0, 0, 0 },
		{ -1, -1, -1, -1, -1, -1 }
	};

	li = litest_create_context();
	litest_disable_log_handler(li);

	assert_device_ignored(li, absinfo);

	litest_restore_log_handler(li);
	litest_destroy_context(li);
}
END_TEST

START_TEST(abs_mt_device_no_range)
{
	struct libinput *li;
	int code = _i; /* looped test */
	/* set x/y so libinput doesn't just reject for missing axes */
	struct input_absinfo absinfo[] = {
		{ ABS_X, 0, 10, 0, 0, 0 },
		{ ABS_Y, 0, 10, 0, 0, 0 },
		{ ABS_MT_SLOT, 0, 10, 0, 0, 0 },
		{ ABS_MT_TRACKING_ID, 0, 255, 0, 0, 0 },
		{ ABS_MT_POSITION_X, 0, 10, 0, 0, 0 },
		{ ABS_MT_POSITION_Y, 0, 10, 0, 0, 0 },
		{ code, 0, 0, 0, 0, 0 },
		{ -1, -1, -1, -1, -1, -1 }
	};

	li = litest_create_context();
	litest_disable_log_handler(li);

	if (code != ABS_MT_TOOL_TYPE &&
	    code != ABS_MT_TRACKING_ID) /* kernel overrides it */
		assert_device_ignored(li, absinfo);

	litest_restore_log_handler(li);
	litest_destroy_context(li);
}
END_TEST

START_TEST(abs_device_missing_res)
{
	struct libinput *li;
	struct input_absinfo absinfo[] = {
		{ ABS_X, 0, 10, 0, 0, 10 },
		{ ABS_Y, 0, 10, 0, 0, 0 },
		{ -1, -1, -1, -1, -1, -1 }
	};

	li = litest_create_context();
	litest_disable_log_handler(li);

	assert_device_ignored(li, absinfo);

	absinfo[0].resolution = 0;
	absinfo[1].resolution = 20;

	assert_device_ignored(li, absinfo);

	litest_restore_log_handler(li);
	litest_destroy_context(li);
}
END_TEST

START_TEST(abs_mt_device_missing_res)
{
	struct libinput *li;
	struct input_absinfo absinfo[] = {
		{ ABS_X, 0, 10, 0, 0, 10 },
		{ ABS_Y, 0, 10, 0, 0, 10 },
		{ ABS_MT_SLOT, 0, 2, 0, 0, 0 },
		{ ABS_MT_TRACKING_ID, 0, 255, 0, 0, 0 },
		{ ABS_MT_POSITION_X, 0, 10, 0, 0, 10 },
		{ ABS_MT_POSITION_Y, 0, 10, 0, 0, 0 },
		{ -1, -1, -1, -1, -1, -1 }
	};

	li = litest_create_context();
	litest_disable_log_handler(li);
	assert_device_ignored(li, absinfo);

	absinfo[4].resolution = 0;
	absinfo[5].resolution = 20;

	assert_device_ignored(li, absinfo);

	litest_restore_log_handler(li);
	litest_destroy_context(li);

}
END_TEST

START_TEST(ignore_joystick)
{
	struct libinput *li;
	struct libevdev_uinput *uinput;
	struct libinput_device *device;
	struct input_absinfo absinfo[] = {
		{ ABS_X, 0, 10, 0, 0, 10 },
		{ ABS_Y, 0, 10, 0, 0, 10 },
		{ ABS_RX, 0, 10, 0, 0, 10 },
		{ ABS_RY, 0, 10, 0, 0, 10 },
		{ ABS_THROTTLE, 0, 2, 0, 0, 0 },
		{ ABS_RUDDER, 0, 255, 0, 0, 0 },
		{ -1, -1, -1, -1, -1, -1 }
	};

	li = litest_create_context();
	litest_disable_log_handler(li);
	litest_drain_events(li);

	uinput = litest_create_uinput_abs_device("joystick test device", NULL,
						 absinfo,
						 EV_KEY, BTN_TRIGGER,
						 EV_KEY, BTN_A,
						 -1);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_assert_ptr_null(device);
	libevdev_uinput_destroy(uinput);
	litest_restore_log_handler(li);
	litest_destroy_context(li);
}
END_TEST

START_TEST(device_wheel_only)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert(libinput_device_has_capability(device,
						 LIBINPUT_DEVICE_CAP_POINTER));
}
END_TEST

START_TEST(device_accelerometer)
{
	struct libinput *li;
	struct libevdev_uinput *uinput;
	struct libinput_device *device;

	struct input_absinfo absinfo[] = {
		{ ABS_X, 0, 10, 0, 0, 10 },
		{ ABS_Y, 0, 10, 0, 0, 10 },
		{ ABS_Z, 0, 10, 0, 0, 10 },
		{ -1, -1, -1, -1, -1, -1 }
	};

	li = litest_create_context();
	litest_disable_log_handler(li);

	uinput = litest_create_uinput_abs_device("test device", NULL,
						 absinfo,
						 -1);
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_assert_ptr_null(device);
	libevdev_uinput_destroy(uinput);
	litest_restore_log_handler(li);
	litest_destroy_context(li);
}
END_TEST

START_TEST(device_udev_tag_wacom_tablet)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct udev_device *d;
	const char *prop;

	d = libinput_device_get_udev_device(device);
	prop = udev_device_get_property_value(d,
					      "ID_INPUT_TABLET");

	ck_assert_notnull(prop);
	udev_device_unref(d);
}
END_TEST

START_TEST(device_nonpointer_rel)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;
	int i;

	uinput = litest_create_uinput_device("test device",
					     NULL,
					     EV_KEY, KEY_A,
					     EV_KEY, KEY_B,
					     EV_REL, REL_X,
					     EV_REL, REL_Y,
					     -1);
	li = litest_create_context();
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	litest_disable_log_handler(li);
	for (i = 0; i < 100; i++) {
		libevdev_uinput_write_event(uinput, EV_REL, REL_X, 1);
		libevdev_uinput_write_event(uinput, EV_REL, REL_Y, -1);
		libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
	}
	litest_restore_log_handler(li);

	litest_destroy_context(li);
	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(device_touchpad_rel)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;
	const struct input_absinfo abs[] = {
		{ ABS_X, 0, 10, 0, 0, 10 },
		{ ABS_Y, 0, 10, 0, 0, 10 },
		{ ABS_MT_SLOT, 0, 2, 0, 0, 0 },
		{ ABS_MT_TRACKING_ID, 0, 255, 0, 0, 0 },
		{ ABS_MT_POSITION_X, 0, 10, 0, 0, 10 },
		{ ABS_MT_POSITION_Y, 0, 10, 0, 0, 10 },
		{ -1, -1, -1, -1, -1, -1 }
	};
	int i;

	uinput = litest_create_uinput_abs_device("test device",
						 NULL, abs,
						 EV_KEY, BTN_TOOL_FINGER,
						 EV_KEY, BTN_TOUCH,
						 EV_REL, REL_X,
						 EV_REL, REL_Y,
						 -1);
	li = litest_create_context();
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	for (i = 0; i < 100; i++) {
		libevdev_uinput_write_event(uinput, EV_REL, REL_X, 1);
		libevdev_uinput_write_event(uinput, EV_REL, REL_Y, -1);
		libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
	}

	litest_destroy_context(li);
	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(device_touch_rel)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;
	const struct input_absinfo abs[] = {
		{ ABS_X, 0, 10, 0, 0, 10 },
		{ ABS_Y, 0, 10, 0, 0, 10 },
		{ ABS_MT_SLOT, 0, 2, 0, 0, 0 },
		{ ABS_MT_TRACKING_ID, 0, 255, 0, 0, 0 },
		{ ABS_MT_POSITION_X, 0, 10, 0, 0, 10 },
		{ ABS_MT_POSITION_Y, 0, 10, 0, 0, 10 },
		{ -1, -1, -1, -1, -1, -1 }
	};
	int i;

	uinput = litest_create_uinput_abs_device("test device",
						 NULL, abs,
						 EV_KEY, BTN_TOUCH,
						 EV_REL, REL_X,
						 EV_REL, REL_Y,
						 -1);
	li = litest_create_context();
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	litest_disable_log_handler(li);
	for (i = 0; i < 100; i++) {
		libevdev_uinput_write_event(uinput, EV_REL, REL_X, 1);
		libevdev_uinput_write_event(uinput, EV_REL, REL_Y, -1);
		libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
	}
	litest_restore_log_handler(li);

	litest_destroy_context(li);
	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(device_abs_rel)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;
	const struct input_absinfo abs[] = {
		{ ABS_X, 0, 10, 0, 0, 10 },
		{ ABS_Y, 0, 10, 0, 0, 10 },
		{ -1, -1, -1, -1, -1, -1 }
	};
	int i;

	uinput = litest_create_uinput_abs_device("test device",
						 NULL, abs,
						 EV_KEY, BTN_TOUCH,
						 EV_KEY, BTN_LEFT,
						 EV_REL, REL_X,
						 EV_REL, REL_Y,
						 -1);
	li = litest_create_context();
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	ck_assert_notnull(device);

	for (i = 0; i < 100; i++) {
		libevdev_uinput_write_event(uinput, EV_REL, REL_X, 1);
		libevdev_uinput_write_event(uinput, EV_REL, REL_Y, -1);
		libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);
	}

	litest_destroy_context(li);
	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(device_quirks_no_abs_mt_y)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *pev;
	bool hi_res_event_found, low_res_event_found;
	int code, i;

	hi_res_event_found = false;
	low_res_event_found = false;

	litest_drain_events(li);

	litest_event(dev, EV_REL, REL_HWHEEL, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	/* both high and low scroll end events must be sent */
	for (i = 0; i < 2; i++) {
		event = libinput_get_event(li);
		pev = litest_is_axis_event(event,
					   LIBINPUT_EVENT_POINTER_SCROLL_WHEEL,
					   LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
					   LIBINPUT_POINTER_AXIS_SOURCE_WHEEL);

		if (litest_is_high_res_axis_event(event)) {
			litest_assert(!hi_res_event_found);
			hi_res_event_found = true;
		} else {
			litest_assert(!low_res_event_found);
			low_res_event_found = true;
		}

		libinput_event_destroy(libinput_event_pointer_get_base_event(pev));
	}

	litest_assert(low_res_event_found);
	litest_assert(hi_res_event_found);

	for (code = ABS_MISC + 1; code < ABS_MAX; code++) {
		litest_event(dev, EV_ABS, code, 1);
		litest_event(dev, EV_SYN, SYN_REPORT, 0);
		litest_assert_empty_queue(li);
	}

}
END_TEST

START_TEST(device_quirks_cyborg_rat_mode_button)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput *li = dev->libinput;

	ck_assert(!libinput_device_pointer_has_button(device, 0x118));
	ck_assert(!libinput_device_pointer_has_button(device, 0x119));
	ck_assert(!libinput_device_pointer_has_button(device, 0x11a));

	litest_drain_events(li);

	litest_event(dev, EV_KEY, 0x118, 0);
	litest_event(dev, EV_KEY, 0x119, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, 0x119, 0);
	litest_event(dev, EV_KEY, 0x11a, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, 0x11a, 0);
	litest_event(dev, EV_KEY, 0x118, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_quirks_apple_magicmouse)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	/* ensure we get no events from the touch */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 10);
	litest_touch_up(dev, 0);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(device_quirks_logitech_marble_mouse)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	ck_assert(!libinput_device_pointer_has_button(dev->libinput_device,
						      BTN_MIDDLE));
}
END_TEST

char *debug_messages[64] = { NULL };

LIBINPUT_ATTRIBUTE_PRINTF(3, 0)
static void
debug_log_handler(struct libinput *libinput,
		  enum libinput_log_priority priority,
		  const char *format,
		  va_list args)
{
	char *message;
	int n;

	if (priority != LIBINPUT_LOG_PRIORITY_DEBUG)
		return;

	n = xvasprintf(&message, format, args);
	litest_assert_int_gt(n, 0);

	ARRAY_FOR_EACH(debug_messages, dmsg) {
		if (*dmsg == NULL) {
			*dmsg = message;
			return;
		}
	}

	litest_abort_msg("Out of space for debug messages");
}

START_TEST(device_quirks)
{
	struct libinput *li;
	struct litest_device *dev;
	struct libinput_device *device;
	char **message;
	bool disable_key_f1 = false,
	     enable_btn_left = false;
#if HAVE_LIBEVDEV_DISABLE_PROPERTY
	bool disable_pointingstick = false,
	     enable_buttonpad = false,
	     enable_direct = false,
	     disable_direct = false,
	     enable_semi_mt = false,
	     disable_semi_mt = false;
#endif

	li = litest_create_context();
	libinput_log_set_priority(li, LIBINPUT_LOG_PRIORITY_DEBUG);
	libinput_log_set_handler(li, debug_log_handler);
	dev = litest_add_device(li, LITEST_KEYBOARD_QUIRKED);
	device = dev->libinput_device;

	ck_assert(libinput_device_pointer_has_button(device,
						     BTN_LEFT));
	ck_assert(libinput_device_pointer_has_button(dev->libinput_device,
						     BTN_RIGHT));
	ck_assert(!libinput_device_pointer_has_button(device,
						      BTN_MIDDLE));
	ck_assert(!libinput_device_keyboard_has_key(dev->libinput_device,
						    KEY_F1));
	ck_assert(!libinput_device_keyboard_has_key(dev->libinput_device,
						    KEY_F2));
	ck_assert(!libinput_device_keyboard_has_key(dev->libinput_device,
						    KEY_F3));

	/* Scrape the debug messages for confirmation that our quirks are
	 * triggered, the above checks cannot work non-key codes */
	message = debug_messages;
	while (*message) {
		if (strstr(*message, "disabling EV_KEY KEY_F1"))
			disable_key_f1 = true;
		if (strstr(*message, "enabling EV_KEY BTN_LEFT"))
			enable_btn_left = true;
#if HAVE_LIBEVDEV_DISABLE_PROPERTY
		if (strstr(*message, "enabling INPUT_PROP_BUTTONPAD"))
			enable_buttonpad = true;
		if (strstr(*message, "disabling INPUT_PROP_POINTING_STICK"))
			disable_pointingstick = true;
		if (strstr(*message, "enabling INPUT_PROP_DIRECT")) {
			ck_assert(!disable_direct);
			enable_direct = true;
		}
		if (strstr(*message, "disabling INPUT_PROP_DIRECT")) {
			ck_assert(enable_direct);
			disable_direct = true;
		}
		if (strstr(*message, "enabling INPUT_PROP_SEMI_MT")) {
			ck_assert(disable_semi_mt);
			enable_semi_mt = true;
		}
		if (strstr(*message, "disabling INPUT_PROP_SEMI_MT")) {
			ck_assert(!enable_semi_mt);
			disable_semi_mt = true;
		}
#endif
		free(*message);
		message++;
	}

	ck_assert(disable_key_f1);
	ck_assert(enable_btn_left);
#if HAVE_LIBEVDEV_DISABLE_PROPERTY
	ck_assert(enable_buttonpad);
	ck_assert(disable_pointingstick);
	ck_assert(enable_direct);
	ck_assert(disable_direct);
	ck_assert(enable_semi_mt);
	ck_assert(disable_semi_mt);
#endif

	litest_disable_log_handler(li);

	litest_delete_device(dev);
	litest_destroy_context(li);
}
END_TEST

START_TEST(device_capability_at_least_one)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_device_capability caps[] = {
		LIBINPUT_DEVICE_CAP_KEYBOARD,
		LIBINPUT_DEVICE_CAP_POINTER,
		LIBINPUT_DEVICE_CAP_TOUCH,
		LIBINPUT_DEVICE_CAP_TABLET_TOOL,
		LIBINPUT_DEVICE_CAP_TABLET_PAD,
		LIBINPUT_DEVICE_CAP_GESTURE,
		LIBINPUT_DEVICE_CAP_SWITCH,
	};
	int ncaps = 0;

	ARRAY_FOR_EACH(caps, cap) {
		if (libinput_device_has_capability(device, *cap))
			ncaps++;
	}
	ck_assert_int_gt(ncaps, 0);

}
END_TEST

START_TEST(device_capability_check_invalid)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert(!libinput_device_has_capability(device, -1));
	ck_assert(!libinput_device_has_capability(device, 7));
	ck_assert(!libinput_device_has_capability(device, 0xffff));

}
END_TEST

START_TEST(device_capability_nocaps_ignored)
{
	struct libevdev_uinput *uinput;
	struct libinput *li;
	struct libinput_device *device;

	/* SW_PEN_INSERTED isn't handled in libinput so the device is
	 * processed but ends up without seat capabilities and is ignored.
	 */
	uinput = litest_create_uinput_device("test device", NULL,
					     EV_SW, SW_PEN_INSERTED,
					     -1);
	li = litest_create_context();
	device = libinput_path_add_device(li,
					  libevdev_uinput_get_devnode(uinput));
	litest_assert_ptr_null(device);

	litest_destroy_context(li);
	libevdev_uinput_destroy(uinput);
}
END_TEST

START_TEST(device_has_size)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	double w, h;
	int rc;

	rc = libinput_device_get_size(device, &w, &h);
	ck_assert_int_eq(rc, 0);
	/* This matches the current set of test devices but may fail if
	 * newer ones are added */
	ck_assert_double_gt(w, 30);
	ck_assert_double_gt(h, 20);
}
END_TEST

START_TEST(device_has_no_size)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	double w = 45, h = 67;
	int rc;

	rc = libinput_device_get_size(device, &w, &h);
	ck_assert_int_eq(rc, -1);
	ck_assert_double_eq(w, 45);
	ck_assert_double_eq(h, 67);
}
END_TEST

START_TEST(device_get_output)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	const char *output_name;

	output_name = libinput_device_get_output_name(device);
	ck_assert_str_eq(output_name, "myOutput");
}
END_TEST

START_TEST(device_no_output)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	const char *output_name;

	output_name = libinput_device_get_output_name(device);
	ck_assert(output_name == NULL);
}
END_TEST

START_TEST(device_seat_phys_name)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput_seat *seat = libinput_device_get_seat(device);
	const char *seat_name;

	seat_name = libinput_seat_get_physical_name(seat);
	ck_assert(streq(seat_name, "seat0"));
}
END_TEST

START_TEST(device_button_down_remove)
{
	struct litest_device *lidev = litest_current_device();
	struct litest_device *dev;
	struct libinput *li;

	for (int code = 0; code < KEY_MAX; code++) {
		struct libinput_event *event;
		struct libinput_event_pointer *p;
		bool have_down = false,
		     have_up = false;
		const char *keyname;
		int button_down = 0, button_up = 0;

		keyname = libevdev_event_code_get_name(EV_KEY, code);
		if (!keyname ||
		    !strneq(keyname, "BTN_", 4) ||
		    strneq(keyname, "BTN_TOOL_", 9))
			continue;

		if (!libevdev_has_event_code(lidev->evdev, EV_KEY, code))
			continue;

		li = litest_create_context();
		dev = litest_add_device(li, lidev->which);
		litest_drain_events(li);

		/* Clickpads require a touch down to trigger the button
		 * press */
		if (libevdev_has_property(lidev->evdev, INPUT_PROP_BUTTONPAD)) {
			litest_touch_down(dev, 0, 20, 90);
			libinput_dispatch(li);
		}

		litest_event(dev, EV_KEY, code, 1);
		litest_event(dev, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);

		litest_delete_device(dev);
		libinput_dispatch(li);

		while ((event = libinput_get_event(li))) {
			if (libinput_event_get_type(event) !=
			    LIBINPUT_EVENT_POINTER_BUTTON) {
				libinput_event_destroy(event);
				continue;
			}

			p = libinput_event_get_pointer_event(event);
			if (libinput_event_pointer_get_button_state(p)) {
				ck_assert(button_down == 0);
				button_down = libinput_event_pointer_get_button(p);
			} else {
				ck_assert(button_up == 0);
				button_up = libinput_event_pointer_get_button(p);
				ck_assert_int_eq(button_down, button_up);
			}
			libinput_event_destroy(event);
		}

		litest_destroy_context(li);
		ck_assert_int_eq(have_down, have_up);
	}
}
END_TEST

TEST_COLLECTION(device)
{
	struct range abs_range = { 0, ABS_MISC };
	struct range abs_mt_range = { ABS_MT_SLOT + 1, ABS_CNT };

	litest_add(device_sendevents_config, LITEST_ANY, LITEST_TOUCHPAD|LITEST_TABLET);
	litest_add(device_sendevents_config_invalid, LITEST_ANY, LITEST_TABLET);
	litest_add(device_sendevents_config_touchpad, LITEST_TOUCHPAD, LITEST_TABLET);
	litest_add(device_sendevents_config_touchpad_superset, LITEST_TOUCHPAD, LITEST_TABLET);
	litest_add(device_sendevents_config_default, LITEST_ANY, LITEST_TABLET);
	litest_add(device_disable, LITEST_RELATIVE, LITEST_TABLET);
	litest_add(device_disable_tablet, LITEST_TABLET, LITEST_ANY);
	litest_add(device_disable_touchpad, LITEST_TOUCHPAD, LITEST_TABLET);
	litest_add(device_disable_touch, LITEST_TOUCH, LITEST_ANY);
	litest_add(device_disable_touch_during_touch, LITEST_TOUCH, LITEST_ANY);
	litest_add(device_disable_touch, LITEST_SINGLE_TOUCH, LITEST_TOUCHPAD);
	litest_add(device_disable_touch_during_touch, LITEST_SINGLE_TOUCH, LITEST_TOUCHPAD);
	litest_add(device_disable_events_pending, LITEST_RELATIVE, LITEST_TOUCHPAD|LITEST_TABLET);
	litest_add(device_double_disable, LITEST_ANY, LITEST_TABLET);
	litest_add(device_double_enable, LITEST_ANY, LITEST_TABLET);
	litest_add_no_device(device_reenable_syspath_changed);
	litest_add_no_device(device_reenable_device_removed);
	litest_add_for_device(device_disable_release_buttons, LITEST_MOUSE);
	litest_add_for_device(device_disable_release_keys, LITEST_KEYBOARD);
	litest_add(device_disable_release_tap, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add(device_disable_release_tap_n_drag, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add(device_disable_release_softbutton, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add(device_disable_topsoftbutton, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add(device_ids, LITEST_ANY, LITEST_ANY);
	litest_add_for_device(device_context, LITEST_SYNAPTICS_CLICKPAD_X220);
	litest_add_for_device(device_user_data, LITEST_SYNAPTICS_CLICKPAD_X220);

	litest_add(device_get_udev_handle, LITEST_ANY, LITEST_ANY);

	litest_add(device_group_get, LITEST_ANY, LITEST_ANY);
	litest_add_no_device(device_group_ref);
	litest_add_no_device(device_group_leak);

	litest_add_no_device(abs_device_no_absx);
	litest_add_no_device(abs_device_no_absy);
	litest_add_no_device(abs_mt_device_no_absx);
	litest_add_no_device(abs_mt_device_no_absy);
	litest_add_ranged_no_device(abs_device_no_range, &abs_range);
	litest_add_ranged_no_device(abs_mt_device_no_range, &abs_mt_range);
	litest_add_no_device(abs_device_missing_res);
	litest_add_no_device(abs_mt_device_missing_res);
	litest_add_no_device(ignore_joystick);

	litest_add(device_wheel_only, LITEST_WHEEL, LITEST_RELATIVE|LITEST_ABSOLUTE|LITEST_TABLET);
	litest_add_no_device(device_accelerometer);

	litest_add(device_udev_tag_wacom_tablet, LITEST_TABLET, LITEST_TOTEM);

	litest_add_no_device(device_nonpointer_rel);
	litest_add_no_device(device_touchpad_rel);
	litest_add_no_device(device_touch_rel);
	litest_add_no_device(device_abs_rel);

	litest_add_for_device(device_quirks_no_abs_mt_y, LITEST_ANKER_MOUSE_KBD);
	litest_add_for_device(device_quirks_cyborg_rat_mode_button, LITEST_CYBORG_RAT);
	litest_add_for_device(device_quirks_apple_magicmouse, LITEST_MAGICMOUSE);
	litest_add_for_device(device_quirks_logitech_marble_mouse, LITEST_LOGITECH_TRACKBALL);
	litest_add_no_device(device_quirks);

	litest_add(device_capability_at_least_one, LITEST_ANY, LITEST_ANY);
	litest_add(device_capability_check_invalid, LITEST_ANY, LITEST_ANY);
	litest_add_no_device(device_capability_nocaps_ignored);

	litest_add(device_has_size, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add(device_has_size, LITEST_TABLET, LITEST_ANY);
	litest_add(device_has_no_size, LITEST_ANY,
		   LITEST_TOUCHPAD|LITEST_TABLET|LITEST_TOUCH|LITEST_ABSOLUTE|LITEST_SINGLE_TOUCH|LITEST_TOTEM);

	litest_add_for_device(device_get_output, LITEST_CALIBRATED_TOUCHSCREEN);
	litest_add(device_no_output, LITEST_RELATIVE, LITEST_ANY);
	litest_add(device_no_output, LITEST_KEYS, LITEST_ANY);

	litest_add(device_seat_phys_name, LITEST_ANY, LITEST_ANY);

	litest_add(device_button_down_remove, LITEST_BUTTON, LITEST_ANY);
}
