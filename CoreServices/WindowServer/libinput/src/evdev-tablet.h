/*
 * Copyright © 2014 Red Hat, Inc.
 * Copyright © 2014 Lyude Paul
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

#ifndef EVDEV_TABLET_H
#define EVDEV_TABLET_H

#include "evdev.h"

#define LIBINPUT_TABLET_TOOL_AXIS_NONE 0
#define LIBINPUT_TOOL_NONE 0
#define LIBINPUT_TABLET_TOOL_TYPE_MAX LIBINPUT_TABLET_TOOL_TYPE_LENS

#define TABLET_HISTORY_LENGTH 4

enum tablet_status {
	TABLET_NONE			= 0,
	TABLET_AXES_UPDATED		= bit(0),
	TABLET_BUTTONS_PRESSED		= bit(1),
	TABLET_BUTTONS_DOWN		= bit(2),
	TABLET_BUTTONS_RELEASED		= bit(3),
	TABLET_TOOL_UPDATED		= bit(4),
	TABLET_TOOL_IN_CONTACT		= bit(5),
	TABLET_TOOL_LEAVING_PROXIMITY	= bit(6),
	TABLET_TOOL_OUT_OF_PROXIMITY	= bit(7),
	TABLET_TOOL_ENTERING_PROXIMITY	= bit(8),
	TABLET_TOOL_ENTERING_CONTACT	= bit(9),
	TABLET_TOOL_LEAVING_CONTACT	= bit(10),
	TABLET_TOOL_OUT_OF_RANGE	= bit(11),
};

struct button_state {
	unsigned char bits[NCHARS(KEY_CNT)];
};

struct tablet_dispatch {
	struct evdev_dispatch base;
	struct evdev_device *device;
	unsigned int status;
	unsigned char changed_axes[NCHARS(LIBINPUT_TABLET_TOOL_AXIS_MAX + 1)];
	struct tablet_axes axes; /* for assembling the current state */
	struct device_coords last_smooth_point;
	struct {
		unsigned int index;
		unsigned int count;
		struct tablet_axes samples[TABLET_HISTORY_LENGTH];
		size_t size;
	} history;

	unsigned char axis_caps[NCHARS(LIBINPUT_TABLET_TOOL_AXIS_MAX + 1)];
	int current_value[LIBINPUT_TABLET_TOOL_AXIS_MAX + 1];
	int prev_value[LIBINPUT_TABLET_TOOL_AXIS_MAX + 1];

	/* Only used for tablets that don't report serial numbers */
	struct list tool_list;

	struct button_state button_state;
	struct button_state prev_button_state;

	uint32_t tool_state;
	uint32_t prev_tool_state;

	struct {
		enum libinput_tablet_tool_type type;
		uint32_t id;
		uint32_t serial;
	} current_tool;

	uint32_t cursor_proximity_threshold;

	struct libinput_device_config_calibration calibration;

	/* The paired touch device on devices with both pen & touch */
	struct evdev_device *touch_device;
	enum evdev_arbitration_state arbitration;

	struct {
		/* The device locked for rotation */
		struct evdev_device *touch_device;
		/* Last known left-handed state of the touchpad */
		bool touch_device_left_handed_state;
		bool rotate;
		bool want_rotate;
	} rotation;

	struct {
		bool need_to_force_prox_out;
		struct libinput_timer prox_out_timer;
		bool proximity_out_forced;
		uint64_t last_event_time;

		/* true while injecting BTN_TOOL_PEN events */
		bool proximity_out_in_progress;
	} quirks;
};

static inline struct tablet_dispatch*
tablet_dispatch(struct evdev_dispatch *dispatch)
{
	evdev_verify_dispatch_type(dispatch, DISPATCH_TABLET);

	return container_of(dispatch, struct tablet_dispatch, base);
}

static inline enum libinput_tablet_tool_axis
evcode_to_axis(const uint32_t evcode)
{
	enum libinput_tablet_tool_axis axis;

	switch (evcode) {
	case ABS_X:
		axis = LIBINPUT_TABLET_TOOL_AXIS_X;
		break;
	case ABS_Y:
		axis = LIBINPUT_TABLET_TOOL_AXIS_Y;
		break;
	case ABS_Z:
		axis = LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z;
		break;
	case ABS_DISTANCE:
		axis = LIBINPUT_TABLET_TOOL_AXIS_DISTANCE;
		break;
	case ABS_PRESSURE:
		axis = LIBINPUT_TABLET_TOOL_AXIS_PRESSURE;
		break;
	case ABS_TILT_X:
		axis = LIBINPUT_TABLET_TOOL_AXIS_TILT_X;
		break;
	case ABS_TILT_Y:
		axis = LIBINPUT_TABLET_TOOL_AXIS_TILT_Y;
		break;
	case ABS_WHEEL:
		axis = LIBINPUT_TABLET_TOOL_AXIS_SLIDER;
		break;
	default:
		axis = LIBINPUT_TABLET_TOOL_AXIS_NONE;
		break;
	}

	return axis;
}

static inline enum libinput_tablet_tool_axis
rel_evcode_to_axis(const uint32_t evcode)
{
	enum libinput_tablet_tool_axis axis;

	switch (evcode) {
	case REL_WHEEL:
		axis = LIBINPUT_TABLET_TOOL_AXIS_REL_WHEEL;
		break;
	default:
		axis = LIBINPUT_TABLET_TOOL_AXIS_NONE;
		break;
	}

	return axis;
}

static inline uint32_t
axis_to_evcode(const enum libinput_tablet_tool_axis axis)
{
	uint32_t evcode;

	switch (axis) {
	case LIBINPUT_TABLET_TOOL_AXIS_X:
		evcode = ABS_X;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_Y:
		evcode = ABS_Y;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_DISTANCE:
		evcode = ABS_DISTANCE;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_PRESSURE:
		evcode = ABS_PRESSURE;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_TILT_X:
		evcode = ABS_TILT_X;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_TILT_Y:
		evcode = ABS_TILT_Y;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z:
		evcode = ABS_Z;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_SLIDER:
		evcode = ABS_WHEEL;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_SIZE_MAJOR:
		evcode = ABS_MT_TOUCH_MAJOR;
		break;
	case LIBINPUT_TABLET_TOOL_AXIS_SIZE_MINOR:
		evcode = ABS_MT_TOUCH_MINOR;
		break;
	default:
		abort();
	}

	return evcode;
}

static inline int
tablet_tool_to_evcode(enum libinput_tablet_tool_type type)
{
	int code;

	switch (type) {
	case LIBINPUT_TABLET_TOOL_TYPE_PEN:	  code = BTN_TOOL_PEN;		break;
	case LIBINPUT_TABLET_TOOL_TYPE_ERASER:	  code = BTN_TOOL_RUBBER;	break;
	case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:	  code = BTN_TOOL_BRUSH;	break;
	case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:	  code = BTN_TOOL_PENCIL;	break;
	case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:  code = BTN_TOOL_AIRBRUSH;	break;
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:	  code = BTN_TOOL_MOUSE;	break;
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:	  code = BTN_TOOL_LENS;		break;
	default:
		abort();
	}

	return code;
}

static inline const char *
tablet_tool_type_to_string(enum libinput_tablet_tool_type type)
{
	const char *str;

	switch (type) {
	case LIBINPUT_TABLET_TOOL_TYPE_PEN:	  str = "pen";		break;
	case LIBINPUT_TABLET_TOOL_TYPE_ERASER:	  str = "eraser";	break;
	case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:	  str = "brush";	break;
	case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:	  str = "pencil";	break;
	case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:  str = "airbrush";	break;
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:	  str = "mouse";	break;
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:	  str = "lens";		break;
	default:
		abort();
	}

	return str;
}

static inline struct libinput *
tablet_libinput_context(const struct tablet_dispatch *tablet)
{
	return evdev_libinput_context(tablet->device);
}

#endif
