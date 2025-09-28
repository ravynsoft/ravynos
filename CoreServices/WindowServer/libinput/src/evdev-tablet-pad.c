/*
 * Copyright © 2016 Red Hat, Inc.
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
#include "evdev-tablet-pad.h"
#include "util-input-event.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#if HAVE_LIBWACOM
#include <libwacom/libwacom.h>
#endif

#define pad_set_status(pad_,s_) (pad_)->status |= (s_)
#define pad_unset_status(pad_,s_) (pad_)->status &= ~(s_)
#define pad_has_status(pad_,s_) (!!((pad_)->status & (s_)))

static void
pad_get_buttons_pressed(struct pad_dispatch *pad,
			struct button_state *buttons)
{
	struct button_state *state = &pad->button_state;
	struct button_state *prev_state = &pad->prev_button_state;
	unsigned int i;

	for (i = 0; i < sizeof(buttons->bits); i++)
		buttons->bits[i] = state->bits[i] & ~(prev_state->bits[i]);
}

static void
pad_get_buttons_released(struct pad_dispatch *pad,
			 struct button_state *buttons)
{
	struct button_state *state = &pad->button_state;
	struct button_state *prev_state = &pad->prev_button_state;
	unsigned int i;

	for (i = 0; i < sizeof(buttons->bits); i++)
		buttons->bits[i] = prev_state->bits[i] & ~(state->bits[i]);
}

static inline bool
pad_button_is_down(const struct pad_dispatch *pad,
		   uint32_t button)
{
	return bit_is_set(pad->button_state.bits, button);
}

static inline bool
pad_any_button_down(const struct pad_dispatch *pad)
{
	const struct button_state *state = &pad->button_state;
	unsigned int i;

	for (i = 0; i < sizeof(state->bits); i++)
		if (state->bits[i] != 0)
			return true;

	return false;
}

static inline void
pad_button_set_down(struct pad_dispatch *pad,
		    uint32_t button,
		    bool is_down)
{
	struct button_state *state = &pad->button_state;

	if (is_down) {
		set_bit(state->bits, button);
		pad_set_status(pad, PAD_BUTTONS_PRESSED);
	} else {
		clear_bit(state->bits, button);
		pad_set_status(pad, PAD_BUTTONS_RELEASED);
	}
}

static void
pad_process_absolute(struct pad_dispatch *pad,
		     struct evdev_device *device,
		     struct input_event *e,
		     uint64_t time)
{
	switch (e->code) {
	case ABS_WHEEL:
		pad->changed_axes |= PAD_AXIS_RING1;
		pad_set_status(pad, PAD_AXES_UPDATED);
		break;
	case ABS_THROTTLE:
		pad->changed_axes |= PAD_AXIS_RING2;
		pad_set_status(pad, PAD_AXES_UPDATED);
		break;
	case ABS_RX:
		pad->changed_axes |= PAD_AXIS_STRIP1;
		pad_set_status(pad, PAD_AXES_UPDATED);
		break;
	case ABS_RY:
		pad->changed_axes |= PAD_AXIS_STRIP2;
		pad_set_status(pad, PAD_AXES_UPDATED);
		break;
	case ABS_MISC:
		/* The wacom driver always sends a 0 axis event on finger
		   up, but we also get an ABS_MISC 15 on touch down and
		   ABS_MISC 0 on touch up, on top of the actual event. This
		   is kernel behavior for xf86-input-wacom backwards
		   compatibility after the 3.17 wacom HID move.

		   We use that event to tell when we truly went a full
		   rotation around the wheel vs. a finger release.

		   FIXME: On the Intuos5 and later the kernel merges all
		   states into that event, so if any finger is down on any
		   button, the wheel release won't trigger the ABS_MISC 0
		   but still send a 0 event. We can't currently detect this.
		 */
		pad->have_abs_misc_terminator = true;
		break;
	default:
		evdev_log_info(device,
			       "Unhandled EV_ABS event code %#x\n",
			       e->code);
		break;
	}
}

static inline double
normalize_ring(const struct input_absinfo *absinfo)
{
	/* libinput has 0 as the ring's northernmost point in the device's
	   current logical rotation, increasing clockwise to 1. Wacom has
	   0 on the left-most wheel position.
	 */
	double range = absinfo_range(absinfo);
	double value = (absinfo->value - absinfo->minimum) / range - 0.25;

	if (value < 0.0)
		value += 1.0;

	return value;
}

static inline double
normalize_strip(const struct input_absinfo *absinfo)
{
	/* strip axes don't use a proper value, they just shift the bit left
	 * for each position. 0 isn't a real value either, it's only sent on
	 * finger release */
	double min = 0,
	       max = log2(absinfo->maximum);
	double range = max - min;
	double value = (log2(absinfo->value) - min) / range;

	return value;
}

static inline double
pad_handle_ring(struct pad_dispatch *pad,
		struct evdev_device *device,
		unsigned int code)
{
	const struct input_absinfo *absinfo;
	double degrees;

	absinfo = libevdev_get_abs_info(device->evdev, code);
	assert(absinfo);

	degrees = normalize_ring(absinfo) * 360;

	if (device->left_handed.enabled)
		degrees = fmod(degrees + 180, 360);

	return degrees;
}

static inline double
pad_handle_strip(struct pad_dispatch *pad,
		 struct evdev_device *device,
		 unsigned int code)
{
	const struct input_absinfo *absinfo;
	double pos;

	absinfo = libevdev_get_abs_info(device->evdev, code);
	assert(absinfo);

	if (absinfo->value == 0)
		return 0.0;

	pos = normalize_strip(absinfo);

	if (device->left_handed.enabled)
		pos = 1.0 - pos;

	return pos;
}

static inline struct libinput_tablet_pad_mode_group *
pad_ring_get_mode_group(struct pad_dispatch *pad,
			unsigned int ring)
{
	struct libinput_tablet_pad_mode_group *group;

	list_for_each(group, &pad->modes.mode_group_list, link) {
		if (libinput_tablet_pad_mode_group_has_ring(group, ring))
			return group;
	}

	assert(!"Unable to find ring mode group");

	return NULL;
}

static inline struct libinput_tablet_pad_mode_group *
pad_strip_get_mode_group(struct pad_dispatch *pad,
			unsigned int strip)
{
	struct libinput_tablet_pad_mode_group *group;

	list_for_each(group, &pad->modes.mode_group_list, link) {
		if (libinput_tablet_pad_mode_group_has_strip(group, strip))
			return group;
	}

	assert(!"Unable to find strip mode group");

	return NULL;
}

static void
pad_check_notify_axes(struct pad_dispatch *pad,
		      struct evdev_device *device,
		      uint64_t time)
{
	struct libinput_device *base = &device->base;
	struct libinput_tablet_pad_mode_group *group;
	double value;
	bool send_finger_up = false;

	/* Suppress the reset to 0 on finger up. See the
	   comment in pad_process_absolute */
	if (pad->have_abs_misc_terminator &&
	    libevdev_get_event_value(device->evdev, EV_ABS, ABS_MISC) == 0)
		send_finger_up = true;

	if (pad->changed_axes & PAD_AXIS_RING1) {
		value = pad_handle_ring(pad, device, ABS_WHEEL);
		if (send_finger_up)
			value = -1.0;

		group = pad_ring_get_mode_group(pad, 0);
		tablet_pad_notify_ring(base,
				       time,
				       0,
				       value,
				       LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER,
				       group);
	}

	if (pad->changed_axes & PAD_AXIS_RING2) {
		value = pad_handle_ring(pad, device, ABS_THROTTLE);
		if (send_finger_up)
			value = -1.0;

		group = pad_ring_get_mode_group(pad, 1);
		tablet_pad_notify_ring(base,
				       time,
				       1,
				       value,
				       LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER,
				       group);
	}

	if (pad->changed_axes & PAD_AXIS_STRIP1) {
		value = pad_handle_strip(pad, device, ABS_RX);
		if (send_finger_up)
			value = -1.0;

		group = pad_strip_get_mode_group(pad, 0);
		tablet_pad_notify_strip(base,
					time,
					0,
					value,
					LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER,
					group);
	}

	if (pad->changed_axes & PAD_AXIS_STRIP2) {
		value = pad_handle_strip(pad, device, ABS_RY);
		if (send_finger_up)
			value = -1.0;

		group = pad_strip_get_mode_group(pad, 1);
		tablet_pad_notify_strip(base,
					time,
					1,
					value,
					LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER,
					group);
	}

	pad->changed_axes = PAD_AXIS_NONE;
	pad->have_abs_misc_terminator = false;
}

static void
pad_process_key(struct pad_dispatch *pad,
		struct evdev_device *device,
		struct input_event *e,
		uint64_t time)
{
	uint32_t button = e->code;
	uint32_t is_press = e->value != 0;

	/* ignore kernel key repeat */
	if (e->value == 2)
		return;

	pad_button_set_down(pad, button, is_press);
}

static inline struct libinput_tablet_pad_mode_group *
pad_button_get_mode_group(struct pad_dispatch *pad,
			  unsigned int button)
{
	struct libinput_tablet_pad_mode_group *group;

	list_for_each(group, &pad->modes.mode_group_list, link) {
		if (libinput_tablet_pad_mode_group_has_button(group, button))
			return group;
	}

	assert(!"Unable to find button mode group\n");

	return NULL;
}

static void
pad_notify_button_mask(struct pad_dispatch *pad,
		       struct evdev_device *device,
		       uint64_t time,
		       const struct button_state *buttons,
		       enum libinput_button_state state)
{
	struct libinput_device *base = &device->base;
	struct libinput_tablet_pad_mode_group *group;
	int32_t code;
	unsigned int i;

	for (i = 0; i < sizeof(buttons->bits); i++) {
		unsigned char buttons_slice = buttons->bits[i];

		code = i * 8;
		while (buttons_slice) {
			int enabled;
			key_or_button_map_t map;

			code++;
			enabled = (buttons_slice & 1);
			buttons_slice >>= 1;

			if (!enabled)
				continue;

			map = pad->button_map[code - 1];
			if (map_is_unmapped(map))
				continue;

			if (map_is_button(map)) {
				int32_t button = map_value(map);

				group = pad_button_get_mode_group(pad, button);
				pad_button_update_mode(group, button, state);
				tablet_pad_notify_button(base,
							 time,
							 button,
							 state,
							 group);
			} else if (map_is_key(map)) {
				uint32_t key = map_value(map);

				tablet_pad_notify_key(base,
						      time,
						      key,
						      (enum libinput_key_state)state);
			} else {
				abort();
			}
		}
	}
}

static void
pad_notify_buttons(struct pad_dispatch *pad,
		   struct evdev_device *device,
		   uint64_t time,
		   enum libinput_button_state state)
{
	struct button_state buttons;

	if (state == LIBINPUT_BUTTON_STATE_PRESSED)
		pad_get_buttons_pressed(pad, &buttons);
	else
		pad_get_buttons_released(pad, &buttons);

	pad_notify_button_mask(pad, device, time, &buttons, state);
}

static void
pad_change_to_left_handed(struct evdev_device *device)
{
	struct pad_dispatch *pad = (struct pad_dispatch*)device->dispatch;

	if (device->left_handed.enabled == device->left_handed.want_enabled)
		return;

	if (pad_any_button_down(pad))
		return;

	device->left_handed.enabled = device->left_handed.want_enabled;
}

static void
pad_flush(struct pad_dispatch *pad,
	  struct evdev_device *device,
	  uint64_t time)
{
	if (pad_has_status(pad, PAD_AXES_UPDATED)) {
		pad_check_notify_axes(pad, device, time);
		pad_unset_status(pad, PAD_AXES_UPDATED);
	}

	if (pad_has_status(pad, PAD_BUTTONS_RELEASED)) {
		pad_notify_buttons(pad,
				   device,
				   time,
				   LIBINPUT_BUTTON_STATE_RELEASED);
		pad_unset_status(pad, PAD_BUTTONS_RELEASED);

		pad_change_to_left_handed(device);
	}

	if (pad_has_status(pad, PAD_BUTTONS_PRESSED)) {
		pad_notify_buttons(pad,
				   device,
				   time,
				   LIBINPUT_BUTTON_STATE_PRESSED);
		pad_unset_status(pad, PAD_BUTTONS_PRESSED);
	}

	/* Update state */
	memcpy(&pad->prev_button_state,
	       &pad->button_state,
	       sizeof(pad->button_state));
}

static void
pad_process(struct evdev_dispatch *dispatch,
	    struct evdev_device *device,
	    struct input_event *e,
	    uint64_t time)
{
	struct pad_dispatch *pad = pad_dispatch(dispatch);

	switch (e->type) {
	case EV_ABS:
		pad_process_absolute(pad, device, e, time);
		break;
	case EV_KEY:
		pad_process_key(pad, device, e, time);
		break;
	case EV_SYN:
		pad_flush(pad, device, time);
		break;
	case EV_MSC:
		/* The EKR sends the serial as MSC_SERIAL, ignore this for
		 * now */
		break;
	default:
		evdev_log_error(device,
				"Unexpected event type %s (%#x)\n",
				libevdev_event_type_get_name(e->type),
				e->type);
		break;
	}
}

static void
pad_suspend(struct evdev_dispatch *dispatch,
	    struct evdev_device *device)
{
	struct pad_dispatch *pad = pad_dispatch(dispatch);
	struct libinput *libinput = pad_libinput_context(pad);
	unsigned int code;

	for (code = KEY_ESC; code < KEY_CNT; code++) {
		if (pad_button_is_down(pad, code))
			pad_button_set_down(pad, code, false);
	}

	pad_flush(pad, device, libinput_now(libinput));
}

static void
pad_destroy(struct evdev_dispatch *dispatch)
{
	struct pad_dispatch *pad = pad_dispatch(dispatch);

	pad_destroy_leds(pad);
	free(pad);
}

static struct evdev_dispatch_interface pad_interface = {
	.process = pad_process,
	.suspend = pad_suspend,
	.remove = NULL,
	.destroy = pad_destroy,
	.device_added = NULL,
	.device_removed = NULL,
	.device_suspended = NULL,
	.device_resumed = NULL,
	.post_added = NULL,
	.touch_arbitration_toggle = NULL,
	.touch_arbitration_update_rect = NULL,
	.get_switch_state = NULL,
};

static bool
pad_init_buttons_from_libwacom(struct pad_dispatch *pad,
			       struct evdev_device *device)
{
	bool rc = false;
#if HAVE_LIBWACOM
	struct libinput *li = pad_libinput_context(pad);
	WacomDeviceDatabase *db = NULL;
	WacomDevice *tablet = NULL;
	int num_buttons;
	int map = 0;
	char event_path[64];

	db = libinput_libwacom_ref(li);
	if (!db)
		goto out;

	snprintf(event_path,
		 sizeof(event_path),
		 "/dev/input/%s",
		 evdev_device_get_sysname(device));
	tablet = libwacom_new_from_path(db,
					event_path,
					WFALLBACK_NONE,
					NULL);
	if (!tablet) {
		tablet = libwacom_new_from_usbid(db,
						 evdev_device_get_id_vendor(device),
						 evdev_device_get_id_product(device),
						 NULL);
	}

	if (!tablet)
		goto out;

	num_buttons = libwacom_get_num_buttons(tablet);
	for (int i = 0; i < num_buttons; i++) {
		unsigned int code;

		code = libwacom_get_button_evdev_code(tablet, 'A' + i);
		if (code == 0)
			continue;

		map_set_button_map(pad->button_map[code], map++);
	}

	pad->nbuttons = map;

	rc = true;
out:
	if (tablet)
		libwacom_destroy(tablet);
	if (db)
		libinput_libwacom_unref(li);
#endif
	return rc;
}

static void
pad_init_buttons_from_kernel(struct pad_dispatch *pad,
			       struct evdev_device *device)
{
	unsigned int code;
	int map = 0;

	/* we match wacom_report_numbered_buttons() from the kernel */
	for (code = BTN_0; code < BTN_0 + 10; code++) {
		if (libevdev_has_event_code(device->evdev, EV_KEY, code))
			map_set_button_map(pad->button_map[code], map++);
	}

	for (code = BTN_BASE; code < BTN_BASE + 2; code++) {
		if (libevdev_has_event_code(device->evdev, EV_KEY, code))
			map_set_button_map(pad->button_map[code], map++);
	}

	for (code = BTN_A; code < BTN_A + 6; code++) {
		if (libevdev_has_event_code(device->evdev, EV_KEY, code))
			map_set_button_map(pad->button_map[code], map++);
	}

	for (code = BTN_LEFT; code < BTN_LEFT + 7; code++) {
		if (libevdev_has_event_code(device->evdev, EV_KEY, code))
			map_set_button_map(pad->button_map[code], map++);
	}

	pad->nbuttons = map;
}

static void
pad_init_keys(struct pad_dispatch *pad, struct evdev_device *device)
{
	unsigned int codes[] = {
		KEY_BUTTONCONFIG,
		KEY_ONSCREEN_KEYBOARD,
		KEY_CONTROLPANEL,
	};

	/* Wacom's keys are the only ones we know anything about */
	if (libevdev_get_id_vendor(device->evdev) != VENDOR_ID_WACOM)
		return;

	ARRAY_FOR_EACH(codes, code) {
		if (libevdev_has_event_code(device->evdev, EV_KEY, *code))
			map_set_key_map(pad->button_map[*code], *code);
	}
}

static void
pad_init_buttons(struct pad_dispatch *pad,
		 struct evdev_device *device)
{
	size_t i;

	for (i = 0; i < ARRAY_LENGTH(pad->button_map); i++)
		map_init(pad->button_map[i]);

	if (!pad_init_buttons_from_libwacom(pad, device))
		pad_init_buttons_from_kernel(pad, device);

	pad_init_keys(pad, device);
}

static void
pad_init_left_handed(struct evdev_device *device)
{
	if (evdev_tablet_has_left_handed(device))
		evdev_init_left_handed(device,
				       pad_change_to_left_handed);
}

static int
pad_init(struct pad_dispatch *pad, struct evdev_device *device)
{
	pad->base.dispatch_type = DISPATCH_TABLET_PAD;
	pad->base.interface = &pad_interface;
	pad->device = device;
	pad->status = PAD_NONE;
	pad->changed_axes = PAD_AXIS_NONE;

	pad_init_buttons(pad, device);
	pad_init_left_handed(device);
	if (pad_init_leds(pad, device) != 0)
		return 1;

	return 0;
}

static uint32_t
pad_sendevents_get_modes(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
}

static enum libinput_config_status
pad_sendevents_set_mode(struct libinput_device *device,
			enum libinput_config_send_events_mode mode)
{
	struct evdev_device *evdev = evdev_device(device);
	struct pad_dispatch *pad = (struct pad_dispatch*)evdev->dispatch;

	if (mode == pad->sendevents.current_mode)
		return LIBINPUT_CONFIG_STATUS_SUCCESS;

	switch(mode) {
	case LIBINPUT_CONFIG_SEND_EVENTS_ENABLED:
		break;
	case LIBINPUT_CONFIG_SEND_EVENTS_DISABLED:
		pad_suspend(evdev->dispatch, evdev);
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
	}

	pad->sendevents.current_mode = mode;

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

static enum libinput_config_send_events_mode
pad_sendevents_get_mode(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);
	struct pad_dispatch *dispatch = (struct pad_dispatch*)evdev->dispatch;

	return dispatch->sendevents.current_mode;
}

static enum libinput_config_send_events_mode
pad_sendevents_get_default_mode(struct libinput_device *device)
{
	return LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
}

struct evdev_dispatch *
evdev_tablet_pad_create(struct evdev_device *device)
{
	struct pad_dispatch *pad;

	pad = zalloc(sizeof *pad);

	if (pad_init(pad, device) != 0) {
		pad_destroy(&pad->base);
		return NULL;
	}

	device->base.config.sendevents = &pad->sendevents.config;
	pad->sendevents.current_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
	pad->sendevents.config.get_modes = pad_sendevents_get_modes;
	pad->sendevents.config.set_mode = pad_sendevents_set_mode;
	pad->sendevents.config.get_mode = pad_sendevents_get_mode;
	pad->sendevents.config.get_default_mode = pad_sendevents_get_default_mode;

	return &pad->base;
}

int
evdev_device_tablet_pad_has_key(struct evdev_device *device, uint32_t code)
{
	if (!(device->seat_caps & EVDEV_DEVICE_TABLET_PAD))
		return -1;

	return libevdev_has_event_code(device->evdev, EV_KEY, code);
}

int
evdev_device_tablet_pad_get_num_buttons(struct evdev_device *device)
{
	struct pad_dispatch *pad = (struct pad_dispatch*)device->dispatch;

	if (!(device->seat_caps & EVDEV_DEVICE_TABLET_PAD))
		return -1;

	return pad->nbuttons;
}

int
evdev_device_tablet_pad_get_num_rings(struct evdev_device *device)
{
	int nrings = 0;

	if (!(device->seat_caps & EVDEV_DEVICE_TABLET_PAD))
		return -1;

	if (libevdev_has_event_code(device->evdev, EV_ABS, ABS_WHEEL)) {
		nrings++;
		if (libevdev_has_event_code(device->evdev,
					    EV_ABS,
					    ABS_THROTTLE))
			nrings++;
	}

	return nrings;
}

int
evdev_device_tablet_pad_get_num_strips(struct evdev_device *device)
{
	int nstrips = 0;

	if (!(device->seat_caps & EVDEV_DEVICE_TABLET_PAD))
		return -1;

	if (libevdev_has_event_code(device->evdev, EV_ABS, ABS_RX)) {
		nstrips++;
		if (libevdev_has_event_code(device->evdev,
					    EV_ABS,
					    ABS_RY))
			nstrips++;
	}

	return nstrips;
}
