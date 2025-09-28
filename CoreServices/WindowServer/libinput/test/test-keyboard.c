/*
 * Copyright © 2014 Jonas Ådahl <jadahl@gmail.com>
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

#include <check.h>
#include <stdio.h>

#include "libinput-util.h"
#include "litest.h"

START_TEST(keyboard_seat_key_count)
{
	struct litest_device *devices[4];
	const int num_devices = ARRAY_LENGTH(devices);
	struct libinput *libinput;
	struct libinput_event *ev;
	struct libinput_event_keyboard *kev;
	int i;
	int seat_key_count = 0;
	int expected_key_button_count = 0;
	char device_name[255];

	libinput = litest_create_context();
	for (i = 0; i < num_devices; ++i) {
		sprintf(device_name, "litest Generic keyboard (%d)", i);
		devices[i] = litest_add_device_with_overrides(libinput,
							      LITEST_KEYBOARD,
							      device_name,
							      NULL, NULL, NULL);
	}

	litest_drain_events(libinput);

	for (i = 0; i < num_devices; ++i)
		litest_keyboard_key(devices[i], KEY_A, true);

	libinput_dispatch(libinput);
	while ((ev = libinput_get_event(libinput))) {
		kev = litest_is_keyboard_event(ev,
					       KEY_A,
					       LIBINPUT_KEY_STATE_PRESSED);

		++expected_key_button_count;
		seat_key_count =
			libinput_event_keyboard_get_seat_key_count(kev);
		ck_assert_int_eq(expected_key_button_count, seat_key_count);

		libinput_event_destroy(ev);
		libinput_dispatch(libinput);
	}

	ck_assert_int_eq(seat_key_count, num_devices);

	for (i = 0; i < num_devices; ++i)
		litest_keyboard_key(devices[i], KEY_A, false);

	libinput_dispatch(libinput);
	while ((ev = libinput_get_event(libinput))) {
		kev = libinput_event_get_keyboard_event(ev);
		ck_assert_notnull(kev);
		ck_assert_int_eq(libinput_event_keyboard_get_key(kev), KEY_A);
		ck_assert_int_eq(libinput_event_keyboard_get_key_state(kev),
				 LIBINPUT_KEY_STATE_RELEASED);

		--expected_key_button_count;
		seat_key_count =
			libinput_event_keyboard_get_seat_key_count(kev);
		ck_assert_int_eq(expected_key_button_count, seat_key_count);

		libinput_event_destroy(ev);
		libinput_dispatch(libinput);
	}

	ck_assert_int_eq(seat_key_count, 0);

	for (i = 0; i < num_devices; ++i)
		litest_delete_device(devices[i]);
	litest_destroy_context(libinput);
}
END_TEST

START_TEST(keyboard_ignore_no_pressed_release)
{
	struct litest_device *dev;
	struct libinput *unused_libinput;
	struct libinput *libinput;
	struct libinput_event *event;
	struct libinput_event_keyboard *kevent;
	int events[] = {
		EV_KEY, KEY_A,
		-1, -1,
	};
	enum libinput_key_state expected_states[] = {
		LIBINPUT_KEY_STATE_PRESSED,
		LIBINPUT_KEY_STATE_RELEASED,
	};

	/* We can't send pressed -> released -> pressed events using uinput
	 * as such non-symmetric events are dropped. Work-around this by first
	 * adding the test device to the tested context after having sent an
	 * initial pressed event. */
	unused_libinput = litest_create_context();
	dev = litest_add_device_with_overrides(unused_libinput,
					       LITEST_KEYBOARD,
					       "Generic keyboard",
					       NULL, NULL, events);

	litest_keyboard_key(dev, KEY_A, true);
	litest_drain_events(unused_libinput);

	libinput = litest_create_context();
	libinput_path_add_device(libinput,
				 libevdev_uinput_get_devnode(dev->uinput));
	litest_drain_events(libinput);

	litest_keyboard_key(dev, KEY_A, false);
	litest_keyboard_key(dev, KEY_A, true);
	litest_keyboard_key(dev, KEY_A, false);

	libinput_dispatch(libinput);

	ARRAY_FOR_EACH(expected_states, state) {
		event = libinput_get_event(libinput);
		ck_assert_notnull(event);
		ck_assert_int_eq(libinput_event_get_type(event),
				 LIBINPUT_EVENT_KEYBOARD_KEY);
		kevent = libinput_event_get_keyboard_event(event);
		ck_assert_int_eq(libinput_event_keyboard_get_key(kevent),
				 KEY_A);
		ck_assert_int_eq(libinput_event_keyboard_get_key_state(kevent),
				 *state);
		libinput_event_destroy(event);
		libinput_dispatch(libinput);
	}

	litest_assert_empty_queue(libinput);
	litest_delete_device(dev);
	litest_destroy_context(libinput);
	litest_destroy_context(unused_libinput);
}
END_TEST

START_TEST(keyboard_key_auto_release)
{
	struct libinput *libinput;
	struct litest_device *dev;
	struct libinput_event *event;
	enum libinput_event_type type;
	struct libinput_event_keyboard *kevent;
	struct {
		int code;
		int released;
	} keys[] = {
		{ .code = KEY_A, },
		{ .code = KEY_S, },
		{ .code = KEY_D, },
		{ .code = KEY_G, },
		{ .code = KEY_Z, },
		{ .code = KEY_DELETE, },
		{ .code = KEY_F24, },
	};
	int events[2 * (ARRAY_LENGTH(keys) + 1)];
	unsigned i;
	int key;
	int valid_code;

	/* Enable all tested keys on the device */
	i = 0;
	while (i < 2 * ARRAY_LENGTH(keys)) {
		key = keys[i / 2].code;
		events[i++] = EV_KEY;
		events[i++] = key;
	}
	events[i++] = -1;
	events[i++] = -1;

	libinput = litest_create_context();
	dev = litest_add_device_with_overrides(libinput,
					       LITEST_KEYBOARD,
					       "Generic keyboard",
					       NULL, NULL, events);

	litest_drain_events(libinput);

	/* Send pressed events, without releasing */
	for (i = 0; i < ARRAY_LENGTH(keys); ++i) {
		key = keys[i].code;
		litest_event(dev, EV_KEY, key, 1);
		litest_event(dev, EV_SYN, SYN_REPORT, 0);

		libinput_dispatch(libinput);

		event = libinput_get_event(libinput);
		litest_is_keyboard_event(event,
					 key,
					 LIBINPUT_KEY_STATE_PRESSED);
		libinput_event_destroy(event);
	}

	litest_drain_events(libinput);

	/* "Disconnect" device */
	litest_delete_device(dev);

	/* Mark all released keys until device is removed */
	while (1) {
		event = libinput_get_event(libinput);
		ck_assert_notnull(event);
		type = libinput_event_get_type(event);

		if (type == LIBINPUT_EVENT_DEVICE_REMOVED) {
			libinput_event_destroy(event);
			break;
		}

		ck_assert_int_eq(type, LIBINPUT_EVENT_KEYBOARD_KEY);
		kevent = libinput_event_get_keyboard_event(event);
		ck_assert_int_eq(libinput_event_keyboard_get_key_state(kevent),
				 LIBINPUT_KEY_STATE_RELEASED);
		key = libinput_event_keyboard_get_key(kevent);

		valid_code = 0;
		for (i = 0; i < ARRAY_LENGTH(keys); ++i) {
			if (keys[i].code == key) {
				ck_assert_int_eq(keys[i].released, 0);
				keys[i].released = 1;
				valid_code = 1;
			}
		}
		ck_assert_int_eq(valid_code, 1);
		libinput_event_destroy(event);
	}

	/* Check that all pressed keys has been released. */
	for (i = 0; i < ARRAY_LENGTH(keys); ++i) {
		ck_assert_int_eq(keys[i].released, 1);
	}

	litest_destroy_context(libinput);
}
END_TEST

START_TEST(keyboard_has_key)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	unsigned int code;
	int evdev_has, libinput_has;

	ck_assert(libinput_device_has_capability(
					 device,
					 LIBINPUT_DEVICE_CAP_KEYBOARD));

	for (code = 0; code < KEY_CNT; code++) {
		evdev_has = libevdev_has_event_code(dev->evdev, EV_KEY, code);
		libinput_has = libinput_device_keyboard_has_key(device, code);
		ck_assert_int_eq(evdev_has, libinput_has);
	}
}
END_TEST

START_TEST(keyboard_keys_bad_device)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	unsigned int code;
	int has_key;

	if (libinput_device_has_capability(device,
					   LIBINPUT_DEVICE_CAP_KEYBOARD))
		return;

	for (code = 0; code < KEY_CNT; code++) {
		has_key = libinput_device_keyboard_has_key(device, code);
		ck_assert_int_eq(has_key, -1);
	}
}
END_TEST

START_TEST(keyboard_time_usec)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event_keyboard *kev;
	struct libinput_event *event;
	uint64_t time_usec;

	if (!libevdev_has_event_code(dev->evdev, EV_KEY, KEY_A))
		return;

	litest_drain_events(dev->libinput);

	litest_keyboard_key(dev, KEY_A, true);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	kev = litest_is_keyboard_event(event,
				       KEY_A,
				       LIBINPUT_KEY_STATE_PRESSED);

	time_usec = libinput_event_keyboard_get_time_usec(kev);
	ck_assert_int_eq(libinput_event_keyboard_get_time(kev),
			 (uint32_t) (time_usec / 1000));

	libinput_event_destroy(event);
	litest_drain_events(dev->libinput);
}
END_TEST

START_TEST(keyboard_no_buttons)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	int code;
	const char *name;

	litest_drain_events(dev->libinput);

	for (code = 0; code < KEY_MAX; code++) {
		if (!libevdev_has_event_code(dev->evdev, EV_KEY, code))
			continue;

		name = libevdev_event_code_get_name(EV_KEY, code);
		if (!name || !strneq(name, "KEY_", 4))
			continue;

		litest_keyboard_key(dev, code, true);
		litest_keyboard_key(dev, code, false);
		libinput_dispatch(li);

		event = libinput_get_event(li);
		litest_is_keyboard_event(event,
					 code,
					 LIBINPUT_KEY_STATE_PRESSED);
		libinput_event_destroy(event);
		event = libinput_get_event(li);
		litest_is_keyboard_event(event,
					 code,
					 LIBINPUT_KEY_STATE_RELEASED);
		libinput_event_destroy(event);
	}
}
END_TEST

START_TEST(keyboard_frame_order)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (!libevdev_has_event_code(dev->evdev, EV_KEY, KEY_A) ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, KEY_LEFTSHIFT))
		return;

	litest_drain_events(li);

	litest_event(dev, EV_KEY, KEY_LEFTSHIFT, 1);
	litest_event(dev, EV_KEY, KEY_A, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_assert_key_event(li,
				KEY_LEFTSHIFT,
				LIBINPUT_KEY_STATE_PRESSED);
	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_PRESSED);

	litest_event(dev, EV_KEY, KEY_LEFTSHIFT, 0);
	litest_event(dev, EV_KEY, KEY_A, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_assert_key_event(li,
				KEY_LEFTSHIFT,
				LIBINPUT_KEY_STATE_RELEASED);
	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_RELEASED);

	litest_event(dev, EV_KEY, KEY_A, 1);
	litest_event(dev, EV_KEY, KEY_LEFTSHIFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_PRESSED);
	litest_assert_key_event(li,
				KEY_LEFTSHIFT,
				LIBINPUT_KEY_STATE_PRESSED);

	litest_event(dev, EV_KEY, KEY_A, 0);
	litest_event(dev, EV_KEY, KEY_LEFTSHIFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_assert_key_event(li, KEY_A, LIBINPUT_KEY_STATE_RELEASED);
	litest_assert_key_event(li,
				KEY_LEFTSHIFT,
				LIBINPUT_KEY_STATE_RELEASED);

	libinput_dispatch(li);
}
END_TEST

START_TEST(keyboard_leds)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	/* we can't actually test the results here without physically
	 * looking at the LEDs. So all we do is trigger the code for devices
	 * with and without LEDs and check that it doesn't go boom
	 */

	libinput_device_led_update(device,
				   LIBINPUT_LED_NUM_LOCK);
	libinput_device_led_update(device,
				   LIBINPUT_LED_CAPS_LOCK);
	libinput_device_led_update(device,
				   LIBINPUT_LED_SCROLL_LOCK);

	libinput_device_led_update(device,
				   LIBINPUT_LED_NUM_LOCK|
				   LIBINPUT_LED_CAPS_LOCK);
	libinput_device_led_update(device,
				   LIBINPUT_LED_NUM_LOCK|
				   LIBINPUT_LED_CAPS_LOCK |
				   LIBINPUT_LED_SCROLL_LOCK);
	libinput_device_led_update(device, 0);
	libinput_device_led_update(device, -1);
}
END_TEST

START_TEST(keyboard_no_scroll)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_scroll_method method;
	enum libinput_config_status status;

	method = libinput_device_config_scroll_get_method(device);
	ck_assert_int_eq(method, LIBINPUT_CONFIG_SCROLL_NO_SCROLL);
	method = libinput_device_config_scroll_get_default_method(device);
	ck_assert_int_eq(method, LIBINPUT_CONFIG_SCROLL_NO_SCROLL);

	status = libinput_device_config_scroll_set_method(device,
				 LIBINPUT_CONFIG_SCROLL_2FG);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	status = libinput_device_config_scroll_set_method(device,
				 LIBINPUT_CONFIG_SCROLL_EDGE);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	status = libinput_device_config_scroll_set_method(device,
				 LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	status = libinput_device_config_scroll_set_method(device,
				 LIBINPUT_CONFIG_SCROLL_NO_SCROLL);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
}
END_TEST

TEST_COLLECTION(keyboard)
{
	litest_add_no_device(keyboard_seat_key_count);
	litest_add_no_device(keyboard_ignore_no_pressed_release);
	litest_add_no_device(keyboard_key_auto_release);
	litest_add(keyboard_has_key, LITEST_KEYS, LITEST_ANY);
	litest_add(keyboard_keys_bad_device, LITEST_ANY, LITEST_ANY);
	litest_add(keyboard_time_usec, LITEST_KEYS, LITEST_ANY);

	litest_add(keyboard_no_buttons, LITEST_KEYS, LITEST_ANY);
	litest_add(keyboard_frame_order, LITEST_KEYS, LITEST_ANY);

	litest_add(keyboard_leds, LITEST_ANY, LITEST_ANY);

	litest_add(keyboard_no_scroll, LITEST_KEYS, LITEST_WHEEL);
}
