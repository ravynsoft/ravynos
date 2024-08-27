/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2013 Jonas Ådahl
 * Copyright © 2013-2017 Red Hat, Inc.
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

#include "config.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "linux/input.h"
#include <unistd.h>
#include <fcntl.h>
#include <mtdev-plumbing.h>
#include <assert.h>
#include <math.h>

#include "libinput.h"
#include "evdev.h"
#include "filter.h"
#include "libinput-private.h"
#include "quirks.h"
#include "util-input-event.h"

#if HAVE_LIBWACOM
#include <libwacom/libwacom.h>
#endif

#define DEFAULT_WHEEL_CLICK_ANGLE 15
#define DEFAULT_BUTTON_SCROLL_TIMEOUT ms2us(200)

enum evdev_device_udev_tags {
	EVDEV_UDEV_TAG_INPUT		= bit(0),
	EVDEV_UDEV_TAG_KEYBOARD		= bit(1),
	EVDEV_UDEV_TAG_MOUSE		= bit(2),
	EVDEV_UDEV_TAG_TOUCHPAD		= bit(3),
	EVDEV_UDEV_TAG_TOUCHSCREEN	= bit(4),
	EVDEV_UDEV_TAG_TABLET		= bit(5),
	EVDEV_UDEV_TAG_JOYSTICK		= bit(6),
	EVDEV_UDEV_TAG_ACCELEROMETER	= bit(7),
	EVDEV_UDEV_TAG_TABLET_PAD	= bit(8),
	EVDEV_UDEV_TAG_POINTINGSTICK	= bit(9),
	EVDEV_UDEV_TAG_TRACKBALL	= bit(10),
	EVDEV_UDEV_TAG_SWITCH		= bit(11),
};

struct evdev_udev_tag_match {
	const char *name;
	enum evdev_device_udev_tags tag;
};

static const struct evdev_udev_tag_match evdev_udev_tag_matches[] = {
	{"ID_INPUT",			EVDEV_UDEV_TAG_INPUT},
	{"ID_INPUT_KEYBOARD",		EVDEV_UDEV_TAG_KEYBOARD},
	{"ID_INPUT_KEY",		EVDEV_UDEV_TAG_KEYBOARD},
	{"ID_INPUT_MOUSE",		EVDEV_UDEV_TAG_MOUSE},
	{"ID_INPUT_TOUCHPAD",		EVDEV_UDEV_TAG_TOUCHPAD},
	{"ID_INPUT_TOUCHSCREEN",	EVDEV_UDEV_TAG_TOUCHSCREEN},
	{"ID_INPUT_TABLET",		EVDEV_UDEV_TAG_TABLET},
	{"ID_INPUT_TABLET_PAD",		EVDEV_UDEV_TAG_TABLET_PAD},
	{"ID_INPUT_JOYSTICK",		EVDEV_UDEV_TAG_JOYSTICK},
	{"ID_INPUT_ACCELEROMETER",	EVDEV_UDEV_TAG_ACCELEROMETER},
	{"ID_INPUT_POINTINGSTICK",	EVDEV_UDEV_TAG_POINTINGSTICK},
	{"ID_INPUT_TRACKBALL",		EVDEV_UDEV_TAG_TRACKBALL},
	{"ID_INPUT_SWITCH",		EVDEV_UDEV_TAG_SWITCH},
};

static const unsigned int well_known_keyboard_keys[] = {
	KEY_LEFTCTRL,
	KEY_CAPSLOCK,
	KEY_NUMLOCK,
	KEY_INSERT,
	KEY_MUTE,
	KEY_CALC,
	KEY_FILE,
	KEY_MAIL,
	KEY_PLAYPAUSE,
	KEY_BRIGHTNESSDOWN,
};

static inline bool
parse_udev_flag(struct evdev_device *device,
		struct udev_device *udev_device,
		const char *property)
{
	const char *val;
	bool b;

	val = udev_device_get_property_value(udev_device, property);
	if (!val)
		return false;

	if (!parse_boolean_property(val, &b)) {
		evdev_log_error(device,
				"property %s has invalid value '%s'\n",
				property,
				val);
		return false;
	}

	return b;
}

int
evdev_update_key_down_count(struct evdev_device *device,
			    int code,
			    int pressed)
{
	int key_count = 0;
	assert(code >= 0 && code < KEY_CNT);

	if (pressed) {
		key_count = ++device->key_count[code];
	} else {
		if (device->key_count[code] > 0) {
			key_count = --device->key_count[code];
		} else {
			evdev_log_bug_libinput(device,
					       "releasing key %s with count %d\n",
					       libevdev_event_code_get_name(EV_KEY, code),
					       device->key_count[code]);
		}
	}

	if (key_count > 32) {
		evdev_log_bug_libinput(device,
				       "key count for %s reached abnormal values\n",
				       libevdev_event_code_get_name(EV_KEY, code));
	}

	return key_count;
}

enum libinput_switch_state
evdev_device_switch_get_state(struct evdev_device *device,
			      enum libinput_switch sw)
{
	struct evdev_dispatch *dispatch = device->dispatch;

	assert(dispatch->interface->get_switch_state);

	return dispatch->interface->get_switch_state(dispatch, sw);
}

void
evdev_pointer_notify_physical_button(struct evdev_device *device,
				     uint64_t time,
				     int button,
				     enum libinput_button_state state)
{
	if (evdev_middlebutton_filter_button(device,
					     time,
					     button,
					     state))
			return;

	evdev_pointer_notify_button(device,
				    time,
				    (unsigned int)button,
				    state);
}

static void
evdev_pointer_post_button(struct evdev_device *device,
			  uint64_t time,
			  unsigned int button,
			  enum libinput_button_state state)
{
	int down_count;

	down_count = evdev_update_key_down_count(device, button, state);

	if ((state == LIBINPUT_BUTTON_STATE_PRESSED && down_count == 1) ||
	    (state == LIBINPUT_BUTTON_STATE_RELEASED && down_count == 0)) {
		pointer_notify_button(&device->base, time, button, state);

		if (state == LIBINPUT_BUTTON_STATE_RELEASED) {
			if (device->left_handed.change_to_enabled)
				device->left_handed.change_to_enabled(device);

			if (device->scroll.change_scroll_method)
				device->scroll.change_scroll_method(device);
		}
	}

}

static void
evdev_button_scroll_timeout(uint64_t time, void *data)
{
	struct evdev_device *device = data;

	device->scroll.button_scroll_state = BUTTONSCROLL_READY;
}

static void
evdev_button_scroll_button(struct evdev_device *device,
			   uint64_t time, int is_press)
{
	/* Where the button lock is enabled, we wrap the buttons into
	   their own little state machine and filter out the events.
	 */
	switch (device->scroll.lock_state) {
	case BUTTONSCROLL_LOCK_DISABLED:
		break;
	case BUTTONSCROLL_LOCK_IDLE:
		assert(is_press);
		device->scroll.lock_state = BUTTONSCROLL_LOCK_FIRSTDOWN;
		evdev_log_debug(device, "scroll lock: first down\n");
		break; /* handle event */
	case BUTTONSCROLL_LOCK_FIRSTDOWN:
		assert(!is_press);
		device->scroll.lock_state = BUTTONSCROLL_LOCK_FIRSTUP;
		evdev_log_debug(device, "scroll lock: first up\n");
		return; /* filter release event */
	case BUTTONSCROLL_LOCK_FIRSTUP:
		assert(is_press);
		device->scroll.lock_state = BUTTONSCROLL_LOCK_SECONDDOWN;
		evdev_log_debug(device, "scroll lock: second down\n");
		return; /* filter press event */
	case BUTTONSCROLL_LOCK_SECONDDOWN:
		assert(!is_press);
		device->scroll.lock_state = BUTTONSCROLL_LOCK_IDLE;
		evdev_log_debug(device, "scroll lock: idle\n");
		break; /* handle event */
	}

	if (is_press) {
		if (device->scroll.button < BTN_MOUSE + 5) {
			/* For mouse buttons 1-5 (0x110 to 0x114) we apply a timeout before scrolling
			 * since the button could also be used for regular clicking. */
			enum timer_flags flags = TIMER_FLAG_NONE;

			device->scroll.button_scroll_state = BUTTONSCROLL_BUTTON_DOWN;

			/* Special case: if middle button emulation is enabled and
			 * our scroll button is the left or right button, we only
			 * get here *after* the middle button timeout has expired
			 * for that button press. The time passed is the button-down
			 * time though (which is in the past), so we have to allow
			 * for a negative timer to be set.
			 */
			if (device->middlebutton.enabled &&
				(device->scroll.button == BTN_LEFT ||
				device->scroll.button == BTN_RIGHT)) {
				flags = TIMER_FLAG_ALLOW_NEGATIVE;
			}

			libinput_timer_set_flags(&device->scroll.timer,
						time + DEFAULT_BUTTON_SCROLL_TIMEOUT,
						flags);
		} else {
			/* For extra mouse buttons numbered 6 or more (0x115+) we assume it is
			 * dedicated exclusively to scrolling, so we don't apply the timeout
			 * in order to provide immediate scrolling responsiveness. */
			device->scroll.button_scroll_state = BUTTONSCROLL_READY;
		}
		device->scroll.button_down_time = time;
		evdev_log_debug(device, "btnscroll: down\n");
	} else {
		libinput_timer_cancel(&device->scroll.timer);
		switch(device->scroll.button_scroll_state) {
		case BUTTONSCROLL_IDLE:
			evdev_log_bug_libinput(device,
				       "invalid state IDLE for button up\n");
			break;
		case BUTTONSCROLL_BUTTON_DOWN:
		case BUTTONSCROLL_READY:
			evdev_log_debug(device, "btnscroll: cancel\n");

			/* If the button is released quickly enough or
			 * without scroll events, emit the
			 * button press/release events. */
			evdev_pointer_post_button(device,
					device->scroll.button_down_time,
					device->scroll.button,
					LIBINPUT_BUTTON_STATE_PRESSED);
			evdev_pointer_post_button(device, time,
					device->scroll.button,
					LIBINPUT_BUTTON_STATE_RELEASED);
			break;
		case BUTTONSCROLL_SCROLLING:
			evdev_log_debug(device, "btnscroll: up\n");
			evdev_stop_scroll(device, time,
					  LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS);
			break;
		}

		device->scroll.button_scroll_state = BUTTONSCROLL_IDLE;
	}
}

void
evdev_pointer_notify_button(struct evdev_device *device,
			    uint64_t time,
			    unsigned int button,
			    enum libinput_button_state state)
{
	if (device->scroll.method == LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN &&
	    button == device->scroll.button) {
		evdev_button_scroll_button(device, time, state);
		return;
	}

	evdev_pointer_post_button(device, time, button, state);
}

void
evdev_device_led_update(struct evdev_device *device, enum libinput_led leds)
{
	static const struct {
		enum libinput_led libinput;
		int evdev;
	} map[] = {
		{ LIBINPUT_LED_NUM_LOCK, LED_NUML },
		{ LIBINPUT_LED_CAPS_LOCK, LED_CAPSL },
		{ LIBINPUT_LED_SCROLL_LOCK, LED_SCROLLL },
	};
	struct input_event ev[ARRAY_LENGTH(map) + 1];
	unsigned int i;

	if (!(device->seat_caps & EVDEV_DEVICE_KEYBOARD))
		return;

	memset(ev, 0, sizeof(ev));
	for (i = 0; i < ARRAY_LENGTH(map); i++) {
		ev[i].type = EV_LED;
		ev[i].code = map[i].evdev;
		ev[i].value = !!(leds & map[i].libinput);
	}
	ev[i].type = EV_SYN;
	ev[i].code = SYN_REPORT;

	i = write(device->fd, ev, sizeof ev);
	(void)i; /* no, we really don't care about the return value */
}

void
evdev_transform_absolute(struct evdev_device *device,
			 struct device_coords *point)
{
	if (!device->abs.apply_calibration)
		return;

	matrix_mult_vec(&device->abs.calibration, &point->x, &point->y);
}

void
evdev_transform_relative(struct evdev_device *device,
			 struct device_coords *point)
{
	struct matrix rel_matrix;

	if (!device->abs.apply_calibration)
		return;

	matrix_to_relative(&rel_matrix, &device->abs.calibration);
	matrix_mult_vec(&rel_matrix, &point->x, &point->y);
}

static inline double
scale_axis(const struct input_absinfo *absinfo, double val, double to_range)
{
	return (val - absinfo->minimum) * to_range / absinfo_range(absinfo);
}

double
evdev_device_transform_x(struct evdev_device *device,
			 double x,
			 uint32_t width)
{
	return scale_axis(device->abs.absinfo_x, x, width);
}

double
evdev_device_transform_y(struct evdev_device *device,
			 double y,
			 uint32_t height)
{
	return scale_axis(device->abs.absinfo_y, y, height);
}

void
evdev_notify_axis_legacy_wheel(struct evdev_device *device,
			       uint64_t time,
			       uint32_t axes,
			       const struct normalized_coords *delta_in,
			       const struct discrete_coords *discrete_in)
{
	struct normalized_coords delta = *delta_in;
	struct discrete_coords discrete = *discrete_in;

	if (device->scroll.invert_horizontal_scrolling) {
		delta.x *= -1;
		discrete.x *= -1;
	}

	if (device->scroll.natural_scrolling_enabled) {
		delta.x *= -1;
		delta.y *= -1;
		discrete.x *= -1;
		discrete.y *= -1;
	}

	pointer_notify_axis_legacy_wheel(&device->base,
					 time,
					 axes,
					 &delta,
					 &discrete);
}

void
evdev_notify_axis_wheel(struct evdev_device *device,
			uint64_t time,
			uint32_t axes,
			const struct normalized_coords *delta_in,
			const struct wheel_v120 *v120_in)
{
	struct normalized_coords delta = *delta_in;
	struct wheel_v120 v120 = *v120_in;

	if (device->scroll.invert_horizontal_scrolling) {
		delta.x *= -1;
		v120.x *= -1;
	}

	if (device->scroll.natural_scrolling_enabled) {
		delta.x *= -1;
		delta.y *= -1;
		v120.x *= -1;
		v120.y *= -1;
	}

	pointer_notify_axis_wheel(&device->base,
				  time,
				  axes,
				  &delta,
				  &v120);
}

void
evdev_notify_axis_finger(struct evdev_device *device,
			uint64_t time,
			uint32_t axes,
			const struct normalized_coords *delta_in)
{
	struct normalized_coords delta = *delta_in;

	if (device->scroll.natural_scrolling_enabled) {
		delta.x *= -1;
		delta.y *= -1;
	}

	pointer_notify_axis_finger(&device->base,
				  time,
				  axes,
				  &delta);
}

void
evdev_notify_axis_continous(struct evdev_device *device,
			    uint64_t time,
			    uint32_t axes,
			    const struct normalized_coords *delta_in)
{
	struct normalized_coords delta = *delta_in;

	if (device->scroll.natural_scrolling_enabled) {
		delta.x *= -1;
		delta.y *= -1;
	}

	pointer_notify_axis_continuous(&device->base,
				       time,
				       axes,
				       &delta);
}

static void
evdev_tag_external_mouse(struct evdev_device *device,
			 struct udev_device *udev_device)
{
	int bustype;

	bustype = libevdev_get_id_bustype(device->evdev);
	if (bustype == BUS_USB || bustype == BUS_BLUETOOTH)
		device->tags |= EVDEV_TAG_EXTERNAL_MOUSE;
}

static void
evdev_tag_trackpoint(struct evdev_device *device,
		     struct udev_device *udev_device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	char *prop;

	if (!libevdev_has_property(device->evdev,
				  INPUT_PROP_POINTING_STICK) &&
	    !parse_udev_flag(device, udev_device, "ID_INPUT_POINTINGSTICK"))
		return;

	device->tags |= EVDEV_TAG_TRACKPOINT;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (q && quirks_get_string(q, QUIRK_ATTR_TRACKPOINT_INTEGRATION, &prop)) {
		if (streq(prop, "internal")) {
			/* noop, this is the default anyway */
		} else if (streq(prop, "external")) {
			device->tags |= EVDEV_TAG_EXTERNAL_MOUSE;
			evdev_log_info(device,
				       "is an external pointing stick\n");
		} else {
			evdev_log_info(device,
				       "tagged with unknown value %s\n",
				       prop);
		}
	}

	quirks_unref(q);
}

static inline void
evdev_tag_keyboard_internal(struct evdev_device *device)
{
	device->tags |= EVDEV_TAG_INTERNAL_KEYBOARD;
	device->tags &= ~EVDEV_TAG_EXTERNAL_KEYBOARD;
}

static inline void
evdev_tag_keyboard_external(struct evdev_device *device)
{
	device->tags |= EVDEV_TAG_EXTERNAL_KEYBOARD;
	device->tags &= ~EVDEV_TAG_INTERNAL_KEYBOARD;
}

static void
evdev_tag_keyboard(struct evdev_device *device,
		   struct udev_device *udev_device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	char *prop;
	int code;

	if (!libevdev_has_event_type(device->evdev, EV_KEY))
		return;

	for (code = KEY_Q; code <= KEY_P; code++) {
		if (!libevdev_has_event_code(device->evdev,
					     EV_KEY,
					     code))
			return;
	}

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (q && quirks_get_string(q, QUIRK_ATTR_KEYBOARD_INTEGRATION, &prop)) {
		if (streq(prop, "internal")) {
			evdev_tag_keyboard_internal(device);
		} else if (streq(prop, "external")) {
			evdev_tag_keyboard_external(device);
		} else {
			evdev_log_info(device,
				       "tagged with unknown value %s\n",
				       prop);
		}
	}

	quirks_unref(q);

	device->tags |= EVDEV_TAG_KEYBOARD;
}

static void
evdev_tag_tablet_touchpad(struct evdev_device *device)
{
	device->tags |= EVDEV_TAG_TABLET_TOUCHPAD;
}

static int
evdev_calibration_has_matrix(struct libinput_device *libinput_device)
{
	struct evdev_device *device = evdev_device(libinput_device);

	return device->abs.absinfo_x && device->abs.absinfo_y;
}

static enum libinput_config_status
evdev_calibration_set_matrix(struct libinput_device *libinput_device,
			     const float matrix[6])
{
	struct evdev_device *device = evdev_device(libinput_device);

	evdev_device_calibrate(device, matrix);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static int
evdev_calibration_get_matrix(struct libinput_device *libinput_device,
			     float matrix[6])
{
	struct evdev_device *device = evdev_device(libinput_device);

	matrix_to_farray6(&device->abs.usermatrix, matrix);

	return !matrix_is_identity(&device->abs.usermatrix);
}

static int
evdev_calibration_get_default_matrix(struct libinput_device *libinput_device,
				     float matrix[6])
{
	struct evdev_device *device = evdev_device(libinput_device);

	matrix_to_farray6(&device->abs.default_calibration, matrix);

	return !matrix_is_identity(&device->abs.default_calibration);
}

static uint32_t
evdev_sendevents_get_modes(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
}

static enum libinput_config_status
evdev_sendevents_set_mode(struct libinput_device *device,
			  enum libinput_config_send_events_mode mode)
{
	struct evdev_device *evdev = evdev_device(device);
	struct evdev_dispatch *dispatch = evdev->dispatch;

	if (mode == dispatch->sendevents.current_mode)
		return LIBINPUT_CONFIG_STATUS_SUCCESS;

	switch(mode) {
	case LIBINPUT_CONFIG_SEND_EVENTS_ENABLED:
		evdev_device_resume(evdev);
		break;
	case LIBINPUT_CONFIG_SEND_EVENTS_DISABLED:
		evdev_device_suspend(evdev);
		break;
	default: /* no support for combined modes yet */
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
	}

	dispatch->sendevents.current_mode = mode;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_send_events_mode
evdev_sendevents_get_mode(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct evdev_dispatch *dispatch = evdev->dispatch;

	return dispatch->sendevents.current_mode;
}

static enum libinput_config_send_events_mode
evdev_sendevents_get_default_mode(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
}

static int
evdev_left_handed_has(struct libinput_device *device)
{
	/* This is only hooked up when we have left-handed configuration, so we
	 * can hardcode 1 here */
	return 1;
}

static enum libinput_config_status
evdev_left_handed_set(struct libinput_device *device, int left_handed)
{
	struct evdev_device *evdev = evdev_device(device);

	evdev->left_handed.want_enabled = left_handed ? true : false;

	evdev->left_handed.change_to_enabled(evdev);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static int
evdev_left_handed_get(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);

	/* return the wanted configuration, even if it hasn't taken
	 * effect yet! */
	return evdev->left_handed.want_enabled;
}

static int
evdev_left_handed_get_default(struct libinput_device *device)
{
	return 0;
}

void
evdev_init_left_handed(struct evdev_device *device,
		       void (*change_to_left_handed)(struct evdev_device *))
{
	device->left_handed.config.has = evdev_left_handed_has;
	device->left_handed.config.set = evdev_left_handed_set;
	device->left_handed.config.get = evdev_left_handed_get;
	device->left_handed.config.get_default = evdev_left_handed_get_default;
	device->base.config.left_handed = &device->left_handed.config;
	device->left_handed.enabled = false;
	device->left_handed.want_enabled = false;
	device->left_handed.change_to_enabled = change_to_left_handed;
}

static uint32_t
evdev_scroll_get_methods(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN;
}

static enum libinput_config_status
evdev_scroll_set_method(struct libinput_device *device,
			enum libinput_config_scroll_method method)
{
	struct evdev_device *evdev = evdev_device(device);

	evdev->scroll.want_method = method;
	evdev->scroll.change_scroll_method(evdev);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_scroll_method
evdev_scroll_get_method(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);

	/* return the wanted configuration, even if it hasn't taken
	 * effect yet! */
	return evdev->scroll.want_method;
}

static enum libinput_config_scroll_method
evdev_scroll_get_default_method(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);

	if (evdev->tags & EVDEV_TAG_TRACKPOINT)
		return LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN;

	/* Mice without a scroll wheel but with middle button have on-button
	 * scrolling by default */
	if (!libevdev_has_event_code(evdev->evdev, EV_REL, REL_WHEEL) &&
	    !libevdev_has_event_code(evdev->evdev, EV_REL, REL_HWHEEL) &&
	    libevdev_has_event_code(evdev->evdev, EV_KEY, BTN_MIDDLE))
		return LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN;

	return LIBINPUT_CONFIG_SCROLL_NO_SCROLL;
}

static enum libinput_config_status
evdev_scroll_set_button(struct libinput_device *device,
			uint32_t button)
{
	struct evdev_device *evdev = evdev_device(device);

	evdev->scroll.want_button = button;
	evdev->scroll.change_scroll_method(evdev);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static uint32_t
evdev_scroll_get_button(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);

	/* return the wanted configuration, even if it hasn't taken
	 * effect yet! */
	return evdev->scroll.want_button;
}

static uint32_t
evdev_scroll_get_default_button(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	unsigned int code;

	if (libevdev_has_event_code(evdev->evdev, EV_KEY, BTN_MIDDLE))
		return BTN_MIDDLE;

	for (code = BTN_SIDE; code <= BTN_TASK; code++) {
		if (libevdev_has_event_code(evdev->evdev, EV_KEY, code))
			return code;
	}

	if (libevdev_has_event_code(evdev->evdev, EV_KEY, BTN_RIGHT))
		return BTN_RIGHT;

	return 0;
}

static enum libinput_config_status
evdev_scroll_set_button_lock(struct libinput_device *device,
			     enum libinput_config_scroll_button_lock_state state)
{
	struct evdev_device *evdev = evdev_device(device);

	switch (state) {
	case LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_DISABLED:
		evdev->scroll.want_lock_enabled = false;
		break;
	case LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_ENABLED:
		evdev->scroll.want_lock_enabled = true;
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	evdev->scroll.change_scroll_method(evdev);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_scroll_button_lock_state
evdev_scroll_get_button_lock(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);

	if (evdev->scroll.lock_state == BUTTONSCROLL_LOCK_DISABLED)
		return LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_DISABLED;

	return LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_ENABLED;
}

static enum libinput_config_scroll_button_lock_state
evdev_scroll_get_default_button_lock(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SCROLL_BUTTON_LOCK_DISABLED;
}

void
evdev_set_button_scroll_lock_enabled(struct evdev_device *device,
				     bool enabled)
{
	if (enabled)
		device->scroll.lock_state = BUTTONSCROLL_LOCK_IDLE;
	else
		device->scroll.lock_state = BUTTONSCROLL_LOCK_DISABLED;
}

void
evdev_init_button_scroll(struct evdev_device *device,
			 void (*change_scroll_method)(struct evdev_device *))
{
	char timer_name[64];

	snprintf(timer_name,
		 sizeof(timer_name),
		 "%s btnscroll",
		 evdev_device_get_sysname(device));
	libinput_timer_init(&device->scroll.timer,
			    evdev_libinput_context(device),
			    timer_name,
			    evdev_button_scroll_timeout, device);
	device->scroll.config.get_methods = evdev_scroll_get_methods;
	device->scroll.config.set_method = evdev_scroll_set_method;
	device->scroll.config.get_method = evdev_scroll_get_method;
	device->scroll.config.get_default_method = evdev_scroll_get_default_method;
	device->scroll.config.set_button = evdev_scroll_set_button;
	device->scroll.config.get_button = evdev_scroll_get_button;
	device->scroll.config.get_default_button = evdev_scroll_get_default_button;
	device->scroll.config.set_button_lock = evdev_scroll_set_button_lock;
	device->scroll.config.get_button_lock = evdev_scroll_get_button_lock;
	device->scroll.config.get_default_button_lock = evdev_scroll_get_default_button_lock;
	device->base.config.scroll_method = &device->scroll.config;
	device->scroll.method = evdev_scroll_get_default_method((struct libinput_device *)device);
	device->scroll.want_method = device->scroll.method;
	device->scroll.button = evdev_scroll_get_default_button((struct libinput_device *)device);
	device->scroll.want_button = device->scroll.button;
	device->scroll.change_scroll_method = change_scroll_method;
}

void
evdev_init_calibration(struct evdev_device *device,
		       struct libinput_device_config_calibration *calibration)
{
	device->base.config.calibration = calibration;

	calibration->has_matrix = evdev_calibration_has_matrix;
	calibration->set_matrix = evdev_calibration_set_matrix;
	calibration->get_matrix = evdev_calibration_get_matrix;
	calibration->get_default_matrix = evdev_calibration_get_default_matrix;
}

void
evdev_init_sendevents(struct evdev_device *device,
		      struct evdev_dispatch *dispatch)
{
	device->base.config.sendevents = &dispatch->sendevents.config;

	dispatch->sendevents.current_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
	dispatch->sendevents.config.get_modes = evdev_sendevents_get_modes;
	dispatch->sendevents.config.set_mode = evdev_sendevents_set_mode;
	dispatch->sendevents.config.get_mode = evdev_sendevents_get_mode;
	dispatch->sendevents.config.get_default_mode = evdev_sendevents_get_default_mode;
}

static int
evdev_scroll_config_natural_has(struct libinput_device *device)
{
	return 1;
}

static enum libinput_config_status
evdev_scroll_config_natural_set(struct libinput_device *device,
				int enabled)
{
	struct evdev_device *dev = evdev_device(device);

	dev->scroll.natural_scrolling_enabled = enabled ? true : false;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static int
evdev_scroll_config_natural_get(struct libinput_device *device)
{
	struct evdev_device *dev = evdev_device(device);

	return dev->scroll.natural_scrolling_enabled ? 1 : 0;
}

static int
evdev_scroll_config_natural_get_default(struct libinput_device *device)
{
	/* Overridden in evdev-mt-touchpad.c for Apple touchpads. */
	return 0;
}

void
evdev_init_natural_scroll(struct evdev_device *device)
{
	device->scroll.config_natural.has = evdev_scroll_config_natural_has;
	device->scroll.config_natural.set_enabled = evdev_scroll_config_natural_set;
	device->scroll.config_natural.get_enabled = evdev_scroll_config_natural_get;
	device->scroll.config_natural.get_default_enabled = evdev_scroll_config_natural_get_default;
	device->scroll.natural_scrolling_enabled = false;
	device->base.config.natural_scroll = &device->scroll.config_natural;
}

int
evdev_need_mtdev(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;

	return (libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_X) &&
		libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_Y) &&
		!libevdev_has_event_code(evdev, EV_ABS, ABS_MT_SLOT));
}

/* Fake MT devices have the ABS_MT_SLOT bit set because of
   the limited ABS_* range - they aren't MT devices, they
   just have too many ABS_ axes */
bool
evdev_is_fake_mt_device(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;

	return libevdev_has_event_code(evdev, EV_ABS, ABS_MT_SLOT) &&
		libevdev_get_num_slots(evdev) == -1;
}

enum switch_reliability
evdev_read_switch_reliability_prop(struct evdev_device *device)
{
	enum switch_reliability r;
	struct quirks_context *quirks;
	struct quirks *q;
	char *prop;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (!q || !quirks_get_string(q, QUIRK_ATTR_LID_SWITCH_RELIABILITY, &prop)) {
		r = RELIABILITY_RELIABLE;
	} else if (!parse_switch_reliability_property(prop, &r)) {
		evdev_log_error(device,
				"%s: switch reliability set to unknown value '%s'\n",
				device->devname,
				prop);
		r = RELIABILITY_RELIABLE;
	} else if (r == RELIABILITY_WRITE_OPEN) {
		evdev_log_info(device, "will write switch open events\n");
	}

	quirks_unref(q);

	return r;
}

LIBINPUT_UNUSED
static inline void
evdev_print_event(struct evdev_device *device,
		  const struct input_event *e)
{
	static uint32_t offset = 0;
	static uint32_t last_time = 0;
	uint32_t time = us2ms(input_event_time(e));

	if (offset == 0) {
		offset = time;
		last_time = time - offset;
	}

	time -= offset;

	if (libevdev_event_is_code(e, EV_SYN, SYN_REPORT)) {
		evdev_log_debug(device,
			  "%u.%03u -------------- EV_SYN ------------ +%ums\n",
			  time / 1000,
			  time % 1000,
			  time - last_time);

		last_time = time;
	} else {
		evdev_log_debug(device,
			  "%u.%03u %-16s %-20s %4d\n",
			  time / 1000,
			  time % 1000,
			  libevdev_event_type_get_name(e->type),
			  libevdev_event_code_get_name(e->type, e->code),
			  e->value);
	}
}

static inline void
evdev_process_event(struct evdev_device *device, struct input_event *e)
{
	struct evdev_dispatch *dispatch = device->dispatch;
	uint64_t time = input_event_time(e);

#if 0
	evdev_print_event(device, e);
#endif

	libinput_timer_flush(evdev_libinput_context(device), time);

	dispatch->interface->process(dispatch, device, e, time);
}

static inline void
evdev_device_dispatch_one(struct evdev_device *device,
			  struct input_event *ev)
{
	if (!device->mtdev) {
		evdev_process_event(device, ev);
	} else {
		mtdev_put_event(device->mtdev, ev);
		if (libevdev_event_is_code(ev, EV_SYN, SYN_REPORT)) {
			while (!mtdev_empty(device->mtdev)) {
				struct input_event e;
				mtdev_get_event(device->mtdev, &e);
				evdev_process_event(device, &e);
			}
		}
	}
}

static int
evdev_sync_device(struct evdev_device *device)
{
	struct input_event ev;
	int rc;

	do {
		rc = libevdev_next_event(device->evdev,
					 LIBEVDEV_READ_FLAG_SYNC, &ev);
		if (rc < 0)
			break;
		evdev_device_dispatch_one(device, &ev);
	} while (rc == LIBEVDEV_READ_STATUS_SYNC);

	return (rc == -EAGAIN || rc == -EINVAL) ? 0 : rc;
}

static inline void
evdev_note_time_delay(struct evdev_device *device,
		      const struct input_event *ev)
{
	struct libinput *libinput = evdev_libinput_context(device);
	uint32_t tdelta;
	uint64_t eventtime = input_event_time(ev);

	/* if we have a current libinput_dispatch() snapshot, compare our
	 * event time with the one from the snapshot. If we have more than
	 * 10ms delay, complain about it. This catches delays in processing
	 * where there is no steady event flow and thus SYN_DROPPED may not
	 * get hit by the kernel despite us being too slow.
	 */
	if (libinput->dispatch_time == 0 ||
	    eventtime > libinput->dispatch_time)
		return;

	tdelta = us2ms(libinput->dispatch_time - eventtime);
	if (tdelta > 20) {
		evdev_log_bug_client_ratelimit(device,
					       &device->delay_warning_limit,
					       "event processing lagging behind by %dms, your system is too slow\n",
					       tdelta);
	}
}

static void
evdev_device_dispatch(void *data)
{
	struct evdev_device *device = data;
	struct libinput *libinput = evdev_libinput_context(device);
	struct input_event ev;
	int rc;
	bool once = false;

	/* If the compositor is repainting, this function is called only once
	 * per frame and we have to process all the events available on the
	 * fd, otherwise there will be input lag. */
	do {
		rc = libevdev_next_event(device->evdev,
					 LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (rc == LIBEVDEV_READ_STATUS_SYNC) {
			evdev_log_info_ratelimit(device,
						 &device->syn_drop_limit,
						 "SYN_DROPPED event - some input events have been lost.\n");

			/* send one more sync event so we handle all
			   currently pending events before we sync up
			   to the current state */
			ev.code = SYN_REPORT;
			evdev_device_dispatch_one(device, &ev);

			rc = evdev_sync_device(device);
			if (rc == 0)
				rc = LIBEVDEV_READ_STATUS_SUCCESS;
		} else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
			if (!once) {
				evdev_note_time_delay(device, &ev);
				once = true;
			}
			evdev_device_dispatch_one(device, &ev);
		} else if (rc == -ENODEV) {
			evdev_device_remove(device);
			return;
		}
	} while (rc == LIBEVDEV_READ_STATUS_SUCCESS);

	if (rc != -EAGAIN && rc != -EINTR) {
		libinput_remove_source(libinput, device->source);
		device->source = NULL;
		int dummy_fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
		if(dummy_fd >= 0) {
			dup2(dummy_fd, device->fd);
			close(dummy_fd);
		}
		device->source = NULL;
	}
}

static inline bool
evdev_init_accel(struct evdev_device *device,
		 enum libinput_config_accel_profile which)
{
	struct motion_filter *filter = NULL;

	if (which == LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM)
		filter = create_custom_accelerator_filter();
	else if (device->tags & EVDEV_TAG_TRACKPOINT) {
		if (which == LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT)
			filter = create_pointer_accelerator_filter_trackpoint_flat(device->trackpoint_multiplier);
		else
			filter = create_pointer_accelerator_filter_trackpoint(device->trackpoint_multiplier,
									      device->use_velocity_averaging);
	} else {
		if (which == LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT)
			filter = create_pointer_accelerator_filter_flat(device->dpi);
		else if (device->dpi < DEFAULT_MOUSE_DPI)
			filter = create_pointer_accelerator_filter_linear_low_dpi(device->dpi,
										  device->use_velocity_averaging);
	}

	if (!filter)
		filter = create_pointer_accelerator_filter_linear(device->dpi,
								  device->use_velocity_averaging);

	if (!filter)
		return false;

	evdev_device_init_pointer_acceleration(device, filter);

	return true;
}

static int
evdev_accel_config_available(struct libinput_device *device)
{
	/* this function is only called if we set up ptraccel, so we can
	   reply with a resounding "Yes" */
	return 1;
}

static enum libinput_config_status
evdev_accel_config_set_speed(struct libinput_device *device, double speed)
{
	struct evdev_device *dev = evdev_device(device);

	if (!filter_set_speed(dev->pointer.filter, speed))
		return LIBINPUT_CONFIG_STATUS_INVALID;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static double
evdev_accel_config_get_speed(struct libinput_device *device)
{
	struct evdev_device *dev = evdev_device(device);

	return filter_get_speed(dev->pointer.filter);
}

static double
evdev_accel_config_get_default_speed(struct libinput_device *device)
{
	return 0.0;
}

static uint32_t
evdev_accel_config_get_profiles(struct libinput_device *libinput_device)
{
	struct evdev_device *device = evdev_device(libinput_device);

	if (!device->pointer.filter)
		return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;

	return LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE |
	       LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT |
	       LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM;
}

static enum libinput_config_status
evdev_accel_config_set_profile(struct libinput_device *libinput_device,
			       enum libinput_config_accel_profile profile)
{
	struct evdev_device *device = evdev_device(libinput_device);
	struct motion_filter *filter;
	double speed;

	filter = device->pointer.filter;
	if (filter_get_type(filter) == profile)
		return LIBINPUT_CONFIG_STATUS_SUCCESS;

	speed = filter_get_speed(filter);
	device->pointer.filter = NULL;

	if (evdev_init_accel(device, profile)) {
		evdev_accel_config_set_speed(libinput_device, speed);
		filter_destroy(filter);
	} else {
		device->pointer.filter = filter;
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
	}

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_accel_profile
evdev_accel_config_get_profile(struct libinput_device *libinput_device)
{
	struct evdev_device *device = evdev_device(libinput_device);

	return filter_get_type(device->pointer.filter);
}

static enum libinput_config_accel_profile
evdev_accel_config_get_default_profile(struct libinput_device *libinput_device)
{
	struct evdev_device *device = evdev_device(libinput_device);

	if (!device->pointer.filter)
		return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;

	/* No device has a flat profile as default */
	return LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
}

static enum libinput_config_status
evdev_set_accel_config(struct libinput_device *libinput_device,
		       struct libinput_config_accel *accel_config)
{
	assert(evdev_accel_config_get_profile(libinput_device) == accel_config->profile);

	struct evdev_device *dev = evdev_device(libinput_device);

	if (!filter_set_accel_config(dev->pointer.filter, accel_config))
		return LIBINPUT_CONFIG_STATUS_INVALID;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

void
evdev_device_init_pointer_acceleration(struct evdev_device *device,
				       struct motion_filter *filter)
{
	device->pointer.filter = filter;

	if (device->base.config.accel == NULL) {
		double default_speed;

		device->pointer.config.available = evdev_accel_config_available;
		device->pointer.config.set_speed = evdev_accel_config_set_speed;
		device->pointer.config.get_speed = evdev_accel_config_get_speed;
		device->pointer.config.get_default_speed = evdev_accel_config_get_default_speed;
		device->pointer.config.get_profiles = evdev_accel_config_get_profiles;
		device->pointer.config.set_profile = evdev_accel_config_set_profile;
		device->pointer.config.get_profile = evdev_accel_config_get_profile;
		device->pointer.config.get_default_profile = evdev_accel_config_get_default_profile;
		device->pointer.config.set_accel_config = evdev_set_accel_config;
		device->base.config.accel = &device->pointer.config;

		default_speed = evdev_accel_config_get_default_speed(&device->base);
		evdev_accel_config_set_speed(&device->base, default_speed);
	}
}

static inline bool
evdev_read_wheel_click_prop(struct evdev_device *device,
			    const char *prop,
			    double *angle)
{
	int val;

	*angle = DEFAULT_WHEEL_CLICK_ANGLE;
	prop = udev_device_get_property_value(device->udev_device, prop);
	if (!prop)
		return false;

	val = parse_mouse_wheel_click_angle_property(prop);
	if (val) {
		*angle = val;
		return true;
	}

	evdev_log_error(device,
		  "mouse wheel click angle is present but invalid, "
		  "using %d degrees instead\n",
		  DEFAULT_WHEEL_CLICK_ANGLE);

	return false;
}

static inline bool
evdev_read_wheel_click_count_prop(struct evdev_device *device,
				  const char *prop,
				  double *angle)
{
	int val;

	prop = udev_device_get_property_value(device->udev_device, prop);
	if (!prop)
		return false;

	val = parse_mouse_wheel_click_angle_property(prop);
	if (val) {
		*angle = 360.0/val;
		return true;
	}

	evdev_log_error(device,
		  "mouse wheel click count is present but invalid, "
		  "using %d degrees for angle instead instead\n",
		  DEFAULT_WHEEL_CLICK_ANGLE);
	*angle = DEFAULT_WHEEL_CLICK_ANGLE;

	return false;
}

static inline struct wheel_angle
evdev_read_wheel_click_props(struct evdev_device *device)
{
	struct wheel_angle angles;
	const char *wheel_count = "MOUSE_WHEEL_CLICK_COUNT";
	const char *wheel_angle = "MOUSE_WHEEL_CLICK_ANGLE";
	const char *hwheel_count = "MOUSE_WHEEL_CLICK_COUNT_HORIZONTAL";
	const char *hwheel_angle = "MOUSE_WHEEL_CLICK_ANGLE_HORIZONTAL";

	/* CLICK_COUNT overrides CLICK_ANGLE */
	if (evdev_read_wheel_click_count_prop(device, wheel_count, &angles.y) ||
	    evdev_read_wheel_click_prop(device, wheel_angle, &angles.y)) {
		evdev_log_debug(device,
				"wheel: vert click angle: %.2f\n", angles.y);
	}
	if (evdev_read_wheel_click_count_prop(device, hwheel_count, &angles.x) ||
	    evdev_read_wheel_click_prop(device, hwheel_angle, &angles.x)) {
		evdev_log_debug(device,
				"wheel: horizontal click angle: %.2f\n", angles.y);
	} else {
		angles.x = angles.y;
	}

	return angles;
}

static inline double
evdev_get_trackpoint_multiplier(struct evdev_device *device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	double multiplier = 1.0;

	if (!(device->tags & EVDEV_TAG_TRACKPOINT))
		return 1.0;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (q) {
		quirks_get_double(q, QUIRK_ATTR_TRACKPOINT_MULTIPLIER, &multiplier);
		quirks_unref(q);
	}

	if (multiplier <= 0.0) {
		evdev_log_bug_libinput(device,
				       "trackpoint multiplier %.2f is invalid\n",
				       multiplier);
		multiplier = 1.0;
	}

	if (multiplier != 1.0)
		evdev_log_info(device,
			       "trackpoint multiplier is %.2f\n",
			       multiplier);

	return multiplier;
}

static inline bool
evdev_need_velocity_averaging(struct evdev_device *device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	bool use_velocity_averaging = false; /* default off unless we have quirk */

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (q) {
		quirks_get_bool(q,
				QUIRK_ATTR_USE_VELOCITY_AVERAGING,
				&use_velocity_averaging);
		quirks_unref(q);
	}

	if (use_velocity_averaging)
		evdev_log_info(device,
			       "velocity averaging is turned on\n");

	return use_velocity_averaging;
}

static inline int
evdev_read_dpi_prop(struct evdev_device *device)
{
	const char *mouse_dpi;
	int dpi = DEFAULT_MOUSE_DPI;

	if (device->tags & EVDEV_TAG_TRACKPOINT)
		return DEFAULT_MOUSE_DPI;

	mouse_dpi = udev_device_get_property_value(device->udev_device,
						   "MOUSE_DPI");
	if (mouse_dpi) {
		dpi = parse_mouse_dpi_property(mouse_dpi);
		if (!dpi) {
			evdev_log_error(device,
					"mouse DPI property is present but invalid, "
					"using %d DPI instead\n",
					DEFAULT_MOUSE_DPI);
			dpi = DEFAULT_MOUSE_DPI;
		}
		evdev_log_info(device,
			       "device set to %d DPI\n",
			       dpi);
	}

	return dpi;
}

static inline uint32_t
evdev_read_model_flags(struct evdev_device *device)
{
	const struct model_map {
		enum quirk quirk;
		enum evdev_device_model model;
	} model_map[] = {
#define MODEL(name) { QUIRK_MODEL_##name, EVDEV_MODEL_##name }
		MODEL(WACOM_TOUCHPAD),
		MODEL(SYNAPTICS_SERIAL_TOUCHPAD),
		MODEL(ALPS_SERIAL_TOUCHPAD),
		MODEL(LENOVO_T450_TOUCHPAD),
		MODEL(TRACKBALL),
		MODEL(APPLE_TOUCHPAD_ONEBUTTON),
		MODEL(LENOVO_SCROLLPOINT),
#undef MODEL
		{ 0, 0 },
	};
	const struct model_map *m = model_map;
	uint32_t model_flags = 0;
	uint32_t all_model_flags = 0;
	struct quirks_context *quirks;
	struct quirks *q;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);

	while (q && m->quirk) {
		bool is_set;

		/* Check for flag re-use */
		assert((all_model_flags & m->model) == 0);
		all_model_flags |= m->model;

		if (quirks_get_bool(q, m->quirk, &is_set)) {
			if (is_set) {
				evdev_log_debug(device,
						"tagged as %s\n",
						quirk_get_name(m->quirk));
				model_flags |= m->model;
			} else {
				evdev_log_debug(device,
						"untagged as %s\n",
						quirk_get_name(m->quirk));
				model_flags &= ~m->model;
			}
		}

		m++;
	}

	quirks_unref(q);

	if (parse_udev_flag(device,
			    device->udev_device,
			    "ID_INPUT_TRACKBALL")) {
		evdev_log_debug(device, "tagged as trackball\n");
		model_flags |= EVDEV_MODEL_TRACKBALL;
	}

	/**
	 * Device is 6 years old at the time of writing this and this was
	 * one of the few udev properties that wasn't reserved for private
	 * usage, so we need to keep this for backwards compat.
	 */
	if (parse_udev_flag(device,
			    device->udev_device,
			    "LIBINPUT_MODEL_LENOVO_X220_TOUCHPAD_FW81")) {
		evdev_log_debug(device, "tagged as trackball\n");
		model_flags |= EVDEV_MODEL_LENOVO_X220_TOUCHPAD_FW81;
	}

	if (parse_udev_flag(device, device->udev_device,
			    "LIBINPUT_TEST_DEVICE")) {
		evdev_log_debug(device, "is a test device\n");
		model_flags |= EVDEV_MODEL_TEST_DEVICE;
	}

	return model_flags;
}

static inline bool
evdev_read_attr_res_prop(struct evdev_device *device,
			 size_t *xres,
			 size_t *yres)
{
	struct quirks_context *quirks;
	struct quirks *q;
	struct quirk_dimensions dim;
	bool rc = false;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (!q)
		return false;

	rc = quirks_get_dimensions(q, QUIRK_ATTR_RESOLUTION_HINT, &dim);
	if (rc) {
		*xres = dim.x;
		*yres = dim.y;
	}

	quirks_unref(q);

	return rc;
}

static inline bool
evdev_read_attr_size_prop(struct evdev_device *device,
			  size_t *size_x,
			  size_t *size_y)
{
	struct quirks_context *quirks;
	struct quirks *q;
	struct quirk_dimensions dim;
	bool rc = false;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (!q)
		return false;

	rc = quirks_get_dimensions(q, QUIRK_ATTR_SIZE_HINT, &dim);
	if (rc) {
		*size_x = dim.x;
		*size_y = dim.y;
	}

	quirks_unref(q);

	return rc;
}

/* Return 1 if the device is set to the fake resolution or 0 otherwise */
static inline int
evdev_fix_abs_resolution(struct evdev_device *device,
			 unsigned int xcode,
			 unsigned int ycode)
{
	struct libevdev *evdev = device->evdev;
	const struct input_absinfo *absx, *absy;
	size_t widthmm = 0, heightmm = 0;
	size_t xres = EVDEV_FAKE_RESOLUTION,
	       yres = EVDEV_FAKE_RESOLUTION;

	if (!(xcode == ABS_X && ycode == ABS_Y)  &&
	    !(xcode == ABS_MT_POSITION_X && ycode == ABS_MT_POSITION_Y)) {
		evdev_log_bug_libinput(device,
				       "invalid x/y code combination %d/%d\n",
				       xcode,
				       ycode);
		return 0;
	}

	absx = libevdev_get_abs_info(evdev, xcode);
	absy = libevdev_get_abs_info(evdev, ycode);

	if (absx->resolution != 0 || absy->resolution != 0)
		return 0;

	/* Note: we *do not* override resolutions if provided by the kernel.
	 * If a device needs this, add it to 60-evdev.hwdb. The libinput
	 * property is only for general size hints where we can make
	 * educated guesses but don't know better.
	 */
	if (!evdev_read_attr_res_prop(device, &xres, &yres) &&
	    evdev_read_attr_size_prop(device, &widthmm, &heightmm)) {
		xres = absinfo_range(absx)/widthmm;
		yres = absinfo_range(absy)/heightmm;
	}

	/* libevdev_set_abs_resolution() changes the absinfo we already
	   have a pointer to, no need to fetch it again */
	libevdev_set_abs_resolution(evdev, xcode, xres);
	libevdev_set_abs_resolution(evdev, ycode, yres);

	return xres == EVDEV_FAKE_RESOLUTION;
}

static enum evdev_device_udev_tags
evdev_device_get_udev_tags(struct evdev_device *device,
			   struct udev_device *udev_device)
{
	enum evdev_device_udev_tags tags = 0;
	int i;

	for (i = 0; i < 2 && udev_device; i++) {
		unsigned j;
		for (j = 0; j < ARRAY_LENGTH(evdev_udev_tag_matches); j++) {
			const struct evdev_udev_tag_match match = evdev_udev_tag_matches[j];
			if (parse_udev_flag(device,
					    udev_device,
					    match.name))
				tags |= match.tag;
		}
		udev_device = udev_device_get_parent(udev_device);
	}

	return tags;
}

static inline void
evdev_fix_android_mt(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;

	if (libevdev_has_event_code(evdev, EV_ABS, ABS_X) ||
	    libevdev_has_event_code(evdev, EV_ABS, ABS_Y))
		return;

	if (!libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_X) ||
	    !libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_Y) ||
	    evdev_is_fake_mt_device(device))
		return;

	libevdev_enable_event_code(evdev, EV_ABS, ABS_X,
		      libevdev_get_abs_info(evdev, ABS_MT_POSITION_X));
	libevdev_enable_event_code(evdev, EV_ABS, ABS_Y,
		      libevdev_get_abs_info(evdev, ABS_MT_POSITION_Y));
}

static inline bool
evdev_check_min_max(struct evdev_device *device, unsigned int code)
{
	struct libevdev *evdev = device->evdev;
	const struct input_absinfo *absinfo;

	if (!libevdev_has_event_code(evdev, EV_ABS, code))
		return true;

	absinfo = libevdev_get_abs_info(evdev, code);
	if (absinfo->minimum == absinfo->maximum) {
		/* Some devices have a sort-of legitimate min/max of 0 for
		 * ABS_MISC and above (e.g. Roccat Kone XTD). Don't ignore
		 * them, simply disable the axes so we won't get events,
		 * we don't know what to do with them anyway.
		 */
		if (absinfo->minimum == 0 &&
		    code >= ABS_MISC && code < ABS_MT_SLOT) {
			evdev_log_info(device,
				       "disabling EV_ABS %#x on device (min == max == 0)\n",
				       code);
			libevdev_disable_event_code(device->evdev,
						    EV_ABS,
						    code);
		} else {
			evdev_log_bug_kernel(device,
					     "device has min == max on %s\n",
					     libevdev_event_code_get_name(EV_ABS, code));
			return false;
		}
	}

	return true;
}

static bool
evdev_reject_device(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;
	unsigned int code;
	const struct input_absinfo *absx, *absy;

	if (libevdev_has_event_code(evdev, EV_ABS, ABS_X) ^
	    libevdev_has_event_code(evdev, EV_ABS, ABS_Y))
		return true;

	if (libevdev_has_event_code(evdev, EV_REL, REL_X) ^
	    libevdev_has_event_code(evdev, EV_REL, REL_Y))
		return true;

	if (!evdev_is_fake_mt_device(device) &&
	    libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_X) ^
	    libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_Y))
		return true;

	if (libevdev_has_event_code(evdev, EV_ABS, ABS_X)) {
		absx = libevdev_get_abs_info(evdev, ABS_X);
		absy = libevdev_get_abs_info(evdev, ABS_Y);
		if ((absx->resolution == 0 && absy->resolution != 0) ||
		    (absx->resolution != 0 && absy->resolution == 0)) {
			evdev_log_bug_kernel(device,
				       "kernel has only x or y resolution, not both.\n");
			return true;
		}
	}

	if (!evdev_is_fake_mt_device(device) &&
	    libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_X)) {
		absx = libevdev_get_abs_info(evdev, ABS_MT_POSITION_X);
		absy = libevdev_get_abs_info(evdev, ABS_MT_POSITION_Y);
		if ((absx->resolution == 0 && absy->resolution != 0) ||
		    (absx->resolution != 0 && absy->resolution == 0)) {
			evdev_log_bug_kernel(device,
				       "kernel has only x or y MT resolution, not both.\n");
			return true;
		}
	}

	for (code = 0; code < ABS_CNT; code++) {
		switch (code) {
		case ABS_MISC:
		case ABS_MT_SLOT:
		case ABS_MT_TOOL_TYPE:
			break;
		default:
			if (!evdev_check_min_max(device, code))
				return true;
		}
	}

	return false;
}

static void
evdev_extract_abs_axes(struct evdev_device *device,
		       enum evdev_device_udev_tags udev_tags)
{
	struct libevdev *evdev = device->evdev;
	int fuzz;

	if (!libevdev_has_event_code(evdev, EV_ABS, ABS_X) ||
	    !libevdev_has_event_code(evdev, EV_ABS, ABS_Y))
		 return;

	if (evdev_fix_abs_resolution(device, ABS_X, ABS_Y))
		device->abs.is_fake_resolution = true;

	if (udev_tags & (EVDEV_UDEV_TAG_TOUCHPAD|EVDEV_UDEV_TAG_TOUCHSCREEN)) {
		fuzz = evdev_read_fuzz_prop(device, ABS_X);
		libevdev_set_abs_fuzz(evdev, ABS_X, fuzz);
		fuzz = evdev_read_fuzz_prop(device, ABS_Y);
		libevdev_set_abs_fuzz(evdev, ABS_Y, fuzz);
	}

	device->abs.absinfo_x = libevdev_get_abs_info(evdev, ABS_X);
	device->abs.absinfo_y = libevdev_get_abs_info(evdev, ABS_Y);
	device->abs.dimensions.x = abs((int)absinfo_range(device->abs.absinfo_x));
	device->abs.dimensions.y = abs((int)absinfo_range(device->abs.absinfo_y));

	if (evdev_is_fake_mt_device(device) ||
	    !libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_X) ||
	    !libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_Y))
		 return;

	if (evdev_fix_abs_resolution(device,
				     ABS_MT_POSITION_X,
				     ABS_MT_POSITION_Y))
		device->abs.is_fake_resolution = true;

	if ((fuzz = evdev_read_fuzz_prop(device, ABS_MT_POSITION_X)))
	    libevdev_set_abs_fuzz(evdev, ABS_MT_POSITION_X, fuzz);
	if ((fuzz = evdev_read_fuzz_prop(device, ABS_MT_POSITION_Y)))
	    libevdev_set_abs_fuzz(evdev, ABS_MT_POSITION_Y, fuzz);

	device->abs.absinfo_x = libevdev_get_abs_info(evdev, ABS_MT_POSITION_X);
	device->abs.absinfo_y = libevdev_get_abs_info(evdev, ABS_MT_POSITION_Y);
	device->abs.dimensions.x = abs((int)absinfo_range(device->abs.absinfo_x));
	device->abs.dimensions.y = abs((int)absinfo_range(device->abs.absinfo_y));
	device->is_mt = 1;
}

static void
evdev_disable_accelerometer_axes(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;

	libevdev_disable_event_code(evdev, EV_ABS, ABS_X);
	libevdev_disable_event_code(evdev, EV_ABS, ABS_Y);
	libevdev_disable_event_code(evdev, EV_ABS, ABS_Z);

	libevdev_disable_event_code(evdev, EV_ABS, REL_X);
	libevdev_disable_event_code(evdev, EV_ABS, REL_Y);
	libevdev_disable_event_code(evdev, EV_ABS, REL_Z);
}

static bool
evdev_device_is_joystick_or_gamepad(struct evdev_device *device)
{
	enum evdev_device_udev_tags udev_tags;
	bool has_joystick_tags;
	struct libevdev *evdev = device->evdev;
	unsigned int code;

	/* The EVDEV_UDEV_TAG_JOYSTICK is set when a joystick or gamepad button
	 * is found. However, it can not be used to identify joysticks or
	 * gamepads because there are keyboards that also have it. Even worse,
	 * many joysticks also map KEY_* and thus are tagged as keyboards.
	 *
	 * In order to be able to detect joysticks and gamepads and
	 * differentiate them from keyboards, apply the following rules:
	 *
	 *  1. The device is tagged as joystick but not as tablet
	 *  2. The device doesn't have 4 well-known keyboard keys
	 *  3. It has at least 2 joystick buttons
	 *  4. It doesn't have 10 keyboard keys */

	udev_tags = evdev_device_get_udev_tags(device, device->udev_device);
	has_joystick_tags = (udev_tags & EVDEV_UDEV_TAG_JOYSTICK) &&
			    !(udev_tags & EVDEV_UDEV_TAG_TABLET) &&
			    !(udev_tags & EVDEV_UDEV_TAG_TABLET_PAD);

	if (!has_joystick_tags)
		return false;

	unsigned int num_well_known_keys = 0;

	for (size_t i = 0; i < ARRAY_LENGTH(well_known_keyboard_keys); i++) {
		code = well_known_keyboard_keys[i];
		if (libevdev_has_event_code(evdev, EV_KEY, code))
			num_well_known_keys++;
	}

	if (num_well_known_keys >= 4) /* should not have 4 well-known keys */
		return false;

	unsigned int num_joystick_btns = 0;

	for (code = BTN_JOYSTICK; code < BTN_DIGI; code++) {
		if (libevdev_has_event_code(evdev, EV_KEY, code))
			num_joystick_btns++;
	}

	for (code = BTN_TRIGGER_HAPPY; code <= BTN_TRIGGER_HAPPY40; code++) {
		if (libevdev_has_event_code(evdev, EV_KEY, code))
			num_joystick_btns++;
	}

	if (num_joystick_btns < 2) /* require at least 2 joystick buttons */
		return false;

	unsigned int num_keys = 0;

	for (code = KEY_ESC; code <= KEY_MICMUTE; code++) {
		if (libevdev_has_event_code(evdev, EV_KEY, code) )
			num_keys++;
	}

	for (code = KEY_OK; code <= KEY_LIGHTS_TOGGLE; code++) {
		if (libevdev_has_event_code(evdev, EV_KEY, code) )
			num_keys++;
	}

	for (code = KEY_ALS_TOGGLE; code < BTN_TRIGGER_HAPPY; code++) {
		if (libevdev_has_event_code(evdev, EV_KEY, code) )
			num_keys++;
	}

	if (num_keys >= 10) /* should not have 10 keyboard keys */
		return false;

	return true;
}

static struct evdev_dispatch *
evdev_configure_device(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;
	enum evdev_device_udev_tags udev_tags;
	unsigned int tablet_tags;
	struct evdev_dispatch *dispatch;

	udev_tags = evdev_device_get_udev_tags(device, device->udev_device);

	if ((udev_tags & EVDEV_UDEV_TAG_INPUT) == 0 ||
	    (udev_tags & ~EVDEV_UDEV_TAG_INPUT) == 0) {
		evdev_log_info(device,
			       "not tagged as supported input device\n");
		return NULL;
	}

	evdev_log_info(device,
		 "is tagged by udev as:%s%s%s%s%s%s%s%s%s%s%s\n",
		 udev_tags & EVDEV_UDEV_TAG_KEYBOARD ? " Keyboard" : "",
		 udev_tags & EVDEV_UDEV_TAG_MOUSE ? " Mouse" : "",
		 udev_tags & EVDEV_UDEV_TAG_TOUCHPAD ? " Touchpad" : "",
		 udev_tags & EVDEV_UDEV_TAG_TOUCHSCREEN ? " Touchscreen" : "",
		 udev_tags & EVDEV_UDEV_TAG_TABLET ? " Tablet" : "",
		 udev_tags & EVDEV_UDEV_TAG_POINTINGSTICK ? " Pointingstick" : "",
		 udev_tags & EVDEV_UDEV_TAG_JOYSTICK ? " Joystick" : "",
		 udev_tags & EVDEV_UDEV_TAG_ACCELEROMETER ? " Accelerometer" : "",
		 udev_tags & EVDEV_UDEV_TAG_TABLET_PAD ? " TabletPad" : "",
		 udev_tags & EVDEV_UDEV_TAG_TRACKBALL ? " Trackball" : "",
		 udev_tags & EVDEV_UDEV_TAG_SWITCH ? " Switch" : "");

	/* Ignore pure accelerometers, but accept devices that are
	 * accelerometers with other axes */
	if (udev_tags == (EVDEV_UDEV_TAG_INPUT|EVDEV_UDEV_TAG_ACCELEROMETER)) {
		evdev_log_info(device,
			 "device is an accelerometer, ignoring\n");
		return NULL;
	}

	if (udev_tags & EVDEV_UDEV_TAG_ACCELEROMETER) {
		evdev_disable_accelerometer_axes(device);
	}

	if (evdev_device_is_joystick_or_gamepad(device)) {
		evdev_log_info(device,
			       "device is a joystick or a gamepad, ignoring\n");
		return NULL;
	}

	if (evdev_reject_device(device)) {
		evdev_log_info(device, "was rejected\n");
		return NULL;
	}

	if (!evdev_is_fake_mt_device(device))
		evdev_fix_android_mt(device);

	if (libevdev_has_event_code(evdev, EV_ABS, ABS_X)) {
		evdev_extract_abs_axes(device, udev_tags);

		if (evdev_is_fake_mt_device(device))
			udev_tags &= ~EVDEV_UDEV_TAG_TOUCHSCREEN;
	}

	if (evdev_device_has_model_quirk(device,
					 QUIRK_MODEL_DELL_CANVAS_TOTEM)) {
		dispatch = evdev_totem_create(device);
		device->seat_caps |= EVDEV_DEVICE_TABLET;
		evdev_log_info(device, "device is a totem\n");
		return dispatch;
	}

	/* libwacom assigns touchpad (or touchscreen) _and_ tablet to the
	   tablet touch bits, so make sure we don't initialize the tablet
	   interface for the touch device */
	tablet_tags = EVDEV_UDEV_TAG_TABLET |
		      EVDEV_UDEV_TAG_TOUCHPAD |
		      EVDEV_UDEV_TAG_TOUCHSCREEN;

	/* libwacom assigns tablet _and_ tablet_pad to the pad devices */
	if (udev_tags & EVDEV_UDEV_TAG_TABLET_PAD) {
		dispatch = evdev_tablet_pad_create(device);
		device->seat_caps |= EVDEV_DEVICE_TABLET_PAD;
		evdev_log_info(device, "device is a tablet pad\n");
		return dispatch;

	}

	if ((udev_tags & tablet_tags) == EVDEV_UDEV_TAG_TABLET) {
		dispatch = evdev_tablet_create(device);
		device->seat_caps |= EVDEV_DEVICE_TABLET;
		evdev_log_info(device, "device is a tablet\n");
		return dispatch;
	}

	if (udev_tags & EVDEV_UDEV_TAG_TOUCHPAD) {
		if (udev_tags & EVDEV_UDEV_TAG_TABLET)
			evdev_tag_tablet_touchpad(device);
		/* whether velocity should be averaged, false by default */
		device->use_velocity_averaging = evdev_need_velocity_averaging(device);
		dispatch = evdev_mt_touchpad_create(device);
		evdev_log_info(device, "device is a touchpad\n");
		return dispatch;
	}

	if (udev_tags & EVDEV_UDEV_TAG_MOUSE ||
	    udev_tags & EVDEV_UDEV_TAG_POINTINGSTICK) {
		evdev_tag_external_mouse(device, device->udev_device);
		evdev_tag_trackpoint(device, device->udev_device);
		if (device->tags & EVDEV_TAG_TRACKPOINT)
			device->trackpoint_multiplier = evdev_get_trackpoint_multiplier(device);
		else
			device->dpi = evdev_read_dpi_prop(device);
		/* whether velocity should be averaged, false by default */
		device->use_velocity_averaging = evdev_need_velocity_averaging(device);

		device->seat_caps |= EVDEV_DEVICE_POINTER;

		evdev_log_info(device, "device is a pointer\n");

		/* want left-handed config option */
		device->left_handed.want_enabled = true;
		/* want natural-scroll config option */
		device->scroll.natural_scrolling_enabled = true;
		/* want button scrolling config option */
		if (libevdev_has_event_code(evdev, EV_REL, REL_X) ||
		    libevdev_has_event_code(evdev, EV_REL, REL_Y))
			device->scroll.want_button = 1;
	}

	if (udev_tags & EVDEV_UDEV_TAG_KEYBOARD) {
		device->seat_caps |= EVDEV_DEVICE_KEYBOARD;
		evdev_log_info(device, "device is a keyboard\n");

		/* want natural-scroll config option */
		if (libevdev_has_event_code(evdev, EV_REL, REL_WHEEL) ||
		    libevdev_has_event_code(evdev, EV_REL, REL_HWHEEL)) {
			device->scroll.natural_scrolling_enabled = true;
			device->seat_caps |= EVDEV_DEVICE_POINTER;
		}

		evdev_tag_keyboard(device, device->udev_device);
	}

	if (udev_tags & EVDEV_UDEV_TAG_TOUCHSCREEN) {
		device->seat_caps |= EVDEV_DEVICE_TOUCH;
		evdev_log_info(device, "device is a touch device\n");
	}

	if (udev_tags & EVDEV_UDEV_TAG_SWITCH) {
		if (libevdev_has_event_code(evdev, EV_SW, SW_LID)) {
			device->seat_caps |= EVDEV_DEVICE_SWITCH;
			device->tags |= EVDEV_TAG_LID_SWITCH;
		}

		if (libevdev_has_event_code(evdev, EV_SW, SW_TABLET_MODE)) {
		    if (evdev_device_has_model_quirk(device,
				 QUIRK_MODEL_TABLET_MODE_SWITCH_UNRELIABLE)) {
			    evdev_log_info(device,
				"device is an unreliable tablet mode switch, filtering events.\n");
			    libevdev_disable_event_code(device->evdev,
							EV_SW,
							SW_TABLET_MODE);
		    } else {
			    device->tags |= EVDEV_TAG_TABLET_MODE_SWITCH;
			    device->seat_caps |= EVDEV_DEVICE_SWITCH;
		    }
		}

		if (device->seat_caps & EVDEV_DEVICE_SWITCH)
		    evdev_log_info(device, "device is a switch device\n");
	}

	if (device->seat_caps & EVDEV_DEVICE_POINTER &&
	    libevdev_has_event_code(evdev, EV_REL, REL_X) &&
	    libevdev_has_event_code(evdev, EV_REL, REL_Y) &&
	    !evdev_init_accel(device, LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE)) {
		evdev_log_error(device,
				"failed to initialize pointer acceleration\n");
		return NULL;
	}

	if (evdev_device_has_model_quirk(device, QUIRK_MODEL_INVERT_HORIZONTAL_SCROLLING)) {
		device->scroll.invert_horizontal_scrolling = true;
	}

	return fallback_dispatch_create(&device->base);
}

static void
evdev_notify_added_device(struct evdev_device *device)
{
	struct libinput_device *dev;

	list_for_each(dev, &device->base.seat->devices_list, link) {
		struct evdev_device *d = evdev_device(dev);
		if (dev == &device->base)
			continue;

		/* Notify existing device d about addition of device */
		if (d->dispatch->interface->device_added)
			d->dispatch->interface->device_added(d, device);

		/* Notify new device about existing device d */
		if (device->dispatch->interface->device_added)
			device->dispatch->interface->device_added(device, d);

		/* Notify new device if existing device d is suspended */
		if (d->is_suspended &&
		    device->dispatch->interface->device_suspended)
			device->dispatch->interface->device_suspended(device, d);
	}

	notify_added_device(&device->base);

	if (device->dispatch->interface->post_added)
		device->dispatch->interface->post_added(device,
							device->dispatch);
}

static bool
evdev_device_have_same_syspath(struct udev_device *udev_device, int fd)
{
	struct udev *udev = udev_device_get_udev(udev_device);
	struct udev_device *udev_device_new = NULL;
	struct stat st;
	bool rc = false;

	if (fstat(fd, &st) < 0)
		goto out;

	udev_device_new = udev_device_new_from_devnum(udev, 'c', st.st_rdev);
	if (!udev_device_new)
		goto out;

	rc = streq(udev_device_get_syspath(udev_device_new),
		   udev_device_get_syspath(udev_device));
out:
	if (udev_device_new)
		udev_device_unref(udev_device_new);
	return rc;
}

static bool
evdev_set_device_group(struct evdev_device *device,
		       struct udev_device *udev_device)
{
	struct libinput *libinput = evdev_libinput_context(device);
	struct libinput_device_group *group = NULL;
	const char *udev_group;

	udev_group = udev_device_get_property_value(udev_device,
						    "LIBINPUT_DEVICE_GROUP");
	if (udev_group)
		group = libinput_device_group_find_group(libinput, udev_group);

	if (!group) {
		group = libinput_device_group_create(libinput, udev_group);
		if (!group)
			return false;
		libinput_device_set_device_group(&device->base, group);
		libinput_device_group_unref(group);
	} else {
		libinput_device_set_device_group(&device->base, group);
	}

	return true;
}

static inline void
evdev_drain_fd(int fd)
{
	struct input_event ev[24];
	size_t sz = sizeof ev;

	while (read(fd, &ev, sz) == (int)sz) {
		/* discard all pending events */
	}
}

static inline void
evdev_pre_configure_model_quirks(struct evdev_device *device)
{
	struct quirks_context *quirks;
	struct quirks *q;
	const struct quirk_tuples *t;
	char *prop;

	/* Touchpad claims to have 4 slots but only ever sends 2
	 * https://bugs.freedesktop.org/show_bug.cgi?id=98100 */
	if (evdev_device_has_model_quirk(device, QUIRK_MODEL_HP_ZBOOK_STUDIO_G3))
		libevdev_set_abs_maximum(device->evdev, ABS_MT_SLOT, 1);

	/* Generally we don't care about MSC_TIMESTAMP and it can cause
	 * unnecessary wakeups but on some devices we need to watch it for
	 * pointer jumps */
	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);
	if (!q ||
	    !quirks_get_string(q, QUIRK_ATTR_MSC_TIMESTAMP, &prop) ||
	    !streq(prop, "watch")) {
		libevdev_disable_event_code(device->evdev, EV_MSC, MSC_TIMESTAMP);
	}

	if (quirks_get_tuples(q, QUIRK_ATTR_EVENT_CODE, &t)) {
		for (size_t i = 0; i < t->ntuples; i++) {
			const struct input_absinfo absinfo = {
				.minimum = 0,
				.maximum = 1,
			};

			int type = t->tuples[i].first;
			int code = t->tuples[i].second;
			bool enable = t->tuples[i].third;

			if (code == EVENT_CODE_UNDEFINED) {
				if (enable)
					libevdev_enable_event_type(device->evdev, type);
				else
					libevdev_disable_event_type(device->evdev, type);
			} else {
				if (enable)
					libevdev_enable_event_code(device->evdev,
								   type,
								   code,
								   type == EV_ABS ?  &absinfo : NULL);
				else
					libevdev_disable_event_code(device->evdev,
								    type,
								    code);
			}
			evdev_log_debug(device,
					"quirks: %s %s %s (%#x %#x)\n",
					enable ? "enabling" : "disabling",
					libevdev_event_type_get_name(type),
					libevdev_event_code_get_name(type, code),
					type,
					code);
		}
	}

	if (quirks_get_tuples(q, QUIRK_ATTR_INPUT_PROP, &t)) {
		for (size_t idx = 0; idx < t->ntuples; idx++) {
			unsigned int p = t->tuples[idx].first;
			bool enable = t->tuples[idx].second;

			if (enable) {
				libevdev_enable_property(device->evdev, p);
			}
			else {
#if HAVE_LIBEVDEV_DISABLE_PROPERTY
				libevdev_disable_property(device->evdev, p);
#else
				evdev_log_error(device,
						"quirks: a quirk for this device requires newer libevdev than installed\n");
#endif
			}
			evdev_log_debug(device,
					"quirks: %s %s (%#x)\n",
					enable ? "enabling" : "disabling",
					libevdev_property_get_name(p),
					p);
		}
	}

	quirks_unref(q);
}

static void
libevdev_log_func(const struct libevdev *evdev,
		  enum libevdev_log_priority priority,
		  void *data,
		  const char *file,
		  int line,
		  const char *func,
		  const char *format,
		  va_list args)
{
	struct libinput *libinput = data;
	enum libinput_log_priority pri = LIBINPUT_LOG_PRIORITY_ERROR;
	const char prefix[] = "libevdev: ";
	char fmt[strlen(format) + strlen(prefix) + 1];

	switch (priority) {
	case LIBEVDEV_LOG_ERROR:
		pri = LIBINPUT_LOG_PRIORITY_ERROR;
		break;
	case LIBEVDEV_LOG_INFO:
		pri = LIBINPUT_LOG_PRIORITY_INFO;
		break;
	case LIBEVDEV_LOG_DEBUG:
		pri = LIBINPUT_LOG_PRIORITY_DEBUG;
		break;
	}

	snprintf(fmt, sizeof(fmt), "%s%s", prefix, format);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	log_msg_va(libinput, pri, fmt, args);
#pragma GCC diagnostic pop
}

static bool
udev_device_should_be_ignored(struct udev_device *udev_device)
{
	const char *value;

	value = udev_device_get_property_value(udev_device,
					       "LIBINPUT_IGNORE_DEVICE");

	return value && !streq(value, "0");
}

struct evdev_device *
evdev_device_create(struct libinput_seat *seat,
		    struct udev_device *udev_device)
{
	struct libinput *libinput = seat->libinput;
	struct evdev_device *device = NULL;
	int rc;
	int fd = -1;
	int unhandled_device = 0;
	const char *devnode = udev_device_get_devnode(udev_device);
	char *sysname = str_sanitize(udev_device_get_sysname(udev_device));

	if (!devnode) {
		log_info(libinput, "%s: no device node associated\n", sysname);
		goto err;
	}

	if (udev_device_should_be_ignored(udev_device)) {
		log_debug(libinput, "%s: device is ignored\n", sysname);
		goto err;
	}

	/* Use non-blocking mode so that we can loop on read on
	 * evdev_device_data() until all events on the fd are
	 * read.  mtdev_get() also expects this. */
	fd = open_restricted(libinput, devnode,
			     O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0) {
		log_info(libinput,
			 "%s: opening input device '%s' failed (%s).\n",
			 sysname,
			 devnode,
			 strerror(-fd));
		goto err;
	}

	if (!evdev_device_have_same_syspath(udev_device, fd))
		goto err;

	device = zalloc(sizeof *device);
	device->sysname = sysname;
	sysname = NULL;

	libinput_device_init(&device->base, seat);
	libinput_seat_ref(seat);

	evdev_drain_fd(fd);

	rc = libevdev_new_from_fd(fd, &device->evdev);
	if (rc != 0)
		goto err;

	libevdev_set_clock_id(device->evdev, CLOCK_MONOTONIC);
	libevdev_set_device_log_function(device->evdev,
					 libevdev_log_func,
					 LIBEVDEV_LOG_ERROR,
					 libinput);
	device->seat_caps = 0;
	device->is_mt = 0;
	device->mtdev = NULL;
	device->udev_device = udev_device_ref(udev_device);
	device->dispatch = NULL;
	device->fd = fd;
	device->devname = libevdev_get_name(device->evdev);
	/* the log_prefix_name is used as part of a printf format string and
	 * must not contain % directives, see evdev_log_msg */
	device->log_prefix_name = str_sanitize(device->devname);
	device->scroll.threshold = 5.0; /* Default may be overridden */
	device->scroll.direction_lock_threshold = 5.0; /* Default may be overridden */
	device->scroll.direction = 0;
	device->scroll.wheel_click_angle =
		evdev_read_wheel_click_props(device);
	device->model_flags = evdev_read_model_flags(device);
	device->dpi = DEFAULT_MOUSE_DPI;

	/* at most 5 SYN_DROPPED log-messages per 30s */
	ratelimit_init(&device->syn_drop_limit, s2us(30), 5);
	/* at most 5 "delayed processing" log messages per hour */
	ratelimit_init(&device->delay_warning_limit, s2us(60 * 60), 5);
	/* at most 5 log-messages per 5s */
	ratelimit_init(&device->nonpointer_rel_limit, s2us(5), 5);

	matrix_init_identity(&device->abs.calibration);
	matrix_init_identity(&device->abs.usermatrix);
	matrix_init_identity(&device->abs.default_calibration);

	evdev_pre_configure_model_quirks(device);

	device->dispatch = evdev_configure_device(device);
	if (device->dispatch == NULL || device->seat_caps == 0)
		goto err;

	device->source =
		libinput_add_fd(libinput, fd, evdev_device_dispatch, device);
	if (!device->source)
		goto err;

	if (!evdev_set_device_group(device, udev_device))
		goto err;

	list_insert(seat->devices_list.prev, &device->base.link);

	evdev_notify_added_device(device);

	return device;

err:
	if (fd >= 0) {
		close_restricted(libinput, fd);
		if (device) {
			unhandled_device = device->seat_caps == 0;
			evdev_device_destroy(device);
		}
	}

	free(sysname);

	return unhandled_device ? EVDEV_UNHANDLED_DEVICE :  NULL;
}

const char *
evdev_device_get_output(struct evdev_device *device)
{
	return device->output_name;
}

const char *
evdev_device_get_sysname(struct evdev_device *device)
{
	return device->sysname;
}

const char *
evdev_device_get_name(struct evdev_device *device)
{
	return device->devname;
}

unsigned int
evdev_device_get_id_product(struct evdev_device *device)
{
	return libevdev_get_id_product(device->evdev);
}

unsigned int
evdev_device_get_id_vendor(struct evdev_device *device)
{
	return libevdev_get_id_vendor(device->evdev);
}

struct udev_device *
evdev_device_get_udev_device(struct evdev_device *device)
{
	return udev_device_ref(device->udev_device);
}

void
evdev_device_set_default_calibration(struct evdev_device *device,
				     const float calibration[6])
{
	matrix_from_farray6(&device->abs.default_calibration, calibration);
	evdev_device_calibrate(device, calibration);
}

void
evdev_device_calibrate(struct evdev_device *device,
		       const float calibration[6])
{
	struct matrix scale,
		      translate,
		      transform;
	double sx, sy;

	matrix_from_farray6(&transform, calibration);
	device->abs.apply_calibration = !matrix_is_identity(&transform);

	/* back up the user matrix so we can return it on request */
	matrix_from_farray6(&device->abs.usermatrix, calibration);

	if (!device->abs.apply_calibration) {
		matrix_init_identity(&device->abs.calibration);
		return;
	}

	sx = absinfo_range(device->abs.absinfo_x);
	sy = absinfo_range(device->abs.absinfo_y);

	/* The transformation matrix is in the form:
	 *  [ a b c ]
	 *  [ d e f ]
	 *  [ 0 0 1 ]
	 * Where a, e are the scale components, a, b, d, e are the rotation
	 * component (combined with scale) and c and f are the translation
	 * component. The translation component in the input matrix must be
	 * normalized to multiples of the device width and height,
	 * respectively. e.g. c == 1 shifts one device-width to the right.
	 *
	 * We pre-calculate a single matrix to apply to event coordinates:
	 *     M = Un-Normalize * Calibration * Normalize
	 *
	 * Normalize: scales the device coordinates to [0,1]
	 * Calibration: user-supplied matrix
	 * Un-Normalize: scales back up to device coordinates
	 * Matrix maths requires the normalize/un-normalize in reverse
	 * order.
	 */

	/* Un-Normalize */
	matrix_init_translate(&translate,
			      device->abs.absinfo_x->minimum,
			      device->abs.absinfo_y->minimum);
	matrix_init_scale(&scale, sx, sy);
	matrix_mult(&scale, &translate, &scale);

	/* Calibration */
	matrix_mult(&transform, &scale, &transform);

	/* Normalize */
	matrix_init_translate(&translate,
			      -device->abs.absinfo_x->minimum/sx,
			      -device->abs.absinfo_y->minimum/sy);
	matrix_init_scale(&scale, 1.0/sx, 1.0/sy);
	matrix_mult(&scale, &translate, &scale);

	/* store final matrix in device */
	matrix_mult(&device->abs.calibration, &transform, &scale);
}

void
evdev_read_calibration_prop(struct evdev_device *device)
{
	const char *prop;
	float calibration[6];

	prop = udev_device_get_property_value(device->udev_device,
					      "LIBINPUT_CALIBRATION_MATRIX");

	if (prop == NULL)
		return;

	if (!device->abs.absinfo_x || !device->abs.absinfo_y)
		return;

	if (!parse_calibration_property(prop, calibration))
		return;

	evdev_device_set_default_calibration(device, calibration);
	evdev_log_info(device,
		       "applying calibration: %f %f %f %f %f %f\n",
		       calibration[0],
		       calibration[1],
		       calibration[2],
		       calibration[3],
		       calibration[4],
		       calibration[5]);
}

int
evdev_read_fuzz_prop(struct evdev_device *device, unsigned int code)
{
	const char *prop;
	char name[32];
	int rc;
	int fuzz = 0;
	const struct input_absinfo *abs;

	rc = snprintf(name, sizeof(name), "LIBINPUT_FUZZ_%02x", code);
	if (rc == -1)
		return 0;

	prop = udev_device_get_property_value(device->udev_device, name);
	if (prop && (safe_atoi(prop, &fuzz) == false || fuzz < 0)) {
		evdev_log_bug_libinput(device,
				       "invalid LIBINPUT_FUZZ property value: %s\n",
				       prop);
		return 0;
	}

	/* The udev callout should have set the kernel fuzz to zero.
	 * If the kernel fuzz is nonzero, something has gone wrong there, so
	 * let's complain but still use a fuzz of zero for our view of the
	 * device. Otherwise, the kernel will use the nonzero fuzz, we then
	 * use the same fuzz on top of the pre-fuzzed data and that leads to
	 * unresponsive behaviur.
	 */
	abs = libevdev_get_abs_info(device->evdev, code);
	if (!abs || abs->fuzz == 0)
		return fuzz;

	if (prop) {
		evdev_log_bug_libinput(device,
				       "kernel fuzz of %d even with LIBINPUT_FUZZ_%02x present\n",
				       abs->fuzz,
				       code);
	} else {
		evdev_log_bug_libinput(device,
				       "kernel fuzz of %d but LIBINPUT_FUZZ_%02x is missing\n",
				       abs->fuzz,
				       code);
	}

	return 0;
}

bool
evdev_device_has_capability(struct evdev_device *device,
			    enum libinput_device_capability capability)
{
	switch (capability) {
	case LIBINPUT_DEVICE_CAP_POINTER:
		return !!(device->seat_caps & EVDEV_DEVICE_POINTER);
	case LIBINPUT_DEVICE_CAP_KEYBOARD:
		return !!(device->seat_caps & EVDEV_DEVICE_KEYBOARD);
	case LIBINPUT_DEVICE_CAP_TOUCH:
		return !!(device->seat_caps & EVDEV_DEVICE_TOUCH);
	case LIBINPUT_DEVICE_CAP_GESTURE:
		return !!(device->seat_caps & EVDEV_DEVICE_GESTURE);
	case LIBINPUT_DEVICE_CAP_TABLET_TOOL:
		return !!(device->seat_caps & EVDEV_DEVICE_TABLET);
	case LIBINPUT_DEVICE_CAP_TABLET_PAD:
		return !!(device->seat_caps & EVDEV_DEVICE_TABLET_PAD);
	case LIBINPUT_DEVICE_CAP_SWITCH:
		return !!(device->seat_caps & EVDEV_DEVICE_SWITCH);
	default:
		return false;
	}
}

int
evdev_device_get_size(const struct evdev_device *device,
		      double *width,
		      double *height)
{
	const struct input_absinfo *x, *y;

	x = libevdev_get_abs_info(device->evdev, ABS_X);
	y = libevdev_get_abs_info(device->evdev, ABS_Y);

	if (!x || !y || device->abs.is_fake_resolution ||
	    !x->resolution || !y->resolution)
		return -1;

	*width = evdev_convert_to_mm(x, x->maximum);
	*height = evdev_convert_to_mm(y, y->maximum);

	return 0;
}

int
evdev_device_has_button(struct evdev_device *device, uint32_t code)
{
	if (!(device->seat_caps & EVDEV_DEVICE_POINTER))
		return -1;

	return libevdev_has_event_code(device->evdev, EV_KEY, code);
}

int
evdev_device_has_key(struct evdev_device *device, uint32_t code)
{
	if (!(device->seat_caps & EVDEV_DEVICE_KEYBOARD))
		return -1;

	return libevdev_has_event_code(device->evdev, EV_KEY, code);
}

int
evdev_device_get_touch_count(struct evdev_device *device)
{
	int ntouches;

	if (!(device->seat_caps & EVDEV_DEVICE_TOUCH))
		return -1;

	ntouches = libevdev_get_num_slots(device->evdev);
	if (ntouches == -1) {
		/* mtdev devices have multitouch but we don't know
		 * how many. Otherwise, any touch device with num_slots of
		 * -1 is a single-touch device */
		if (device->mtdev)
			ntouches = 0;
		else
			ntouches = 1;
	}

	return ntouches;
}

int
evdev_device_has_switch(struct evdev_device *device,
			enum libinput_switch sw)
{
	unsigned int code;

	if (!(device->seat_caps & EVDEV_DEVICE_SWITCH))
		return -1;

	switch (sw) {
	case LIBINPUT_SWITCH_LID:
		code = SW_LID;
		break;
	case LIBINPUT_SWITCH_TABLET_MODE:
		code = SW_TABLET_MODE;
		break;
	default:
		return -1;
	}

	return libevdev_has_event_code(device->evdev, EV_SW, code);
}

static inline bool
evdev_is_scrolling(const struct evdev_device *device,
		   enum libinput_pointer_axis axis)
{
	assert(axis == LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL ||
	       axis == LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

	return (device->scroll.direction & bit(axis)) != 0;
}

static inline void
evdev_start_scrolling(struct evdev_device *device,
		      enum libinput_pointer_axis axis)
{
	assert(axis == LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL ||
	       axis == LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

	device->scroll.direction |= bit(axis);
}

void
evdev_post_scroll(struct evdev_device *device,
		  uint64_t time,
		  enum libinput_pointer_axis_source source,
		  const struct normalized_coords *delta)
{
	const struct normalized_coords *trigger;
	struct normalized_coords event;

	if (!evdev_is_scrolling(device,
				LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
		device->scroll.buildup.y += delta->y;
	if (!evdev_is_scrolling(device,
				LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
		device->scroll.buildup.x += delta->x;

	trigger = &device->scroll.buildup;

	/* If we're not scrolling yet, use a distance trigger: moving
	   past a certain distance starts scrolling */
	if (!evdev_is_scrolling(device,
				LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) &&
	    !evdev_is_scrolling(device,
				LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
		if (fabs(trigger->y) >= device->scroll.threshold)
			evdev_start_scrolling(device,
					      LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
		if (fabs(trigger->x) >= device->scroll.threshold)
			evdev_start_scrolling(device,
					      LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
	/* We're already scrolling in one direction. Require some
	   trigger speed to start scrolling in the other direction */
	} else if (!evdev_is_scrolling(device,
			       LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
		if (fabs(delta->y) >= device->scroll.direction_lock_threshold)
			evdev_start_scrolling(device,
				      LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
	} else if (!evdev_is_scrolling(device,
				LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
		if (fabs(delta->x) >= device->scroll.direction_lock_threshold)
			evdev_start_scrolling(device,
				      LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
	}

	event = *delta;

	/* We use the trigger to enable, but the delta from this event for
	 * the actual scroll movement. Otherwise we get a jump once
	 * scrolling engages */
	if (!evdev_is_scrolling(device,
			       LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
		event.y = 0.0;

	if (!evdev_is_scrolling(device,
			       LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
		event.x = 0.0;

	if (!normalized_is_zero(event)) {
		uint32_t axes = device->scroll.direction;

		if (event.y == 0.0)
			axes &= ~bit(LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
		if (event.x == 0.0)
			axes &= ~bit(LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);

		switch (source) {
		case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
			evdev_notify_axis_finger(device, time, axes, &event);
			break;
		case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
			evdev_notify_axis_continous(device, time, axes, &event);
			break;
		default:
			evdev_log_bug_libinput(device,
					       "Posting invalid scroll source %d\n",
					       source);
			break;
		}
	}
}

void
evdev_stop_scroll(struct evdev_device *device,
		  uint64_t time,
		  enum libinput_pointer_axis_source source)
{
	const struct normalized_coords zero = { 0.0, 0.0 };

	/* terminate scrolling with a zero scroll event */
	if (device->scroll.direction != 0) {
		switch (source) {
		case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
			pointer_notify_axis_finger(&device->base,
						   time,
						   device->scroll.direction,
						   &zero);
			break;
		case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
			pointer_notify_axis_continuous(&device->base,
						       time,
						       device->scroll.direction,
						       &zero);
			break;
		default:
			evdev_log_bug_libinput(device,
					       "Stopping invalid scroll source %d\n",
					       source);
			break;
		}
	}

	device->scroll.buildup.x = 0;
	device->scroll.buildup.y = 0;
	device->scroll.direction = 0;
}

void
evdev_notify_suspended_device(struct evdev_device *device)
{
	struct libinput_device *it;

	if (device->is_suspended)
		return;

	list_for_each(it, &device->base.seat->devices_list, link) {
		struct evdev_device *d = evdev_device(it);
		if (it == &device->base)
			continue;

		if (d->dispatch->interface->device_suspended)
			d->dispatch->interface->device_suspended(d, device);
	}

	device->is_suspended = true;
}

void
evdev_notify_resumed_device(struct evdev_device *device)
{
	struct libinput_device *it;

	if (!device->is_suspended)
		return;

	list_for_each(it, &device->base.seat->devices_list, link) {
		struct evdev_device *d = evdev_device(it);
		if (it == &device->base)
			continue;

		if (d->dispatch->interface->device_resumed)
			d->dispatch->interface->device_resumed(d, device);
	}

	device->is_suspended = false;
}

void
evdev_device_suspend(struct evdev_device *device)
{
	struct libinput *libinput = evdev_libinput_context(device);

	evdev_notify_suspended_device(device);

	if (device->dispatch->interface->suspend)
		device->dispatch->interface->suspend(device->dispatch,
						     device);

	if (device->source) {
		libinput_remove_source(libinput, device->source);
		device->source = NULL;
	}

	if (device->mtdev) {
		mtdev_close_delete(device->mtdev);
		device->mtdev = NULL;
	}

	if (device->fd != -1) {
		close_restricted(libinput, device->fd);
		device->fd = -1;
	}
}

int
evdev_device_resume(struct evdev_device *device)
{
	struct libinput *libinput = evdev_libinput_context(device);
	int fd;
	const char *devnode;
	struct input_event ev;
	enum libevdev_read_status status;

	if (device->fd != -1)
		return 0;

	if (device->was_removed)
		return -ENODEV;

	devnode = udev_device_get_devnode(device->udev_device);
	if (!devnode)
		return -ENODEV;

	fd = open_restricted(libinput, devnode,
			     O_RDWR | O_NONBLOCK | O_CLOEXEC);

	if (fd < 0)
		return -errno;

	if (!evdev_device_have_same_syspath(device->udev_device, fd)) {
		close_restricted(libinput, fd);
		return -ENODEV;
	}

	evdev_drain_fd(fd);

	device->fd = fd;

	if (evdev_need_mtdev(device)) {
		device->mtdev = mtdev_new_open(device->fd);
		if (!device->mtdev)
			return -ENODEV;
	}

	libevdev_change_fd(device->evdev, fd);
	libevdev_set_clock_id(device->evdev, CLOCK_MONOTONIC);

	/* re-sync libevdev's view of the device, but discard the actual
	   events. Our device is in a neutral state already */
	libevdev_next_event(device->evdev,
			    LIBEVDEV_READ_FLAG_FORCE_SYNC,
			    &ev);
	do {
		status = libevdev_next_event(device->evdev,
					     LIBEVDEV_READ_FLAG_SYNC,
					     &ev);
	} while (status == LIBEVDEV_READ_STATUS_SYNC);

	device->source =
		libinput_add_fd(libinput, fd, evdev_device_dispatch, device);
	if (!device->source) {
		mtdev_close_delete(device->mtdev);
		return -ENOMEM;
	}

	evdev_notify_resumed_device(device);

	return 0;
}

void
evdev_device_remove(struct evdev_device *device)
{
	struct libinput_device *dev;

	evdev_log_info(device, "device removed\n");

	libinput_timer_cancel(&device->scroll.timer);
	libinput_timer_cancel(&device->middlebutton.timer);

	list_for_each(dev, &device->base.seat->devices_list, link) {
		struct evdev_device *d = evdev_device(dev);
		if (dev == &device->base)
			continue;

		if (d->dispatch->interface->device_removed)
			d->dispatch->interface->device_removed(d, device);
	}

	evdev_device_suspend(device);

	if (device->dispatch->interface->remove)
		device->dispatch->interface->remove(device->dispatch);

	/* A device may be removed while suspended, mark it to
	 * skip re-opening a different device with the same node */
	device->was_removed = true;

	list_remove(&device->base.link);

	notify_removed_device(&device->base);
	libinput_device_unref(&device->base);
}

void
evdev_device_destroy(struct evdev_device *device)
{
	struct evdev_dispatch *dispatch;

	dispatch = device->dispatch;
	if (dispatch)
		dispatch->interface->destroy(dispatch);

	if (device->base.group)
		libinput_device_group_unref(device->base.group);

	free(device->log_prefix_name);
	free(device->sysname);
	free(device->output_name);
	filter_destroy(device->pointer.filter);
	libinput_timer_destroy(&device->scroll.timer);
	libinput_timer_destroy(&device->middlebutton.timer);
	libinput_seat_unref(device->base.seat);
	libevdev_free(device->evdev);
	udev_device_unref(device->udev_device);
	free(device);
}

bool
evdev_tablet_has_left_handed(struct evdev_device *device)
{
	bool has_left_handed = true;
#if HAVE_LIBWACOM
	struct libinput *li = evdev_libinput_context(device);
	WacomDeviceDatabase *db = NULL;
	WacomDevice *d = NULL;
	WacomError *error;
	const char *devnode;

	db = libinput_libwacom_ref(li);
	if (!db)
		goto out;

	error = libwacom_error_new();
	devnode = udev_device_get_devnode(device->udev_device);

	d = libwacom_new_from_path(db,
				   devnode,
				   WFALLBACK_NONE,
				   error);

	if (d) {
		has_left_handed = !!libwacom_is_reversible(d);
	} else if (libwacom_error_get_code(error) == WERROR_UNKNOWN_MODEL) {
		evdev_log_info(device,
			       "tablet '%s' unknown to libwacom\n",
			       device->devname);
	} else {
		evdev_log_error(device,
				"libwacom error: %s\n",
				libwacom_error_get_message(error));
	}

	if (error)
		libwacom_error_free(&error);
	if (d)
		libwacom_destroy(d);
	if (db)
		libinput_libwacom_unref(li);

out:
#endif
	return has_left_handed;
}
