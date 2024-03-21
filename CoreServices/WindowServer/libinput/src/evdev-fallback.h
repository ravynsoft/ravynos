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

#ifndef EVDEV_FALLBACK_H
#define EVDEV_FALLBACK_H

#include "evdev.h"

enum debounce_state {
	DEBOUNCE_STATE_IS_UP = 100,
	DEBOUNCE_STATE_IS_DOWN,
	DEBOUNCE_STATE_IS_DOWN_WAITING,
	DEBOUNCE_STATE_IS_UP_DELAYING,
	DEBOUNCE_STATE_IS_UP_DELAYING_SPURIOUS,
	DEBOUNCE_STATE_IS_UP_DETECTING_SPURIOUS,
	DEBOUNCE_STATE_IS_DOWN_DETECTING_SPURIOUS,
	DEBOUNCE_STATE_IS_UP_WAITING,
	DEBOUNCE_STATE_IS_DOWN_DELAYING,

	DEBOUNCE_STATE_DISABLED = 999,
};

enum mt_slot_state {
	SLOT_STATE_NONE,
	SLOT_STATE_BEGIN,
	SLOT_STATE_UPDATE,
	SLOT_STATE_END,
};

enum palm_state {
	PALM_NONE,
	PALM_NEW,
	PALM_IS_PALM,
	PALM_WAS_PALM, /* this touch sequence was a palm but isn't now */
};

enum wheel_state {
	WHEEL_STATE_NONE,
	WHEEL_STATE_ACCUMULATING_SCROLL,
	WHEEL_STATE_SCROLLING,
};

enum wheel_direction {
	WHEEL_DIR_UNKNOW,
	WHEEL_DIR_VPOS,
	WHEEL_DIR_VNEG,
	WHEEL_DIR_HPOS,
	WHEEL_DIR_HNEG,
};

struct mt_slot {
	bool dirty;
	enum mt_slot_state state;
	int32_t seat_slot;
	struct device_coords point;
	struct device_coords hysteresis_center;
	enum palm_state palm_state;
};

struct fallback_dispatch {
	struct evdev_dispatch base;
	struct evdev_device *device;

	struct libinput_device_config_calibration calibration;

	struct {
		int angle;
		struct matrix matrix;
		struct libinput_device_config_rotation config;
	} rotation;

	struct {
		struct device_coords point;
		int32_t seat_slot;
	} abs;

	struct {
		int slot;
		struct mt_slot *slots;
		size_t slots_len;
		bool want_hysteresis;
		struct device_coords hysteresis_margin;
		bool has_palm;
	} mt;

	struct device_coords rel;

	struct {
		enum wheel_state state;
		struct device_coords lo_res;
		struct device_coords hi_res;
		bool emulate_hi_res_wheel;
		bool hi_res_event_received;
		struct libinput_timer scroll_timer;
		enum wheel_direction dir;
	} wheel;

	struct {
		/* The struct for the tablet mode switch device itself */
		struct {
			int state;
		} sw;
		/* The struct for other devices listening to the tablet mode
		   switch */
		struct {
			struct evdev_device *sw_device;
			struct libinput_event_listener listener;
		} other;
	} tablet_mode;

	/* Bitmask of pressed keys used to ignore initial release events from
	 * the kernel. */
	unsigned long hw_key_mask[NLONGS(KEY_CNT)];
	unsigned long last_hw_key_mask[NLONGS(KEY_CNT)];

	enum evdev_event_type pending_event;

	struct {
		unsigned int button_code;
		uint64_t button_time;
		struct libinput_timer timer;
		struct libinput_timer timer_short;
		enum debounce_state state;
		bool spurious_enabled;
	} debounce;

	struct {
		enum switch_reliability reliability;

		bool is_closed;
		bool is_closed_client_state;

		/* We allow multiple paired keyboards for the lid switch
		 * listener. Only one keyboard should exist, but that can
		 * have more than one event node. And it's a list because
		 * otherwise the test suite run fails too often.
		 */
		struct list paired_keyboard_list;
	} lid;

	/* pen/touch arbitration has a delayed state,
	 * in_arbitration is what decides when to filter.
	 */
	struct {
		enum evdev_arbitration_state state;
		bool in_arbitration;
		struct device_coord_rect rect;
		struct libinput_timer arbitration_timer;
	} arbitration;
};

static inline struct fallback_dispatch*
fallback_dispatch(struct evdev_dispatch *dispatch)
{
	evdev_verify_dispatch_type(dispatch, DISPATCH_FALLBACK);

	return container_of(dispatch, struct fallback_dispatch, base);
}

enum key_type {
	KEY_TYPE_NONE,
	KEY_TYPE_KEY,
	KEY_TYPE_BUTTON,
};

static inline enum key_type
get_key_type(uint16_t code)
{
	switch (code) {
	case BTN_TOOL_PEN:
	case BTN_TOOL_RUBBER:
	case BTN_TOOL_BRUSH:
	case BTN_TOOL_PENCIL:
	case BTN_TOOL_AIRBRUSH:
	case BTN_TOOL_MOUSE:
	case BTN_TOOL_LENS:
	case BTN_TOOL_QUINTTAP:
	case BTN_TOOL_DOUBLETAP:
	case BTN_TOOL_TRIPLETAP:
	case BTN_TOOL_QUADTAP:
	case BTN_TOOL_FINGER:
	case BTN_TOUCH:
		return KEY_TYPE_NONE;
	}

	if (code >= KEY_ESC && code <= KEY_MICMUTE)
		return KEY_TYPE_KEY;
	if (code >= BTN_MISC && code <= BTN_GEAR_UP)
		return KEY_TYPE_BUTTON;
	if (code >= KEY_OK && code <= KEY_LIGHTS_TOGGLE)
		return KEY_TYPE_KEY;
	if (code >= BTN_DPAD_UP && code <= BTN_DPAD_RIGHT)
		return KEY_TYPE_BUTTON;
	if (code >= KEY_ALS_TOGGLE && code < BTN_TRIGGER_HAPPY)
		return KEY_TYPE_KEY;
	if (code >= BTN_TRIGGER_HAPPY && code <= BTN_TRIGGER_HAPPY40)
		return KEY_TYPE_BUTTON;
	return KEY_TYPE_NONE;
}

static inline void
hw_set_key_down(struct fallback_dispatch *dispatch, int code, int pressed)
{
	long_set_bit_state(dispatch->hw_key_mask, code, pressed);
}

static inline bool
hw_key_has_changed(struct fallback_dispatch *dispatch, int code)
{
	return long_bit_is_set(dispatch->hw_key_mask, code) !=
		long_bit_is_set(dispatch->last_hw_key_mask, code);
}

static inline void
hw_key_update_last_state(struct fallback_dispatch *dispatch)
{
	static_assert(sizeof(dispatch->hw_key_mask) ==
		      sizeof(dispatch->last_hw_key_mask),
		      "Mismatching key mask size");

	memcpy(dispatch->last_hw_key_mask,
	       dispatch->hw_key_mask,
	       sizeof(dispatch->hw_key_mask));
}

static inline bool
hw_is_key_down(struct fallback_dispatch *dispatch, int code)
{
	return long_bit_is_set(dispatch->hw_key_mask, code);
}

static inline int
get_key_down_count(struct evdev_device *device, int code)
{
	return device->key_count[code];
}

void fallback_init_debounce(struct fallback_dispatch *dispatch);
void fallback_debounce_handle_state(struct fallback_dispatch *dispatch,
				    uint64_t time);
void
fallback_notify_physical_button(struct fallback_dispatch *dispatch,
				struct evdev_device *device,
				uint64_t time,
				int button,
				enum libinput_button_state state);

void
fallback_init_wheel(struct fallback_dispatch *dispatch,
		    struct evdev_device *device);

void
fallback_wheel_process_relative(struct fallback_dispatch *dispatch,
				struct evdev_device *device,
				struct input_event *e, uint64_t time);

void
fallback_wheel_handle_state(struct fallback_dispatch *dispatch,
			    struct evdev_device *device,
			    uint64_t time);

#endif
