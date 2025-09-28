/*
 * Copyright © 2017 James Ye <jye836@gmail.com>
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
#include <fcntl.h>
#include <libinput.h>

#include "libinput-util.h"
#include "litest.h"

static inline bool
switch_has_lid(struct litest_device *dev)
{
	return libinput_device_switch_has_switch(dev->libinput_device,
						 LIBINPUT_SWITCH_LID) > 0;
}

static inline bool
switch_has_tablet_mode(struct litest_device *dev)
{
	return libinput_device_switch_has_switch(dev->libinput_device,
						 LIBINPUT_SWITCH_TABLET_MODE) > 0;
}

START_TEST(switch_has_cap)
{
	struct litest_device *dev = litest_current_device();

	/* Need to check for this specific device here because the
	 * unreliable tablet mode switch removes the capability too */
	if (dev->which == LITEST_TABLET_MODE_UNRELIABLE) {
		ck_assert(!libinput_device_has_capability(dev->libinput_device,
							  LIBINPUT_DEVICE_CAP_SWITCH));
		return;
	}

	ck_assert(libinput_device_has_capability(dev->libinput_device,
						 LIBINPUT_DEVICE_CAP_SWITCH));

}
END_TEST

START_TEST(switch_has_lid_switch)
{
	struct litest_device *dev = litest_current_device();

	if (!libevdev_has_event_code(dev->evdev, EV_SW, SW_LID))
		return;

	ck_assert_int_eq(libinput_device_switch_has_switch(dev->libinput_device,
							   LIBINPUT_SWITCH_LID),
			 1);
}
END_TEST

static bool
tablet_mode_switch_is_reliable(struct litest_device *dev)
{
	bool is_unreliable = false;

       quirks_get_bool(dev->quirks,
		QUIRK_MODEL_TABLET_MODE_SWITCH_UNRELIABLE,
		&is_unreliable);

	return !is_unreliable;
}

START_TEST(switch_has_tablet_mode_switch)
{
	struct litest_device *dev = litest_current_device();
	int has_switch;

	if (!libevdev_has_event_code(dev->evdev, EV_SW, SW_TABLET_MODE))
		return;

	has_switch = libinput_device_switch_has_switch(dev->libinput_device,
						       LIBINPUT_SWITCH_TABLET_MODE);

	if (!tablet_mode_switch_is_reliable(dev))
		ck_assert_int_ne(has_switch, 1);
	else
		ck_assert_int_eq(has_switch, 1);
}
END_TEST

START_TEST(switch_toggle)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	enum libinput_switch sw = _i; /* ranged test */

	litest_drain_events(li);

	litest_grab_device(dev);
	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_ON);
	libinput_dispatch(li);

	if (libinput_device_switch_has_switch(dev->libinput_device, sw) > 0) {
		event = libinput_get_event(li);
		litest_is_switch_event(event, sw, LIBINPUT_SWITCH_STATE_ON);
		libinput_event_destroy(event);
	} else {
		litest_assert_empty_queue(li);
	}

	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_OFF);
	libinput_dispatch(li);

	if (libinput_device_switch_has_switch(dev->libinput_device, sw) > 0) {
		event = libinput_get_event(li);
		litest_is_switch_event(event, sw, LIBINPUT_SWITCH_STATE_OFF);
		libinput_event_destroy(event);
	}

	litest_assert_empty_queue(li);
	litest_ungrab_device(dev);
}
END_TEST

START_TEST(switch_toggle_double)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	enum libinput_switch sw = _i; /* ranged test */

	if (libinput_device_switch_has_switch(dev->libinput_device, sw) <= 0)
		return;

	litest_drain_events(li);

	litest_grab_device(dev);
	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_ON);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_switch_event(event, sw, LIBINPUT_SWITCH_STATE_ON);
	libinput_event_destroy(event);

	/* This will be filtered by the kernel, so this test is a bit
	 * useless */
	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_ON);
	libinput_dispatch(li);

	litest_assert_empty_queue(li);
	litest_ungrab_device(dev);
}
END_TEST

static bool
lid_switch_is_reliable(struct litest_device *dev)
{
	char *prop;
	bool is_reliable = false;

	if (quirks_get_string(dev->quirks,
			      QUIRK_ATTR_LID_SWITCH_RELIABILITY,
			      &prop)) {
		is_reliable = streq(prop, "reliable");
	}


	return is_reliable;
}

START_TEST(switch_down_on_init)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li;
	struct libinput_event *event;
	enum libinput_switch sw = _i; /* ranged test */

	if (libinput_device_switch_has_switch(dev->libinput_device, sw) <= 0)
		return;

	if (sw == LIBINPUT_SWITCH_LID && !lid_switch_is_reliable(dev))
		return;

	litest_grab_device(dev);
	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_ON);
	litest_ungrab_device(dev);

	/* need separate context to test */
	li = litest_create_context();
	libinput_path_add_device(li,
				 libevdev_uinput_get_devnode(dev->uinput));
	libinput_dispatch(li);

	litest_wait_for_event_of_type(li, LIBINPUT_EVENT_SWITCH_TOGGLE, -1);
	event = libinput_get_event(li);
	litest_is_switch_event(event, sw, LIBINPUT_SWITCH_STATE_ON);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li))) {
		ck_assert_int_ne(libinput_event_get_type(event),
				 LIBINPUT_EVENT_SWITCH_TOGGLE);
		libinput_event_destroy(event);
	}

	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_OFF);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	litest_is_switch_event(event, sw, LIBINPUT_SWITCH_STATE_OFF);
	libinput_event_destroy(event);
	litest_assert_empty_queue(li);

	litest_destroy_context(li);
}
END_TEST

START_TEST(switch_not_down_on_init)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li;
	struct libinput_event *event;
	enum libinput_switch sw = LIBINPUT_SWITCH_LID;

	if (libinput_device_switch_has_switch(dev->libinput_device, sw) <= 0)
		return;

	if (sw == LIBINPUT_SWITCH_LID && lid_switch_is_reliable(dev))
		return;

	litest_grab_device(dev);
	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_ON);
	litest_ungrab_device(dev);

	/* need separate context to test */
	li = litest_create_context();
	libinput_path_add_device(li,
				 libevdev_uinput_get_devnode(dev->uinput));
	libinput_dispatch(li);

	while ((event = libinput_get_event(li)) != NULL) {
		ck_assert_int_ne(libinput_event_get_type(event),
				 LIBINPUT_EVENT_SWITCH_TOGGLE);
		libinput_event_destroy(event);
	}

	litest_switch_action(dev, sw, LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_empty_queue(li);
	litest_destroy_context(li);
}
END_TEST

static inline struct litest_device *
switch_init_paired_touchpad(struct libinput *li)
{
	enum litest_device_type which = LITEST_SYNAPTICS_I2C;

	return litest_add_device(li, which);
}

START_TEST(switch_disable_touchpad)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	enum libinput_switch which = _i; /* ranged test */

	if (libinput_device_switch_has_switch(sw->libinput_device, which) <= 0)
		return;

	touchpad = switch_init_paired_touchpad(li);
	litest_disable_tap(touchpad->libinput_device);
	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	litest_grab_device(sw);

	/* switch is on - no events */
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	/* switch is off - motion events */
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
	litest_ungrab_device(sw);
}
END_TEST

START_TEST(switch_disable_touchpad_during_touch)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	enum libinput_switch which = _i; /* ranged test */

	if (libinput_device_switch_has_switch(sw->libinput_device, which) <= 0)
		return;

	touchpad = switch_init_paired_touchpad(li);
	litest_disable_tap(touchpad->libinput_device);
	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 5);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_grab_device(sw);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);
	litest_ungrab_device(sw);

	litest_touch_move_to(touchpad, 0, 70, 50, 50, 50, 5);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(switch_disable_touchpad_edge_scroll)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	enum libinput_switch which = _i; /* ranged test */

	if (libinput_device_switch_has_switch(sw->libinput_device, which) <= 0)
		return;

	touchpad = switch_init_paired_touchpad(li);
	litest_enable_edge_scroll(touchpad);

	litest_drain_events(li);

	litest_grab_device(sw);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);
	litest_ungrab_device(sw);

	litest_touch_down(touchpad, 0, 99, 20);
	libinput_dispatch(li);
	litest_timeout_edgescroll();
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

	litest_touch_move_to(touchpad, 0, 99, 20, 99, 80, 60);
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

	litest_touch_move_to(touchpad, 0, 99, 80, 99, 20, 60);
	litest_touch_up(touchpad, 0);
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(switch_disable_touchpad_edge_scroll_interrupt)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	struct libinput_event *event;
	enum libinput_switch which = _i; /* ranged test */

	if (libinput_device_switch_has_switch(sw->libinput_device, which) <= 0)
		return;

	touchpad = switch_init_paired_touchpad(li);
	litest_enable_edge_scroll(touchpad);

	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 99, 20);
	libinput_dispatch(li);
	litest_timeout_edgescroll();
	litest_touch_move_to(touchpad, 0, 99, 20, 99, 30, 10);
	libinput_dispatch(li);
	litest_assert_only_axis_events(li, LIBINPUT_EVENT_POINTER_SCROLL_FINGER);

	litest_grab_device(sw);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_ungrab_device(sw);
	libinput_dispatch(li);

	litest_assert_axis_end_sequence(li,
					LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
					LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
					LIBINPUT_POINTER_AXIS_SOURCE_FINGER);

	event = libinput_get_event(li);
	litest_is_switch_event(event, which, LIBINPUT_SWITCH_STATE_ON);
	libinput_event_destroy(event);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(switch_disable_touchpad_already_open)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	enum libinput_switch which = _i; /* ranged test */

	if (libinput_device_switch_has_switch(sw->libinput_device, which) <= 0)
		return;

	touchpad = switch_init_paired_touchpad(li);

	litest_disable_tap(touchpad->libinput_device);
	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	/* default: switch is off - motion events */
	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	/* disable switch - motion events */
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_empty_queue(li);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(switch_dont_resume_disabled_touchpad)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	enum libinput_switch which = _i; /* ranged test */

	if (libinput_device_switch_has_switch(sw->libinput_device, which) <= 0)
		return;

	touchpad = switch_init_paired_touchpad(li);
	litest_disable_tap(touchpad->libinput_device);
	litest_disable_hold_gestures(touchpad->libinput_device);
	libinput_device_config_send_events_set_mode(touchpad->libinput_device,
						    LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	litest_drain_events(li);

	/* switch is on - no events */
	litest_grab_device(sw);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_ungrab_device(sw);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	/* switch is off but but tp is still disabled */
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(switch_dont_resume_disabled_touchpad_external_mouse)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad, *mouse;
	struct libinput *li = sw->libinput;
	enum libinput_switch which = _i; /* ranged test */

	if (libinput_device_switch_has_switch(sw->libinput_device, which) <= 0)
		return;

	touchpad = switch_init_paired_touchpad(li);
	mouse = litest_add_device(li, LITEST_MOUSE);
	litest_disable_tap(touchpad->libinput_device);
	litest_disable_hold_gestures(touchpad->libinput_device);
	libinput_device_config_send_events_set_mode(touchpad->libinput_device,
						    LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE);
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	/* switch is on - no events */
	litest_grab_device(sw);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_ungrab_device(sw);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	/* switch is off but but tp is still disabled */
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	litest_delete_device(touchpad);
	litest_delete_device(mouse);
}
END_TEST

START_TEST(lid_open_on_key)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *keyboard;
	struct libinput *li = sw->libinput;
	struct libinput_event *event;

	if (!switch_has_lid(sw))
		return;

	keyboard = litest_add_device(li, LITEST_KEYBOARD);

	litest_grab_device(sw);
	for (int i = 0; i < 3; i++) {
		litest_switch_action(sw,
				     LIBINPUT_SWITCH_LID,
				     LIBINPUT_SWITCH_STATE_ON);
		litest_drain_events(li);

		litest_event(keyboard, EV_KEY, KEY_A, 1);
		litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
		litest_event(keyboard, EV_KEY, KEY_A, 0);
		litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
		libinput_dispatch(li);

		event = libinput_get_event(li);
		litest_is_switch_event(event,
				       LIBINPUT_SWITCH_LID,
				       LIBINPUT_SWITCH_STATE_OFF);
		libinput_event_destroy(event);

		litest_assert_only_typed_events(li, LIBINPUT_EVENT_KEYBOARD_KEY);

		litest_switch_action(sw,
				     LIBINPUT_SWITCH_LID,
				     LIBINPUT_SWITCH_STATE_OFF);
		litest_assert_empty_queue(li);
	}
	litest_ungrab_device(sw);

	litest_delete_device(keyboard);
}
END_TEST

START_TEST(lid_open_on_key_touchpad_enabled)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *keyboard, *touchpad;
	struct libinput *li = sw->libinput;

	if (!switch_has_lid(sw))
		return;

	keyboard = litest_add_device(li, LITEST_KEYBOARD);
	touchpad = litest_add_device(li, LITEST_SYNAPTICS_I2C);

	litest_disable_hold_gestures(touchpad->libinput_device);

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_LID,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_ungrab_device(sw);
	litest_drain_events(li);

	litest_event(keyboard, EV_KEY, KEY_A, 1);
	litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
	litest_event(keyboard, EV_KEY, KEY_A, 0);
	litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
	litest_drain_events(li);
	litest_timeout_dwt_long();

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 70, 10);
	litest_touch_up(touchpad, 0);
	libinput_dispatch(li);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(keyboard);
	litest_delete_device(touchpad);
}
END_TEST

START_TEST(switch_suspend_with_keyboard)
{
	struct libinput *li;
	struct litest_device *keyboard;
	struct litest_device *sw;
	enum libinput_switch which = _i; /* ranged test */

	li = litest_create_context();

	switch(which) {
	case LIBINPUT_SWITCH_LID:
		sw = litest_add_device(li, LITEST_LID_SWITCH);
		break;
	case LIBINPUT_SWITCH_TABLET_MODE:
		sw = litest_add_device(li, LITEST_THINKPAD_EXTRABUTTONS);
		break;
	default:
		abort();
	}

	libinput_dispatch(li);

	keyboard = litest_add_device(li, LITEST_KEYBOARD);
	libinput_dispatch(li);

	litest_grab_device(sw);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_OFF);
	litest_drain_events(li);
	litest_ungrab_device(sw);

	litest_delete_device(keyboard);
	litest_drain_events(li);

	litest_delete_device(sw);
	libinput_dispatch(li);

	litest_destroy_context(li);
}
END_TEST

START_TEST(switch_suspend_with_touchpad)
{
	struct libinput *li;
	struct litest_device *touchpad, *sw;
	enum libinput_switch which = _i; /* ranged test */

	li = litest_create_context();

	switch(which) {
	case LIBINPUT_SWITCH_LID:
		sw = litest_add_device(li, LITEST_LID_SWITCH);
		break;
	case LIBINPUT_SWITCH_TABLET_MODE:
		sw = litest_add_device(li, LITEST_THINKPAD_EXTRABUTTONS);
		break;
	default:
		abort();
	}

	litest_drain_events(li);

	touchpad = litest_add_device(li, LITEST_SYNAPTICS_I2C);
	litest_drain_events(li);

	litest_grab_device(sw);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);
	litest_switch_action(sw, which, LIBINPUT_SWITCH_STATE_OFF);
	litest_drain_events(li);
	litest_ungrab_device(sw);

	litest_delete_device(sw);
	litest_drain_events(li);
	litest_delete_device(touchpad);
	litest_drain_events(li);

	litest_destroy_context(li);
}
END_TEST

START_TEST(lid_update_hw_on_key)
{
	struct litest_device *sw = litest_current_device();
	struct libinput *li = sw->libinput;
	struct litest_device *keyboard;
	struct libevdev *evdev;
	struct input_event event;
	int fd;
	int rc;

	if (!switch_has_lid(sw))
		return;

	keyboard = litest_add_device(li, LITEST_KEYBOARD);

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_LID,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);
	litest_ungrab_device(sw);

	/* Separate direct libevdev context to check if the HW event goes
	 * through */
	fd = open(libevdev_uinput_get_devnode(sw->uinput), O_RDONLY|O_NONBLOCK);
	ck_assert_int_ge(fd, 0);
	ck_assert_int_eq(libevdev_new_from_fd(fd, &evdev), 0);
	ck_assert_int_eq(libevdev_get_event_value(evdev, EV_SW, SW_LID), 1);

	/* Typing on the keyboard should trigger a lid open event */
	litest_event(keyboard, EV_KEY, KEY_A, 1);
	litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
	litest_event(keyboard, EV_KEY, KEY_A, 0);
	litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
	litest_drain_events(li);

	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);
	ck_assert_int_eq(event.type, EV_SW);
	ck_assert_int_eq(event.code, SW_LID);
	ck_assert_int_eq(event.value, 0);
	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);
	ck_assert_int_eq(event.type, EV_SYN);
	ck_assert_int_eq(event.code, SYN_REPORT);
	ck_assert_int_eq(event.value, 0);
	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, -EAGAIN);

	litest_delete_device(keyboard);
	close(fd);
	libevdev_free(evdev);
}
END_TEST

START_TEST(lid_update_hw_on_key_closed_on_init)
{
	struct litest_device *sw = litest_current_device();
	struct libinput *li;
	struct litest_device *keyboard;
	struct libevdev *evdev = sw->evdev;
	struct input_event event;
	int fd;
	int rc;

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_LID,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_ungrab_device(sw);

	/* Separate direct libevdev context to check if the HW event goes
	 * through */
	fd = open(libevdev_uinput_get_devnode(sw->uinput), O_RDONLY|O_NONBLOCK);
	ck_assert_int_ge(fd, 0);
	ck_assert_int_eq(libevdev_new_from_fd(fd, &evdev), 0);
	ck_assert_int_eq(libevdev_get_event_value(evdev, EV_SW, SW_LID), 1);

	keyboard = litest_add_device(sw->libinput, LITEST_KEYBOARD);

	/* separate context for the right state on init */
	li = litest_create_context();
	libinput_path_add_device(li,
				 libevdev_uinput_get_devnode(sw->uinput));
	libinput_path_add_device(li,
				 libevdev_uinput_get_devnode(keyboard->uinput));

	/* don't expect a switch waiting for us, this is run for an
	 * unreliable device */
	while (libinput_next_event_type(li) != LIBINPUT_EVENT_NONE) {
		ck_assert_int_ne(libinput_next_event_type(li),
				 LIBINPUT_EVENT_SWITCH_TOGGLE);
		libinput_event_destroy(libinput_get_event(li));
	}

	litest_event(keyboard, EV_KEY, KEY_A, 1);
	litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
	litest_event(keyboard, EV_KEY, KEY_A, 0);
	litest_event(keyboard, EV_SYN, SYN_REPORT, 0);
	/* No switch event, we're still in vanilla (open) state */
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_KEYBOARD_KEY);

	/* Make sure kernel state has updated */
	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);
	ck_assert_int_eq(event.type, EV_SW);
	ck_assert_int_eq(event.code, SW_LID);
	ck_assert_int_eq(event.value, 0);
	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);
	ck_assert_int_eq(event.type, EV_SYN);
	ck_assert_int_eq(event.code, SYN_REPORT);
	ck_assert_int_eq(event.value, 0);
	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, -EAGAIN);

	litest_destroy_context(li);
	litest_delete_device(keyboard);
	close(fd);
	libevdev_free(evdev);
}
END_TEST

START_TEST(lid_update_hw_on_key_multiple_keyboards)
{
	struct litest_device *sw = litest_current_device();
	struct libinput *li = sw->libinput;
	struct litest_device *keyboard1, *keyboard2;
	struct libevdev *evdev = sw->evdev;
	struct input_event event;
	int fd;
	int rc;

	if (!switch_has_lid(sw))
		return;

	keyboard1 = litest_add_device(li,
				LITEST_KEYBOARD_BLADE_STEALTH_VIDEOSWITCH);
	libinput_dispatch(li);

	keyboard2 = litest_add_device(li, LITEST_KEYBOARD_BLADE_STEALTH);
	libinput_dispatch(li);

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_LID,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);
	litest_ungrab_device(sw);

	/* Separate direct libevdev context to check if the HW event goes
	 * through */
	fd = open(libevdev_uinput_get_devnode(sw->uinput), O_RDONLY|O_NONBLOCK);
	ck_assert_int_ge(fd, 0);
	ck_assert_int_eq(libevdev_new_from_fd(fd, &evdev), 0);
	ck_assert_int_eq(libevdev_get_event_value(evdev, EV_SW, SW_LID), 1);

	/* Typing on the second keyboard should trigger a lid open event */
	litest_event(keyboard2, EV_KEY, KEY_A, 1);
	litest_event(keyboard2, EV_SYN, SYN_REPORT, 0);
	litest_event(keyboard2, EV_KEY, KEY_A, 0);
	litest_event(keyboard2, EV_SYN, SYN_REPORT, 0);
	litest_drain_events(li);

	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);
	ck_assert_int_eq(event.type, EV_SW);
	ck_assert_int_eq(event.code, SW_LID);
	ck_assert_int_eq(event.value, 0);
	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);
	ck_assert_int_eq(event.type, EV_SYN);
	ck_assert_int_eq(event.code, SYN_REPORT);
	ck_assert_int_eq(event.value, 0);
	rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event);
	ck_assert_int_eq(rc, -EAGAIN);

	litest_delete_device(keyboard1);
	litest_delete_device(keyboard2);
	close(fd);
	libevdev_free(evdev);
}
END_TEST

START_TEST(lid_key_press)
{
	struct litest_device *sw = litest_current_device();
	struct libinput *li = sw->libinput;

	litest_drain_events(li);

	litest_keyboard_key(sw, KEY_VOLUMEUP, true);
	litest_keyboard_key(sw, KEY_VOLUMEUP, false);
	libinput_dispatch(li);

	/* Check that we're routing key events from a lid device too */
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_KEYBOARD_KEY);
}
END_TEST

START_TEST(tablet_mode_disable_touchpad_on_init)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;

	if (!switch_has_tablet_mode(sw))
		return;

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);

	/* touchpad comes with switch already on - no events */
	touchpad = switch_init_paired_touchpad(li);
	litest_disable_tap(touchpad->libinput_device);
	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);
	litest_ungrab_device(sw);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(tablet_mode_disable_touchpad_on_resume)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	struct libinput_event *event;
	bool have_switch_toggle = false;

	if (!switch_has_tablet_mode(sw))
		return;

	touchpad = switch_init_paired_touchpad(li);
	litest_disable_tap(touchpad->libinput_device);
	litest_disable_hold_gestures(touchpad->libinput_device);
	litest_drain_events(li);

	libinput_suspend(li);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);
	libinput_resume(li);
	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;

		type = libinput_event_get_type(event);
		switch (type) {
		case LIBINPUT_EVENT_DEVICE_ADDED:
			break;
		case LIBINPUT_EVENT_SWITCH_TOGGLE:
			litest_is_switch_event(event,
					       LIBINPUT_SWITCH_TABLET_MODE,
					       LIBINPUT_SWITCH_STATE_ON);
			have_switch_toggle = true;
			break;
		default:
			ck_abort();
		}
		libinput_event_destroy(event);
	}

	ck_assert(have_switch_toggle);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	litest_is_switch_event(event,
			       LIBINPUT_SWITCH_TABLET_MODE,
			       LIBINPUT_SWITCH_STATE_OFF);
	libinput_event_destroy(event);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(tablet_mode_enable_touchpad_on_resume)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;
	struct libinput_event *event;

	if (!switch_has_tablet_mode(sw))
		return;

	touchpad = switch_init_paired_touchpad(li);
	litest_disable_tap(touchpad->libinput_device);
	litest_drain_events(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	libinput_suspend(li);
	litest_drain_events(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);

	libinput_resume(li);
	libinput_dispatch(li);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_DEVICE_ADDED);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_drain_events_of_type(li,
				    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN,
				    LIBINPUT_EVENT_GESTURE_HOLD_END,
				    -1);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	litest_is_switch_event(event,
			       LIBINPUT_SWITCH_TABLET_MODE,
			       LIBINPUT_SWITCH_STATE_ON);
	libinput_event_destroy(event);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10);
	litest_touch_up(touchpad, 0);
	litest_assert_empty_queue(li);

	litest_delete_device(touchpad);
}
END_TEST

START_TEST(tablet_mode_disable_keyboard)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *keyboard;
	struct libinput *li = sw->libinput;

	if (!switch_has_tablet_mode(sw))
		return;

	keyboard = litest_add_device(li, LITEST_KEYBOARD);
	litest_drain_events(li);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_keyboard_key(keyboard, KEY_B, true); /* KEY_B down but not up */
	libinput_dispatch(li);

	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_PRESSED);
	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_RELEASED);
	litest_assert_key_event(li, KEY_B, LIBINPUT_KEY_STATE_PRESSED); /* KEY_B down but not up */

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);

	/* The key currently down must be released */
	litest_assert_key_event(li, KEY_B, LIBINPUT_KEY_STATE_RELEASED);
	litest_assert_switch_event(li, LIBINPUT_SWITCH_TABLET_MODE, LIBINPUT_SWITCH_STATE_ON);
	litest_assert_empty_queue(li);

	litest_keyboard_key(keyboard, KEY_B, false); /* release the kernel device */
	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_assert_empty_queue(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_switch_event(li, LIBINPUT_SWITCH_TABLET_MODE, LIBINPUT_SWITCH_STATE_OFF);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_keyboard_key(keyboard, KEY_B, true);
	litest_keyboard_key(keyboard, KEY_B, false);
	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_PRESSED);
	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_RELEASED);
	litest_assert_key_event(li, KEY_B, LIBINPUT_KEY_STATE_PRESSED);
	litest_assert_key_event(li, KEY_B, LIBINPUT_KEY_STATE_RELEASED);

	litest_delete_device(keyboard);
}
END_TEST

START_TEST(tablet_mode_disable_keyboard_on_init)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *keyboard;
	struct libinput *li = sw->libinput;

	if (!switch_has_tablet_mode(sw))
		return;

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);

	/* keyboard comes with switch already on - no events */
	keyboard = litest_add_device(li, LITEST_KEYBOARD);
	litest_drain_events(li);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_assert_empty_queue(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_KEYBOARD_KEY);

	litest_delete_device(keyboard);
}
END_TEST

START_TEST(tablet_mode_disable_keyboard_on_resume)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *keyboard;
	struct libinput *li = sw->libinput;
	struct libinput_event *event;
	bool have_switch_toggle = false;

	if (!switch_has_tablet_mode(sw))
		return;

	keyboard = litest_add_device(li, LITEST_KEYBOARD);
	litest_drain_events(li);
	libinput_suspend(li);

	/* We cannot grab this device because libinput doesn't have an open
	 * fd to this device, we need an independent grab.
	 */
	libevdev_grab(sw->evdev, LIBEVDEV_GRAB);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	libevdev_grab(sw->evdev, LIBEVDEV_UNGRAB);
	litest_drain_events(li);

	libinput_resume(li);
	libinput_dispatch(li);

	while ((event = libinput_get_event(li))) {
		enum libinput_event_type type;

		type = libinput_event_get_type(event);
		switch (type) {
		case LIBINPUT_EVENT_DEVICE_ADDED:
			break;
		case LIBINPUT_EVENT_SWITCH_TOGGLE:
			litest_is_switch_event(event,
					       LIBINPUT_SWITCH_TABLET_MODE,
					       LIBINPUT_SWITCH_STATE_ON);
			have_switch_toggle = true;
			break;
		default:
			ck_abort();
		}
		libinput_event_destroy(event);
	}

	ck_assert(have_switch_toggle);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_assert_empty_queue(li);

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);
	litest_ungrab_device(sw);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_KEYBOARD_KEY);

	litest_delete_device(keyboard);
}
END_TEST

START_TEST(tablet_mode_enable_keyboard_on_resume)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *keyboard;
	struct libinput *li = sw->libinput;

	if (!switch_has_tablet_mode(sw))
		return;

	keyboard = litest_add_device(li, LITEST_KEYBOARD);
	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);
	litest_ungrab_device(sw);
	libinput_suspend(li);
	litest_drain_events(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);

	libinput_resume(li);
	libinput_dispatch(li);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_DEVICE_ADDED);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_KEYBOARD_KEY);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_assert_empty_queue(li);

	litest_delete_device(keyboard);
}
END_TEST

START_TEST(tablet_mode_disable_trackpoint)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *trackpoint;
	struct libinput *li = sw->libinput;

	if (!switch_has_tablet_mode(sw))
		return;

	trackpoint = litest_add_device(li, LITEST_TRACKPOINT);
	litest_drain_events(li);

	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);

	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_assert_empty_queue(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);
	litest_ungrab_device(sw);

	litest_delete_device(trackpoint);
}
END_TEST

START_TEST(tablet_mode_disable_trackpoint_on_init)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *trackpoint;
	struct libinput *li = sw->libinput;

	if (!switch_has_tablet_mode(sw))
		return;

	litest_grab_device(sw);
	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_ON);
	litest_drain_events(li);

	/* trackpoint comes with switch already on - no events */
	trackpoint = litest_add_device(li, LITEST_TRACKPOINT);
	litest_drain_events(li);

	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_assert_empty_queue(li);

	litest_switch_action(sw,
			     LIBINPUT_SWITCH_TABLET_MODE,
			     LIBINPUT_SWITCH_STATE_OFF);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_event(trackpoint, EV_REL, REL_Y, -1);
	litest_event(trackpoint, EV_SYN, SYN_REPORT, 0);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);
	litest_ungrab_device(sw);

	litest_delete_device(trackpoint);
}
END_TEST

START_TEST(dock_toggle)
{
	struct litest_device *sw = litest_current_device();
	struct libinput *li = sw->libinput;

	if (!libevdev_has_event_code(sw->evdev, EV_SW, SW_DOCK))
		return;

	litest_drain_events(li);

	litest_grab_device(sw);
	litest_event(sw, EV_SW, SW_DOCK, 1);
	libinput_dispatch(li);

	litest_event(sw, EV_SW, SW_DOCK, 0);
	libinput_dispatch(li);
	litest_ungrab_device(sw);

	litest_assert_empty_queue(li);
}
END_TEST

TEST_COLLECTION(switch)
{
	struct range switches = { LIBINPUT_SWITCH_LID,
				  LIBINPUT_SWITCH_TABLET_MODE + 1};

	litest_add(switch_has_cap, LITEST_SWITCH, LITEST_ANY);
	litest_add(switch_has_lid_switch, LITEST_SWITCH, LITEST_ANY);
	litest_add(switch_has_tablet_mode_switch, LITEST_SWITCH, LITEST_ANY);
	litest_add_ranged(switch_toggle, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_toggle_double, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_down_on_init, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add(switch_not_down_on_init, LITEST_SWITCH, LITEST_ANY);
	litest_add_ranged(switch_disable_touchpad, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_disable_touchpad_during_touch, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_disable_touchpad_edge_scroll, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_disable_touchpad_edge_scroll_interrupt, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_disable_touchpad_already_open, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_dont_resume_disabled_touchpad, LITEST_SWITCH, LITEST_ANY, &switches);
	litest_add_ranged(switch_dont_resume_disabled_touchpad_external_mouse, LITEST_SWITCH, LITEST_ANY, &switches);

	litest_add_ranged_no_device(switch_suspend_with_keyboard, &switches);
	litest_add_ranged_no_device(switch_suspend_with_touchpad, &switches);

	litest_add(lid_open_on_key, LITEST_SWITCH, LITEST_ANY);
	litest_add(lid_open_on_key_touchpad_enabled, LITEST_SWITCH, LITEST_ANY);
	litest_add_for_device(lid_update_hw_on_key, LITEST_LID_SWITCH_SURFACE3);
	litest_add_for_device(lid_update_hw_on_key_closed_on_init, LITEST_LID_SWITCH_SURFACE3);
	litest_add_for_device(lid_update_hw_on_key_multiple_keyboards, LITEST_LID_SWITCH_SURFACE3);
	litest_add_for_device(lid_key_press, LITEST_GPIO_KEYS);

	litest_add(tablet_mode_disable_touchpad_on_init, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_disable_touchpad_on_resume, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_enable_touchpad_on_resume, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_disable_keyboard, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_disable_keyboard_on_init, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_disable_keyboard_on_resume, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_enable_keyboard_on_resume, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_disable_trackpoint, LITEST_SWITCH, LITEST_ANY);
	litest_add(tablet_mode_disable_trackpoint_on_init, LITEST_SWITCH, LITEST_ANY);

	litest_add(dock_toggle, LITEST_SWITCH, LITEST_ANY);
}
