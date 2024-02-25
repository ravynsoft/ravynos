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
#include "config.h"
#include "evdev-tablet.h"
#include "util-input-event.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#if HAVE_LIBWACOM
#include <libwacom/libwacom.h>
#endif

enum notify {
	DONT_NOTIFY,
	DO_NOTIFY,
};

/* The tablet sends events every ~2ms , 50ms should be plenty enough to
   detect out-of-range.
   This value is higher during test suite runs */
static int FORCED_PROXOUT_TIMEOUT = 50 * 1000; /* µs */

#define tablet_set_status(tablet_,s_) (tablet_)->status |= (s_)
#define tablet_unset_status(tablet_,s_) (tablet_)->status &= ~(s_)
#define tablet_has_status(tablet_,s_) (!!((tablet_)->status & (s_)))

static inline void
tablet_get_pressed_buttons(struct tablet_dispatch *tablet,
			   struct button_state *buttons)
{
	size_t i;
	const struct button_state *state = &tablet->button_state,
			          *prev_state = &tablet->prev_button_state;

	for (i = 0; i < sizeof(buttons->bits); i++)
		buttons->bits[i] = state->bits[i] & ~(prev_state->bits[i]);
}

static inline void
tablet_get_released_buttons(struct tablet_dispatch *tablet,
			    struct button_state *buttons)
{
	size_t i;
	const struct button_state *state = &tablet->button_state,
			          *prev_state = &tablet->prev_button_state;

	for (i = 0; i < sizeof(buttons->bits); i++)
		buttons->bits[i] = prev_state->bits[i] &
					~(state->bits[i]);
}

/* Merge the previous state with the current one so all buttons look like
 * they just got pressed in this frame */
static inline void
tablet_force_button_presses(struct tablet_dispatch *tablet)
{
	struct button_state *state = &tablet->button_state,
			    *prev_state = &tablet->prev_button_state;
	size_t i;

	for (i = 0; i < sizeof(state->bits); i++) {
		state->bits[i] = state->bits[i] | prev_state->bits[i];
		prev_state->bits[i] = 0;
	}
}

static inline size_t
tablet_history_size(const struct tablet_dispatch *tablet)
{
	return tablet->history.size;
}

static inline void
tablet_history_reset(struct tablet_dispatch *tablet)
{
	tablet->history.count = 0;
}

static inline void
tablet_history_push(struct tablet_dispatch *tablet,
		    const struct tablet_axes *axes)
{
	unsigned int index = (tablet->history.index + 1) %
				tablet_history_size(tablet);

	tablet->history.samples[index] = *axes;
	tablet->history.index = index;
	tablet->history.count = min(tablet->history.count + 1,
				    tablet_history_size(tablet));

	if (tablet->history.count < tablet_history_size(tablet))
		tablet_history_push(tablet, axes);
}

/**
 * Return a previous axis state, where index of 0 means "most recent", 1 is
 * "one before most recent", etc.
 */
static inline const struct tablet_axes*
tablet_history_get(const struct tablet_dispatch *tablet, unsigned int index)
{
	size_t sz = tablet_history_size(tablet);

	assert(index < sz);
	assert(index < tablet->history.count);

	index = (tablet->history.index + sz - index) % sz;
	return &tablet->history.samples[index];
}

static inline void
tablet_reset_changed_axes(struct tablet_dispatch *tablet)
{
	memset(tablet->changed_axes, 0, sizeof(tablet->changed_axes));
}

static bool
tablet_device_has_axis(struct tablet_dispatch *tablet,
		       enum libinput_tablet_tool_axis axis)
{
	struct libevdev *evdev = tablet->device->evdev;
	bool has_axis = false;
	unsigned int code;

	if (axis == LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z) {
		has_axis = (libevdev_has_event_code(evdev,
						    EV_KEY,
						    BTN_TOOL_MOUSE) &&
			    libevdev_has_event_code(evdev,
						    EV_ABS,
						    ABS_TILT_X) &&
			    libevdev_has_event_code(evdev,
						    EV_ABS,
						    ABS_TILT_Y));
		code = axis_to_evcode(axis);
		has_axis |= libevdev_has_event_code(evdev,
						    EV_ABS,
						    code);
	} else if (axis == LIBINPUT_TABLET_TOOL_AXIS_REL_WHEEL) {
		has_axis = libevdev_has_event_code(evdev,
						   EV_REL,
						   REL_WHEEL);
	} else {
		code = axis_to_evcode(axis);
		has_axis = libevdev_has_event_code(evdev,
						   EV_ABS,
						   code);
	}

	return has_axis;
}

static inline bool
tablet_filter_axis_fuzz(const struct tablet_dispatch *tablet,
			const struct evdev_device *device,
			const struct input_event *e,
			enum libinput_tablet_tool_axis axis)
{
	int delta, fuzz;
	int current, previous;

	previous = tablet->prev_value[axis];
	current = e->value;
	delta = previous - current;

	fuzz = libevdev_get_abs_fuzz(device->evdev, e->code);

	/* ABS_DISTANCE doesn't have have fuzz set and causes continuous
	 * updates for the cursor/lens tools. Add a minimum fuzz of 2, same
	 * as the xf86-input-wacom driver
	 */
	switch (e->code) {
	case ABS_DISTANCE:
		fuzz = max(2, fuzz);
		break;
	default:
		break;
	}

	return abs(delta) <= fuzz;
}

static void
tablet_process_absolute(struct tablet_dispatch *tablet,
			struct evdev_device *device,
			struct input_event *e,
			uint64_t time)
{
	enum libinput_tablet_tool_axis axis;

	switch (e->code) {
	case ABS_X:
	case ABS_Y:
	case ABS_Z:
	case ABS_PRESSURE:
	case ABS_TILT_X:
	case ABS_TILT_Y:
	case ABS_DISTANCE:
	case ABS_WHEEL:
		axis = evcode_to_axis(e->code);
		if (axis == LIBINPUT_TABLET_TOOL_AXIS_NONE) {
			evdev_log_bug_libinput(device,
					       "Invalid ABS event code %#x\n",
					       e->code);
			break;
		}

		tablet->prev_value[axis] = tablet->current_value[axis];
		if (tablet_filter_axis_fuzz(tablet, device, e, axis))
			break;

		tablet->current_value[axis] = e->value;
		set_bit(tablet->changed_axes, axis);
		tablet_set_status(tablet, TABLET_AXES_UPDATED);
		break;
	/* tool_id is the identifier for the tool we can use in libwacom
	 * to identify it (if we have one anyway) */
	case ABS_MISC:
		tablet->current_tool.id = e->value;
		break;
	/* Intuos 3 strip data. Should only happen on the Pad device, not on
	   the Pen device. */
	case ABS_RX:
	case ABS_RY:
	/* Only on the 4D mouse (Intuos2), obsolete */
	case ABS_RZ:
	/* Only on the 4D mouse (Intuos2), obsolete.
	   The 24HD sends ABS_THROTTLE on the Pad device for the second
	   wheel but we shouldn't get here on kernel >= 3.17.
	   */
	case ABS_THROTTLE:
	default:
		evdev_log_info(device,
			       "Unhandled ABS event code %#x\n",
			       e->code);
		break;
	}
}

static void
tablet_apply_rotation(struct evdev_device *device)
{
	struct tablet_dispatch *tablet = tablet_dispatch(device->dispatch);

	if (tablet->rotation.rotate == tablet->rotation.want_rotate)
		return;

	if (!tablet_has_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY))
		return;

	tablet->rotation.rotate = tablet->rotation.want_rotate;

	evdev_log_debug(device,
			"tablet-rotation: rotation is %s\n",
			tablet->rotation.rotate ? "on" : "off");
}

static void
tablet_change_rotation(struct evdev_device *device, enum notify notify)
{
	struct tablet_dispatch *tablet = tablet_dispatch(device->dispatch);
	struct evdev_device *touch_device = tablet->touch_device;
	struct evdev_dispatch *dispatch;
	bool tablet_is_left, touchpad_is_left;

	tablet_is_left = tablet->device->left_handed.enabled;
	touchpad_is_left = tablet->rotation.touch_device_left_handed_state;

	tablet->rotation.want_rotate = tablet_is_left || touchpad_is_left;
	tablet_apply_rotation(device);

	if (notify == DO_NOTIFY && touch_device) {
		bool enable = device->left_handed.want_enabled;

		dispatch = touch_device->dispatch;
		if (dispatch->interface->left_handed_toggle)
			dispatch->interface->left_handed_toggle(dispatch,
								touch_device,
								enable);
	}
}

static void
tablet_change_to_left_handed(struct evdev_device *device)
{
	if (device->left_handed.enabled == device->left_handed.want_enabled)
		return;

	device->left_handed.enabled = device->left_handed.want_enabled;

	tablet_change_rotation(device, DO_NOTIFY);
}

static void
tablet_update_tool(struct tablet_dispatch *tablet,
		   struct evdev_device *device,
		   enum libinput_tablet_tool_type tool,
		   bool enabled)
{
	assert(tool != LIBINPUT_TOOL_NONE);

	if (enabled) {
		tablet->current_tool.type = tool;
		tablet_set_status(tablet, TABLET_TOOL_ENTERING_PROXIMITY);
		tablet_unset_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY);
	}
	else if (!tablet_has_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY)) {
		tablet_set_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY);
	}
}

static inline double
normalize_slider(const struct input_absinfo *absinfo)
{
	double value = (absinfo->value - absinfo->minimum) / absinfo_range(absinfo);

	return value * 2 - 1;
}

static inline double
normalize_distance(const struct input_absinfo *absinfo)
{
	double value = (absinfo->value - absinfo->minimum) / absinfo_range(absinfo);

	return value;
}

static inline double
normalize_pressure(const struct input_absinfo *absinfo,
		   struct libinput_tablet_tool *tool)
{
	/**
	 * Note: the upper threshold takes the offset into account so that
	 *            |- 4% -|
	 * min |------X------X-------------------------| max
	 *            |      |
	 *            |      + upper threshold / tip trigger
	 *            +- offset and lower threshold
	 *
	 * The axis is scaled into the range [lower, max] so that the lower
	 * threshold is 0 pressure.
	 */
	int base = tool->pressure.threshold.lower;
	double range = absinfo->maximum - base + 1;
	double value = (absinfo->value - base) / range;

	return max(0.0, value);
}

static inline double
adjust_tilt(const struct input_absinfo *absinfo)
{
	double value = (absinfo->value - absinfo->minimum) / absinfo_range(absinfo);
	const int WACOM_MAX_DEGREES = 64;

	/* If resolution is nonzero, it's in units/radian. But require
	 * a min/max less/greater than zero so we can assume 0 is the
	 * center */
	if (absinfo->resolution != 0 &&
	    absinfo->maximum > 0 &&
	    absinfo->minimum < 0) {
		value = 180.0/M_PI * absinfo->value/absinfo->resolution;
	} else {
		/* Wacom supports physical [-64, 64] degrees, so map to that by
		 * default. If other tablets have a different physical range or
		 * nonzero physical offsets, they need extra treatment
		 * here.
		 */
		/* Map to the (-1, 1) range */
		value = (value * 2) - 1;
		value *= WACOM_MAX_DEGREES;
	}

	return value;
}

static inline int32_t
invert_axis(const struct input_absinfo *absinfo)
{
	return absinfo->maximum - (absinfo->value - absinfo->minimum);
}

static void
convert_tilt_to_rotation(struct tablet_dispatch *tablet)
{
	const int offset = 5;
	double x, y;
	double angle = 0.0;

	/* Wacom Intuos 4, 5, Pro mouse calculates rotation from the x/y tilt
	   values. The device has a 175 degree CCW hardware offset but since we use
	   atan2 the effective offset is just 5 degrees.
	   */
	x = tablet->axes.tilt.x;
	y = tablet->axes.tilt.y;

	/* atan2 is CCW, we want CW -> negate x */
	if (x || y)
		angle = ((180.0 * atan2(-x, y)) / M_PI);

	angle = fmod(360 + angle - offset, 360);

	tablet->axes.rotation = angle;
	set_bit(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
}

static double
convert_to_degrees(const struct input_absinfo *absinfo, double offset)
{
	/* range is [0, 360[, i.e. range + 1 */
	double value = (absinfo->value - absinfo->minimum) / absinfo_range(absinfo);

	return fmod(value * 360.0 + offset, 360.0);
}

static inline double
normalize_wheel(struct tablet_dispatch *tablet,
		int value)
{
	struct evdev_device *device = tablet->device;

	return value * device->scroll.wheel_click_angle.x;
}

static inline void
tablet_update_xy(struct tablet_dispatch *tablet,
		 struct evdev_device *device)
{
	const struct input_absinfo *absinfo;
	int value;

	if (!libevdev_has_event_code(device->evdev, EV_ABS, ABS_X) ||
	    !libevdev_has_event_code(device->evdev, EV_ABS, ABS_Y))
		return;

	if (bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_X) ||
	    bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_Y)) {
		absinfo = libevdev_get_abs_info(device->evdev, ABS_X);

		if (tablet->rotation.rotate)
			value = invert_axis(absinfo);
		else
			value = absinfo->value;

		tablet->axes.point.x = value;

		absinfo = libevdev_get_abs_info(device->evdev, ABS_Y);

		if (tablet->rotation.rotate)
			value = invert_axis(absinfo);
		else
			value = absinfo->value;

		tablet->axes.point.y = value;

		evdev_transform_absolute(device, &tablet->axes.point);
	}
}

static inline struct normalized_coords
tablet_tool_process_delta(struct tablet_dispatch *tablet,
			  struct libinput_tablet_tool *tool,
			  const struct evdev_device *device,
			  struct tablet_axes *axes,
			  uint64_t time)
{
	const struct normalized_coords zero = { 0.0, 0.0 };
	struct device_coords delta = { 0, 0 };
	struct device_float_coords accel;

	/* When tool contact changes, we probably got a cursor jump. Don't
	   try to calculate a delta for that event */
	if (!tablet_has_status(tablet,
			       TABLET_TOOL_ENTERING_PROXIMITY) &&
	    !tablet_has_status(tablet, TABLET_TOOL_ENTERING_CONTACT) &&
	    !tablet_has_status(tablet, TABLET_TOOL_LEAVING_CONTACT) &&
	    (bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_X) ||
	     bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_Y))) {
		delta.x = axes->point.x - tablet->last_smooth_point.x;
		delta.y = axes->point.y - tablet->last_smooth_point.y;
	}

	if (axes->point.x != tablet->last_smooth_point.x)
		set_bit(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_X);
	if (axes->point.y != tablet->last_smooth_point.y)
		set_bit(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_Y);

	tablet->last_smooth_point = axes->point;

	accel.x = 1.0 * delta.x;
	accel.y = 1.0 * delta.y;

	if (device_float_is_zero(accel))
		return zero;

	return filter_dispatch(device->pointer.filter,
			       &accel,
			       tool,
			       time);
}

static inline void
tablet_update_pressure(struct tablet_dispatch *tablet,
		       struct evdev_device *device,
		       struct libinput_tablet_tool *tool)
{
	const struct input_absinfo *absinfo;

	if (!libevdev_has_event_code(device->evdev, EV_ABS, ABS_PRESSURE))
		return;

	if (bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_PRESSURE)) {
		absinfo = libevdev_get_abs_info(device->evdev, ABS_PRESSURE);
		tablet->axes.pressure = normalize_pressure(absinfo, tool);
	}
}

static inline void
tablet_update_distance(struct tablet_dispatch *tablet,
		       struct evdev_device *device)
{
	const struct input_absinfo *absinfo;

	if (!libevdev_has_event_code(device->evdev, EV_ABS, ABS_DISTANCE))
		return;

	if (bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_DISTANCE)) {
		absinfo = libevdev_get_abs_info(device->evdev, ABS_DISTANCE);
		tablet->axes.distance = normalize_distance(absinfo);
	}
}

static inline void
tablet_update_slider(struct tablet_dispatch *tablet,
		     struct evdev_device *device)
{
	const struct input_absinfo *absinfo;

	if (!libevdev_has_event_code(device->evdev, EV_ABS, ABS_WHEEL))
		return;

	if (bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_SLIDER)) {
		absinfo = libevdev_get_abs_info(device->evdev, ABS_WHEEL);
		tablet->axes.slider = normalize_slider(absinfo);
	}
}

static inline void
tablet_update_tilt(struct tablet_dispatch *tablet,
		   struct evdev_device *device)
{
	const struct input_absinfo *absinfo;

	if (!libevdev_has_event_code(device->evdev, EV_ABS, ABS_TILT_X) ||
	    !libevdev_has_event_code(device->evdev, EV_ABS, ABS_TILT_Y))
		return;

	/* mouse rotation resets tilt to 0 so always fetch both axes if
	 * either has changed */
	if (bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_TILT_X) ||
	    bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_TILT_Y)) {

		absinfo = libevdev_get_abs_info(device->evdev, ABS_TILT_X);
		tablet->axes.tilt.x = adjust_tilt(absinfo);

		absinfo = libevdev_get_abs_info(device->evdev, ABS_TILT_Y);
		tablet->axes.tilt.y = adjust_tilt(absinfo);

		if (device->left_handed.enabled) {
			tablet->axes.tilt.x *= -1;
			tablet->axes.tilt.y *= -1;
		}
	}
}

static inline void
tablet_update_artpen_rotation(struct tablet_dispatch *tablet,
			      struct evdev_device *device)
{
	const struct input_absinfo *absinfo;

	if (!libevdev_has_event_code(device->evdev, EV_ABS, ABS_Z))
		return;

	if (bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z)) {
		absinfo = libevdev_get_abs_info(device->evdev,
						ABS_Z);
		/* artpen has 0 with buttons pointing east */
		tablet->axes.rotation = convert_to_degrees(absinfo, 90);
	}
}

static inline void
tablet_update_mouse_rotation(struct tablet_dispatch *tablet,
			     struct evdev_device *device)
{
	if (bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_TILT_X) ||
	    bit_is_set(tablet->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_TILT_Y)) {
		convert_tilt_to_rotation(tablet);
	}
}

static inline void
tablet_update_rotation(struct tablet_dispatch *tablet,
		       struct evdev_device *device)
{
	/* We must check ROTATION_Z after TILT_X/Y so that the tilt axes are
	 * already normalized and set if we have the mouse/lens tool */
	if (tablet->current_tool.type == LIBINPUT_TABLET_TOOL_TYPE_MOUSE ||
	    tablet->current_tool.type == LIBINPUT_TABLET_TOOL_TYPE_LENS) {
		tablet_update_mouse_rotation(tablet, device);
		clear_bit(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_TILT_X);
		clear_bit(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_TILT_Y);
		tablet->axes.tilt.x = 0;
		tablet->axes.tilt.y = 0;

		/* tilt is already converted to left-handed, so mouse
		 * rotation is converted to left-handed automatically */
	} else {

		tablet_update_artpen_rotation(tablet, device);

		if (device->left_handed.enabled) {
			double r = tablet->axes.rotation;
			tablet->axes.rotation = fmod(180 + r, 360);
		}
	}
}

static inline void
tablet_update_wheel(struct tablet_dispatch *tablet,
		    struct evdev_device *device)
{
	int a;

	a = LIBINPUT_TABLET_TOOL_AXIS_REL_WHEEL;
	if (bit_is_set(tablet->changed_axes, a)) {
		/* tablet->axes.wheel_discrete is already set */
		tablet->axes.wheel = normalize_wheel(tablet,
						     tablet->axes.wheel_discrete);
	} else {
		tablet->axes.wheel = 0;
		tablet->axes.wheel_discrete = 0;
	}
}

static void
tablet_smoothen_axes(const struct tablet_dispatch *tablet,
		     struct tablet_axes *axes)
{
	size_t i;
	size_t count = tablet_history_size(tablet);
	struct tablet_axes smooth = { 0 };

	for (i = 0; i < count; i++) {
		const struct tablet_axes *a = tablet_history_get(tablet, i);

		smooth.point.x += a->point.x;
		smooth.point.y += a->point.y;

		smooth.tilt.x += a->tilt.x;
		smooth.tilt.y += a->tilt.y;
	}

	axes->point.x = smooth.point.x/count;
	axes->point.y = smooth.point.y/count;

	axes->tilt.x = smooth.tilt.x/count;
	axes->tilt.y = smooth.tilt.y/count;
}

static bool
tablet_check_notify_axes(struct tablet_dispatch *tablet,
			 struct evdev_device *device,
			 struct libinput_tablet_tool *tool,
			 struct tablet_axes *axes_out,
			 uint64_t time)
{
	struct tablet_axes axes = {0};
	const char tmp[sizeof(tablet->changed_axes)] = {0};
	bool rc = false;

	if (memcmp(tmp, tablet->changed_axes, sizeof(tmp)) == 0) {
		axes = tablet->axes;
		goto out;
	}

	tablet_update_xy(tablet, device);
	tablet_update_pressure(tablet, device, tool);
	tablet_update_distance(tablet, device);
	tablet_update_slider(tablet, device);
	tablet_update_tilt(tablet, device);
	tablet_update_wheel(tablet, device);
	/* We must check ROTATION_Z after TILT_X/Y so that the tilt axes are
	 * already normalized and set if we have the mouse/lens tool */
	tablet_update_rotation(tablet, device);

	axes.point = tablet->axes.point;
	axes.pressure = tablet->axes.pressure;
	axes.distance = tablet->axes.distance;
	axes.slider = tablet->axes.slider;
	axes.tilt = tablet->axes.tilt;
	axes.wheel = tablet->axes.wheel;
	axes.wheel_discrete = tablet->axes.wheel_discrete;
	axes.rotation = tablet->axes.rotation;

	rc = true;

out:
	/* The tool position often jumps to a different spot when contact changes.
	 * If tool contact changes, clear the history to prevent axis smoothing
	 * from trying to average over the spatial discontinuity. */
	if (tablet_has_status(tablet, TABLET_TOOL_ENTERING_CONTACT) ||
	    tablet_has_status(tablet, TABLET_TOOL_LEAVING_CONTACT)) {
		tablet_history_reset(tablet);
	}

	tablet_history_push(tablet, &tablet->axes);
	tablet_smoothen_axes(tablet, &axes);

	/* The delta relies on the last *smooth* point, so we do it last */
	axes.delta = tablet_tool_process_delta(tablet, tool, device, &axes, time);

	*axes_out = axes;

	return rc;
}

static void
tablet_update_button(struct tablet_dispatch *tablet,
		     uint32_t evcode,
		     uint32_t enable)
{
	switch (evcode) {
	case BTN_LEFT:
	case BTN_RIGHT:
	case BTN_MIDDLE:
	case BTN_SIDE:
	case BTN_EXTRA:
	case BTN_FORWARD:
	case BTN_BACK:
	case BTN_TASK:
	case BTN_STYLUS:
	case BTN_STYLUS2:
		break;
	default:
		evdev_log_info(tablet->device,
			       "Unhandled button %s (%#x)\n",
			       libevdev_event_code_get_name(EV_KEY, evcode),
			       evcode);
		return;
	}

	if (enable) {
		set_bit(tablet->button_state.bits, evcode);
		tablet_set_status(tablet, TABLET_BUTTONS_PRESSED);
	} else {
		clear_bit(tablet->button_state.bits, evcode);
		tablet_set_status(tablet, TABLET_BUTTONS_RELEASED);
	}
}

static inline enum libinput_tablet_tool_type
tablet_evcode_to_tool(int code)
{
	enum libinput_tablet_tool_type type;

	switch (code) {
	case BTN_TOOL_PEN:	type = LIBINPUT_TABLET_TOOL_TYPE_PEN;		break;
	case BTN_TOOL_RUBBER:	type = LIBINPUT_TABLET_TOOL_TYPE_ERASER;	break;
	case BTN_TOOL_BRUSH:	type = LIBINPUT_TABLET_TOOL_TYPE_BRUSH;		break;
	case BTN_TOOL_PENCIL:	type = LIBINPUT_TABLET_TOOL_TYPE_PENCIL;	break;
	case BTN_TOOL_AIRBRUSH:	type = LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH;	break;
	case BTN_TOOL_MOUSE:	type = LIBINPUT_TABLET_TOOL_TYPE_MOUSE;		break;
	case BTN_TOOL_LENS:	type = LIBINPUT_TABLET_TOOL_TYPE_LENS;		break;
	default:
		abort();
	}

	return type;
}

static void
tablet_process_key(struct tablet_dispatch *tablet,
		   struct evdev_device *device,
		   struct input_event *e,
		   uint64_t time)
{
	enum libinput_tablet_tool_type type;

	/* ignore kernel key repeat */
	if (e->value == 2)
		return;

	switch (e->code) {
	case BTN_TOOL_FINGER:
		evdev_log_bug_libinput(device,
			       "Invalid tool 'finger' on tablet interface\n");
		break;
	case BTN_TOOL_PEN:
	case BTN_TOOL_RUBBER:
	case BTN_TOOL_BRUSH:
	case BTN_TOOL_PENCIL:
	case BTN_TOOL_AIRBRUSH:
	case BTN_TOOL_MOUSE:
	case BTN_TOOL_LENS:
		type = tablet_evcode_to_tool(e->code);
		tablet_set_status(tablet, TABLET_TOOL_UPDATED);
		if (e->value)
			tablet->tool_state |= bit(type);
		else
			tablet->tool_state &= ~bit(type);
		break;
	case BTN_TOUCH:
		if (!bit_is_set(tablet->axis_caps,
				LIBINPUT_TABLET_TOOL_AXIS_PRESSURE)) {
			if (e->value)
				tablet_set_status(tablet,
						  TABLET_TOOL_ENTERING_CONTACT);
			else
				tablet_set_status(tablet,
						  TABLET_TOOL_LEAVING_CONTACT);
		}
		break;
	default:
		tablet_update_button(tablet, e->code, e->value);
		break;
	}
}

static void
tablet_process_relative(struct tablet_dispatch *tablet,
			struct evdev_device *device,
			struct input_event *e,
			uint64_t time)
{
	enum libinput_tablet_tool_axis axis;

	switch (e->code) {
	case REL_WHEEL:
		axis = rel_evcode_to_axis(e->code);
		if (axis == LIBINPUT_TABLET_TOOL_AXIS_NONE) {
			evdev_log_bug_libinput(device,
					       "Invalid ABS event code %#x\n",
					       e->code);
			break;
		}
		set_bit(tablet->changed_axes, axis);
		tablet->axes.wheel_discrete = -1 * e->value;
		tablet_set_status(tablet, TABLET_AXES_UPDATED);
		break;
	default:
		evdev_log_info(device,
			       "Unhandled relative axis %s (%#x)\n",
			       libevdev_event_code_get_name(EV_REL, e->code),
			       e->code);
		return;
	}
}

static void
tablet_process_misc(struct tablet_dispatch *tablet,
		    struct evdev_device *device,
		    struct input_event *e,
		    uint64_t time)
{
	switch (e->code) {
	case MSC_SERIAL:
		if (e->value != -1)
			tablet->current_tool.serial = e->value;

		break;
	case MSC_SCAN:
		break;
	default:
		evdev_log_info(device,
			       "Unhandled MSC event code %s (%#x)\n",
			       libevdev_event_code_get_name(EV_MSC, e->code),
			       e->code);
		break;
	}
}

static inline void
copy_axis_cap(const struct tablet_dispatch *tablet,
	      struct libinput_tablet_tool *tool,
	      enum libinput_tablet_tool_axis axis)
{
	if (bit_is_set(tablet->axis_caps, axis))
		set_bit(tool->axis_caps, axis);
}

static inline void
copy_button_cap(const struct tablet_dispatch *tablet,
		struct libinput_tablet_tool *tool,
		uint32_t button)
{
	struct libevdev *evdev = tablet->device->evdev;
	if (libevdev_has_event_code(evdev, EV_KEY, button))
		set_bit(tool->buttons, button);
}

#if HAVE_LIBWACOM
static inline int
tool_set_bits_from_libwacom(const struct tablet_dispatch *tablet,
			    struct libinput_tablet_tool *tool)
{
	int rc = 1;
	WacomDeviceDatabase *db;
	const WacomStylus *s = NULL;
	int code;
	WacomStylusType type;
	WacomAxisTypeFlags axes;

	db = tablet_libinput_context(tablet)->libwacom.db;
	if (!db)
		return rc;

	s = libwacom_stylus_get_for_id(db, tool->tool_id);
	if (!s)
		return rc;

	type = libwacom_stylus_get_type(s);
	if (type == WSTYLUS_PUCK) {
		for (code = BTN_LEFT;
		     code < BTN_LEFT + libwacom_stylus_get_num_buttons(s);
		     code++)
			copy_button_cap(tablet, tool, code);
	} else {
		if (libwacom_stylus_get_num_buttons(s) >= 2)
			copy_button_cap(tablet, tool, BTN_STYLUS2);
		if (libwacom_stylus_get_num_buttons(s) >= 1)
			copy_button_cap(tablet, tool, BTN_STYLUS);
	}

	if (libwacom_stylus_has_wheel(s))
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_REL_WHEEL);

	axes = libwacom_stylus_get_axes(s);

	if (axes & WACOM_AXIS_TYPE_TILT) {
		/* tilt on the puck is converted to rotation */
		if (type == WSTYLUS_PUCK) {
			set_bit(tool->axis_caps,
				LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
		} else {
			copy_axis_cap(tablet,
				      tool,
				      LIBINPUT_TABLET_TOOL_AXIS_TILT_X);
			copy_axis_cap(tablet,
				      tool,
				      LIBINPUT_TABLET_TOOL_AXIS_TILT_Y);
		}
	}
	if (axes & WACOM_AXIS_TYPE_ROTATION_Z)
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
	if (axes & WACOM_AXIS_TYPE_DISTANCE)
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_DISTANCE);
	if (axes & WACOM_AXIS_TYPE_SLIDER)
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_SLIDER);
	if (axes & WACOM_AXIS_TYPE_PRESSURE)
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_PRESSURE);

	rc = 0;

	return rc;
}
#endif

static void
tool_set_bits(const struct tablet_dispatch *tablet,
	      struct libinput_tablet_tool *tool)
{
	enum libinput_tablet_tool_type type = tool->type;

	copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_X);
	copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_Y);

#if HAVE_LIBWACOM
	if (tool_set_bits_from_libwacom(tablet, tool) == 0)
		return;
#endif
	/* If we don't have libwacom, we simply copy any axis we have on the
	   tablet onto the tool. Except we know that mice only have rotation
	   anyway.
	 */
	switch (type) {
	case LIBINPUT_TABLET_TOOL_TYPE_PEN:
	case LIBINPUT_TABLET_TOOL_TYPE_ERASER:
	case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:
	case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:
	case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_PRESSURE);
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_DISTANCE);
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_TILT_X);
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_TILT_Y);
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_SLIDER);

		/* Rotation is special, it can be either ABS_Z or
		 * BTN_TOOL_MOUSE+ABS_TILT_X/Y. Aiptek tablets have
		 * mouse+tilt (and thus rotation), but they do not have
		 * ABS_Z. So let's not copy the axis bit if we don't have
		 * ABS_Z, otherwise we try to get the value from it later on
		 * proximity in and go boom because the absinfo isn't there.
		 */
		if (libevdev_has_event_code(tablet->device->evdev, EV_ABS,
					    ABS_Z))
			copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
		copy_axis_cap(tablet, tool, LIBINPUT_TABLET_TOOL_AXIS_REL_WHEEL);
		break;
	default:
		break;
	}

	/* If we don't have libwacom, copy all pen-related buttons from the
	   tablet vs all mouse-related buttons */
	switch (type) {
	case LIBINPUT_TABLET_TOOL_TYPE_PEN:
	case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:
	case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:
	case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:
	case LIBINPUT_TABLET_TOOL_TYPE_ERASER:
		copy_button_cap(tablet, tool, BTN_STYLUS);
		copy_button_cap(tablet, tool, BTN_STYLUS2);
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:
		copy_button_cap(tablet, tool, BTN_LEFT);
		copy_button_cap(tablet, tool, BTN_MIDDLE);
		copy_button_cap(tablet, tool, BTN_RIGHT);
		copy_button_cap(tablet, tool, BTN_SIDE);
		copy_button_cap(tablet, tool, BTN_EXTRA);
		break;
	default:
		break;
	}
}

static inline int
axis_range_percentage(const struct input_absinfo *a, double percent)
{
	return absinfo_range(a) * percent/100.0 + a->minimum;
}

static inline void
tool_set_pressure_thresholds(struct tablet_dispatch *tablet,
			     struct libinput_tablet_tool *tool)
{
	struct evdev_device *device = tablet->device;
	const struct input_absinfo *pressure, *distance;
	struct quirks_context *quirks = NULL;
	struct quirks *q = NULL;
	struct quirk_range r;
	int lo = 0, hi = 1;

	tool->pressure.offset = 0;
	tool->pressure.has_offset = false;

	pressure = libevdev_get_abs_info(device->evdev, ABS_PRESSURE);
	if (!pressure)
		goto out;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);

	distance = libevdev_get_abs_info(device->evdev, ABS_DISTANCE);
	if (distance) {
		tool->pressure.offset = pressure->minimum;
		tool->pressure.heuristic_state = PRESSURE_HEURISTIC_STATE_DONE;
	} else {
		tool->pressure.offset = pressure->maximum;
		tool->pressure.heuristic_state = PRESSURE_HEURISTIC_STATE_PROXIN1;
	}

	/* 5 and 1% of the pressure range */
	hi = axis_range_percentage(pressure, 5);
	lo = axis_range_percentage(pressure, 1);

	if (q && quirks_get_range(q, QUIRK_ATTR_PRESSURE_RANGE, &r)) {
		if (r.lower >= r.upper) {
			evdev_log_info(device,
				       "Invalid pressure range, using defaults\n");
		} else {
			hi = r.upper;
			lo = r.lower;
		}
	}
out:
	tool->pressure.threshold.upper = hi;
	tool->pressure.threshold.lower = lo;

	quirks_unref(q);
}

static struct libinput_tablet_tool *
tablet_get_tool(struct tablet_dispatch *tablet,
		enum libinput_tablet_tool_type type,
		uint32_t tool_id,
		uint32_t serial)
{
	struct libinput *libinput = tablet_libinput_context(tablet);
	struct libinput_tablet_tool *tool = NULL, *t;
	struct list *tool_list;

	if (serial) {
		tool_list = &libinput->tool_list;
		/* Check if we already have the tool in our list of tools */
		list_for_each(t, tool_list, link) {
			if (type == t->type && serial == t->serial) {
				tool = t;
				break;
			}
		}
	}

	/* If we get a tool with a delayed serial number, we already created
	 * a 0-serial number tool for it earlier. Re-use that, even though
	 * it means we can't distinguish this tool from others.
	 * https://bugs.freedesktop.org/show_bug.cgi?id=97526
	 */
	if (!tool) {
		tool_list = &tablet->tool_list;
		/* We can't guarantee that tools without serial numbers are
		 * unique, so we keep them local to the tablet that they come
		 * into proximity of instead of storing them in the global tool
		 * list
		 * Same as above, but don't bother checking the serial number
		 */
		list_for_each(t, tool_list, link) {
			if (type == t->type) {
				tool = t;
				break;
			}
		}

		/* Didn't find the tool but we have a serial. Switch
		 * tool_list back so we create in the correct list */
		if (!tool && serial)
			tool_list = &libinput->tool_list;
	}

	/* If we didn't already have the new_tool in our list of tools,
	 * add it */
	if (!tool) {
		tool = zalloc(sizeof *tool);

		*tool = (struct libinput_tablet_tool) {
			.type = type,
			.serial = serial,
			.tool_id = tool_id,
			.refcount = 1,
		};

		tool_set_pressure_thresholds(tablet, tool);
		tool_set_bits(tablet, tool);

		list_insert(tool_list, &tool->link);
	}

	return tool;
}

static void
tablet_notify_button_mask(struct tablet_dispatch *tablet,
			  struct evdev_device *device,
			  uint64_t time,
			  struct libinput_tablet_tool *tool,
			  const struct button_state *buttons,
			  enum libinput_button_state state)
{
	struct libinput_device *base = &device->base;
	size_t i;
	size_t nbits = 8 * sizeof(buttons->bits);
	enum libinput_tablet_tool_tip_state tip_state;

	if (tablet_has_status(tablet, TABLET_TOOL_IN_CONTACT))
		tip_state = LIBINPUT_TABLET_TOOL_TIP_DOWN;
	else
		tip_state = LIBINPUT_TABLET_TOOL_TIP_UP;

	for (i = 0; i < nbits; i++) {
		if (!bit_is_set(buttons->bits, i))
			continue;

		tablet_notify_button(base,
				     time,
				     tool,
				     tip_state,
				     &tablet->axes,
				     i,
				     state);
	}
}

static void
tablet_notify_buttons(struct tablet_dispatch *tablet,
		      struct evdev_device *device,
		      uint64_t time,
		      struct libinput_tablet_tool *tool,
		      enum libinput_button_state state)
{
	struct button_state buttons;

	if (state == LIBINPUT_BUTTON_STATE_PRESSED)
		tablet_get_pressed_buttons(tablet, &buttons);
	else
		tablet_get_released_buttons(tablet, &buttons);

	tablet_notify_button_mask(tablet,
				  device,
				  time,
				  tool,
				  &buttons,
				  state);
}

static void
sanitize_pressure_distance(struct tablet_dispatch *tablet,
			   struct libinput_tablet_tool *tool)
{
	bool tool_in_contact;
	const struct input_absinfo *distance,
	                           *pressure;

	distance = libevdev_get_abs_info(tablet->device->evdev, ABS_DISTANCE);
	pressure = libevdev_get_abs_info(tablet->device->evdev, ABS_PRESSURE);

	if (!pressure || !distance)
		return;

	bool pressure_changed = bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_PRESSURE);
	bool distance_changed = bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_DISTANCE);

	if (!pressure_changed && !distance_changed)
		return;

	/* Note: this is an arbitrary "in contact" decision rather than "tip
	 * down". We use the lower threshold as minimum pressure value,
	 * anything less than that gets filtered away */
	tool_in_contact = (pressure->value > tool->pressure.threshold.lower);

	/* Keep distance and pressure mutually exclusive */
	if (distance &&
	    distance->value > distance->minimum &&
	    pressure->value > pressure->minimum) {
		if (tool_in_contact) {
			clear_bit(tablet->changed_axes,
				  LIBINPUT_TABLET_TOOL_AXIS_DISTANCE);
			tablet->axes.distance = 0;
		} else {
			clear_bit(tablet->changed_axes,
				  LIBINPUT_TABLET_TOOL_AXIS_PRESSURE);
			tablet->axes.pressure = 0;
		}
	} else if (pressure_changed && !tool_in_contact) {
		/* Make sure that the last axis value sent to the caller is a 0 */
		if (tablet->axes.pressure == 0)
			clear_bit(tablet->changed_axes,
				  LIBINPUT_TABLET_TOOL_AXIS_PRESSURE);
		else
			tablet->axes.pressure = 0;
	}
}

static inline void
sanitize_mouse_lens_rotation(struct tablet_dispatch *tablet)
{
	/* If we have a mouse/lens cursor and the tilt changed, the rotation
	   changed. Mark this, calculate the angle later */
	if ((tablet->current_tool.type == LIBINPUT_TABLET_TOOL_TYPE_MOUSE ||
	    tablet->current_tool.type == LIBINPUT_TABLET_TOOL_TYPE_LENS) &&
	    (bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_TILT_X) ||
	     bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_TILT_Y)))
		set_bit(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
}

static void
sanitize_tablet_axes(struct tablet_dispatch *tablet,
		     struct libinput_tablet_tool *tool)
{
	sanitize_pressure_distance(tablet, tool);
	sanitize_mouse_lens_rotation(tablet);
}

static void
set_pressure_offset(struct libinput_tablet_tool *tool, int offset)
{
	tool->pressure.offset = offset;
	tool->pressure.has_offset = true;

	/* Adjust the tresholds accordingly - we use the same gap (4% in
	 * device coordinates) between upper and lower as before which isn't
	 * technically correct (our range shrunk) but it's easy to calculate.
	 */
	int gap = tool->pressure.threshold.upper - tool->pressure.threshold.lower;
	tool->pressure.threshold.lower = offset;
	tool->pressure.threshold.upper = offset + gap;
}

static void
update_pressure_offset(struct tablet_dispatch *tablet,
		       struct evdev_device *device,
		       struct libinput_tablet_tool *tool)
{
	const struct input_absinfo *pressure =
		libevdev_get_abs_info(device->evdev, ABS_PRESSURE);

	if (!pressure ||
	    !bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_PRESSURE))
		return;

	/* If we have an event that falls below the current offset, adjust
	 * the offset downwards. A fast contact can start with a
	 * higher-than-needed pressure offset and then we'd be tied into a
	 * high pressure offset for the rest of the session.
	 *
	 * If we are still pending the offset decision, only update the observed
	 * offset value, don't actually set it to have an offset.
	 */
	int offset = pressure->value;
	if (tool->pressure.has_offset) {
		if (offset < tool->pressure.offset)
			set_pressure_offset(tool, offset);
	} else if (tool->pressure.heuristic_state != PRESSURE_HEURISTIC_STATE_DONE) {
		tool->pressure.offset = min(offset, tool->pressure.offset);
	}
}

static void
detect_pressure_offset(struct tablet_dispatch *tablet,
		       struct evdev_device *device,
		       struct libinput_tablet_tool *tool)
{
	const struct input_absinfo *pressure, *distance;
	int offset;

	if (tool->pressure.has_offset ||
	    !bit_is_set(tablet->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_PRESSURE))
		return;

	pressure = libevdev_get_abs_info(device->evdev, ABS_PRESSURE);
	distance = libevdev_get_abs_info(device->evdev, ABS_DISTANCE);

	if (!pressure)
		return;

	offset = pressure->value;
	if (offset <= pressure->minimum)
		return;

	if (distance) {
		/* If we're closer than 50% of the distance axis, skip pressure
		 * offset detection, too likely to be wrong */
		if (distance->value < axis_range_percentage(distance, 50))
			return;
	} else {
                /* A device without distance will always have some pressure on
                 * contact. Offset detection is delayed for a few proximity ins
                 * in the hope we'll find the minimum value until then. That
                 * offset is updated during motion events so by the time the
                 * deciding prox-in arrives we should know the minimum offset.
                 */
                if (offset > pressure->minimum)
			tool->pressure.offset = min(offset, tool->pressure.offset);

		switch (tool->pressure.heuristic_state) {
		case PRESSURE_HEURISTIC_STATE_PROXIN1:
		case PRESSURE_HEURISTIC_STATE_PROXIN2:
			tool->pressure.heuristic_state++;
			return;
		case PRESSURE_HEURISTIC_STATE_DECIDE:
			tool->pressure.heuristic_state++;
			offset = tool->pressure.offset;
			break;
		case PRESSURE_HEURISTIC_STATE_DONE:
			return;
		}
	}

	if (offset <= pressure->minimum)
		return;

	if (offset > axis_range_percentage(pressure, 50)) {
		evdev_log_error(device,
			 "Ignoring pressure offset greater than 50%% detected on tool %s (serial %#x). "
			 "See %s/tablet-support.html\n",
			 tablet_tool_type_to_string(tool->type),
			 tool->serial,
			 HTTP_DOC_LINK);
		return;
	}

	evdev_log_info(device,
		 "Pressure offset detected on tool %s (serial %#x).  "
		 "See %s/tablet-support.html\n",
		 tablet_tool_type_to_string(tool->type),
		 tool->serial,
		 HTTP_DOC_LINK);

	set_pressure_offset(tool, offset);
}

static void
detect_tool_contact(struct tablet_dispatch *tablet,
		    struct evdev_device *device,
		    struct libinput_tablet_tool *tool)
{
	const struct input_absinfo *p;
	int pressure;

	if (!bit_is_set(tool->axis_caps, LIBINPUT_TABLET_TOOL_AXIS_PRESSURE))
		return;

	/* if we have pressure, always use that for contact, not BTN_TOUCH */
	if (tablet_has_status(tablet, TABLET_TOOL_ENTERING_CONTACT))
		evdev_log_bug_libinput(device,
				       "Invalid status: entering contact\n");
	if (tablet_has_status(tablet, TABLET_TOOL_LEAVING_CONTACT) &&
	    !tablet_has_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY))
		evdev_log_bug_libinput(device,
				       "Invalid status: leaving contact\n");

	p = libevdev_get_abs_info(tablet->device->evdev, ABS_PRESSURE);
	if (!p) {
		evdev_log_bug_libinput(device,
				       "Missing pressure axis\n");
		return;
	}
	pressure = p->value;

	if (pressure <= tool->pressure.threshold.lower &&
	    tablet_has_status(tablet, TABLET_TOOL_IN_CONTACT)) {
		tablet_set_status(tablet, TABLET_TOOL_LEAVING_CONTACT);
	} else if (pressure >= tool->pressure.threshold.upper &&
		   !tablet_has_status(tablet, TABLET_TOOL_IN_CONTACT)) {
		tablet_set_status(tablet, TABLET_TOOL_ENTERING_CONTACT);
	}
}

static void
tablet_mark_all_axes_changed(struct tablet_dispatch *tablet,
			     struct libinput_tablet_tool *tool)
{
	static_assert(sizeof(tablet->changed_axes) ==
			      sizeof(tool->axis_caps),
		      "Mismatching array sizes");

	memcpy(tablet->changed_axes,
	       tool->axis_caps,
	       sizeof(tablet->changed_axes));
}

static void
tablet_update_proximity_state(struct tablet_dispatch *tablet,
			      struct evdev_device *device,
			      struct libinput_tablet_tool *tool)
{
	const struct input_absinfo *distance;
	int dist_max = tablet->cursor_proximity_threshold;
	int dist;

	distance = libevdev_get_abs_info(tablet->device->evdev, ABS_DISTANCE);
	if (!distance)
		return;

	dist = distance->value;
	if (dist == 0)
		return;

	/* Tool got into permitted range */
	if (dist < dist_max &&
	    (tablet_has_status(tablet, TABLET_TOOL_OUT_OF_RANGE) ||
	     tablet_has_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY))) {
		tablet_unset_status(tablet,
				    TABLET_TOOL_OUT_OF_RANGE);
		tablet_unset_status(tablet,
				    TABLET_TOOL_OUT_OF_PROXIMITY);
		tablet_set_status(tablet, TABLET_TOOL_ENTERING_PROXIMITY);
		tablet_mark_all_axes_changed(tablet, tool);

		tablet_set_status(tablet, TABLET_BUTTONS_PRESSED);
		tablet_force_button_presses(tablet);
		return;
	}

	if (dist < dist_max)
		return;

	/* Still out of range/proximity */
	if (tablet_has_status(tablet, TABLET_TOOL_OUT_OF_RANGE) ||
	    tablet_has_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY))
	    return;

	/* Tool entered prox but is outside of permitted range */
	if (tablet_has_status(tablet,
			      TABLET_TOOL_ENTERING_PROXIMITY)) {
		tablet_set_status(tablet,
				  TABLET_TOOL_OUT_OF_RANGE);
		tablet_unset_status(tablet,
				    TABLET_TOOL_ENTERING_PROXIMITY);
		return;
	}

	/* Tool was in prox and is now outside of range. Set leaving
	 * proximity, on the next event it will be OUT_OF_PROXIMITY and thus
	 * caught by the above conditions */
	tablet_set_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY);
}

static struct phys_rect
tablet_calculate_arbitration_rect(struct tablet_dispatch *tablet)
{
	struct evdev_device *device = tablet->device;
	struct phys_rect r = {0};
	struct phys_coords mm;

	mm = evdev_device_units_to_mm(device, &tablet->axes.point);

	/* The rect we disable is 20mm left of the tip, 100mm north of the
	 * tip, and 200x250mm large.
	 * If the stylus is tilted left (tip further right than the eraser
	 * end) assume left-handed mode.
	 *
	 * Obviously if we'd run out of the boundaries, we clip the rect
	 * accordingly.
	 */
	if (tablet->axes.tilt.x > 0) {
		r.x = mm.x - 20;
		r.w = 200;
	} else {
		r.x = mm.x + 20;
		r.w = 200;
		r.x -= r.w;
	}

	if (r.x < 0) {
		r.w += r.x;
		r.x = 0;
	}

	r.y = mm.y - 100;
	r.h = 250;
	if (r.y < 0) {
		r.h += r.y;
		r.y = 0;
	}

	return r;
}

static inline void
tablet_update_touch_device_rect(struct tablet_dispatch *tablet,
				const struct tablet_axes *axes,
				uint64_t time)
{
	struct evdev_dispatch *dispatch;
	struct phys_rect rect = {0};

	if (tablet->touch_device == NULL ||
	    tablet->arbitration != ARBITRATION_IGNORE_RECT)
		return;

	rect = tablet_calculate_arbitration_rect(tablet);

	dispatch = tablet->touch_device->dispatch;
	if (dispatch->interface->touch_arbitration_update_rect)
		dispatch->interface->touch_arbitration_update_rect(dispatch,
								   tablet->touch_device,
								   &rect,
								   time);
}

static inline bool
tablet_send_proximity_in(struct tablet_dispatch *tablet,
			 struct libinput_tablet_tool *tool,
			 struct evdev_device *device,
			 struct tablet_axes *axes,
			 uint64_t time)
{
	if (!tablet_has_status(tablet, TABLET_TOOL_ENTERING_PROXIMITY))
		return false;

	tablet_notify_proximity(&device->base,
				time,
				tool,
				LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN,
				tablet->changed_axes,
				axes);
	tablet_unset_status(tablet, TABLET_TOOL_ENTERING_PROXIMITY);
	tablet_unset_status(tablet, TABLET_AXES_UPDATED);

	tablet_reset_changed_axes(tablet);
	axes->delta.x = 0;
	axes->delta.y = 0;

	return true;
}

static inline bool
tablet_send_proximity_out(struct tablet_dispatch *tablet,
			 struct libinput_tablet_tool *tool,
			 struct evdev_device *device,
			 struct tablet_axes *axes,
			 uint64_t time)
{
	if (!tablet_has_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY))
		return false;

	tablet_notify_proximity(&device->base,
				time,
				tool,
				LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT,
				tablet->changed_axes,
				axes);

	tablet_set_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY);
	tablet_unset_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY);

	tablet_reset_changed_axes(tablet);
	axes->delta.x = 0;
	axes->delta.y = 0;

	return true;
}

static inline bool
tablet_send_tip(struct tablet_dispatch *tablet,
		struct libinput_tablet_tool *tool,
		struct evdev_device *device,
		struct tablet_axes *axes,
		uint64_t time)
{
	if (tablet_has_status(tablet, TABLET_TOOL_ENTERING_CONTACT)) {
		tablet_notify_tip(&device->base,
				  time,
				  tool,
				  LIBINPUT_TABLET_TOOL_TIP_DOWN,
				  tablet->changed_axes,
				  axes);
		tablet_unset_status(tablet, TABLET_AXES_UPDATED);
		tablet_unset_status(tablet, TABLET_TOOL_ENTERING_CONTACT);
		tablet_set_status(tablet, TABLET_TOOL_IN_CONTACT);

		tablet_reset_changed_axes(tablet);
		axes->delta.x = 0;
		axes->delta.y = 0;

		return true;
	}

	if (tablet_has_status(tablet, TABLET_TOOL_LEAVING_CONTACT)) {
		tablet_notify_tip(&device->base,
				  time,
				  tool,
				  LIBINPUT_TABLET_TOOL_TIP_UP,
				  tablet->changed_axes,
				  axes);
		tablet_unset_status(tablet, TABLET_AXES_UPDATED);
		tablet_unset_status(tablet, TABLET_TOOL_LEAVING_CONTACT);
		tablet_unset_status(tablet, TABLET_TOOL_IN_CONTACT);

		tablet_reset_changed_axes(tablet);
		axes->delta.x = 0;
		axes->delta.y = 0;

		return true;
	}

	return false;
}

static inline void
tablet_send_axes(struct tablet_dispatch *tablet,
		 struct libinput_tablet_tool *tool,
		 struct evdev_device *device,
		 struct tablet_axes *axes,
		 uint64_t time)
{
	enum libinput_tablet_tool_tip_state tip_state;

	if (!tablet_has_status(tablet, TABLET_AXES_UPDATED))
		return;

	if (tablet_has_status(tablet,
			      TABLET_TOOL_IN_CONTACT))
		tip_state = LIBINPUT_TABLET_TOOL_TIP_DOWN;
	else
		tip_state = LIBINPUT_TABLET_TOOL_TIP_UP;

	tablet_notify_axis(&device->base,
			   time,
			   tool,
			   tip_state,
			   tablet->changed_axes,
			   axes);
	tablet_unset_status(tablet, TABLET_AXES_UPDATED);
	tablet_reset_changed_axes(tablet);
	axes->delta.x = 0;
	axes->delta.y = 0;
}

static inline void
tablet_send_buttons(struct tablet_dispatch *tablet,
		    struct libinput_tablet_tool *tool,
		    struct evdev_device *device,
		    uint64_t time)
{
	if (tablet_has_status(tablet, TABLET_BUTTONS_RELEASED)) {
		tablet_notify_buttons(tablet,
				      device,
				      time,
				      tool,
				      LIBINPUT_BUTTON_STATE_RELEASED);
		tablet_unset_status(tablet, TABLET_BUTTONS_RELEASED);
	}

	if (tablet_has_status(tablet, TABLET_BUTTONS_PRESSED)) {
		tablet_notify_buttons(tablet,
				      device,
				      time,
				      tool,
				      LIBINPUT_BUTTON_STATE_PRESSED);
		tablet_unset_status(tablet, TABLET_BUTTONS_PRESSED);
	}
}

static void
tablet_send_events(struct tablet_dispatch *tablet,
		   struct libinput_tablet_tool *tool,
		   struct evdev_device *device,
		   uint64_t time)
{
	struct tablet_axes axes = {0};

	if (tablet_has_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY)) {
		/* Tool is leaving proximity, we can't rely on the last axis
		 * information (it'll be mostly 0), so we just get the
		 * current state and skip over updating the axes.
		 */
		axes = tablet->axes;

		/* Don't send an axis event, but we may have a tip event
		 * update */
		tablet_unset_status(tablet, TABLET_AXES_UPDATED);
	} else {
		if (tablet_check_notify_axes(tablet, device, tool, &axes, time))
			tablet_update_touch_device_rect(tablet, &axes, time);
	}

	assert(tablet->axes.delta.x == 0);
	assert(tablet->axes.delta.y == 0);

	tablet_send_proximity_in(tablet, tool, device, &axes, time);
	if (!tablet_send_tip(tablet, tool, device, &axes, time))
		tablet_send_axes(tablet, tool, device, &axes, time);

	tablet_unset_status(tablet, TABLET_TOOL_ENTERING_CONTACT);
	tablet_reset_changed_axes(tablet);

	tablet_send_buttons(tablet, tool, device, time);

	if (tablet_send_proximity_out(tablet, tool, device, &axes, time)) {
		tablet_change_to_left_handed(device);
		tablet_apply_rotation(device);
		tablet_history_reset(tablet);
	}
}

/**
 * Handling for the proximity out workaround. Some tablets only send
 * BTN_TOOL_PEN on the very first event, then leave it set even when the pen
 * leaves the detectable range. To libinput this looks like we always have
 * the pen in proximity.
 *
 * To avoid this, we set a timer on BTN_TOOL_PEN in. We expect the tablet to
 * continuously send events, and while it's doing so we keep updating the
 * timer. Once we go Xms without an event we assume proximity out and inject
 * a BTN_TOOL_PEN event into the sequence through the timer func.
 *
 * We need to remember that we did that, on the first event after the
 * timeout we need to emulate a BTN_TOOL_PEN event again to force proximity
 * in.
 *
 * Other tools never send the BTN_TOOL_PEN event. For those tools, we
 * piggyback along with the proximity out quirks by injecting
 * the event during the first event frame.
 */
static inline void
tablet_proximity_out_quirk_set_timer(struct tablet_dispatch *tablet,
				     uint64_t time)
{
	if (tablet->quirks.need_to_force_prox_out)
		libinput_timer_set(&tablet->quirks.prox_out_timer,
				   time + FORCED_PROXOUT_TIMEOUT);
}

static bool
tablet_update_tool_state(struct tablet_dispatch *tablet,
			 struct evdev_device *device,
			 uint64_t time)
{
	enum libinput_tablet_tool_type type;
	uint32_t changed;
	int state;
	uint32_t doubled_up_new_tool_bit = 0;

	/* we were already out of proximity but now got a tool update but
	 * our tool state is zero - i.e. we got a valid prox out from the
	 * device.
	 */
	if (tablet->quirks.proximity_out_forced &&
	    tablet_has_status(tablet, TABLET_TOOL_UPDATED) &&
	    !tablet->tool_state) {
		tablet->quirks.need_to_force_prox_out = false;
		tablet->quirks.proximity_out_forced = false;
	}
	/* We need to emulate a BTN_TOOL_PEN if we get an axis event (i.e.
	 * stylus is def. in proximity) and:
	 * - we forced a proximity out before, or
	 * - on the very first event after init, because if we didn't get a
	 *   BTN_TOOL_PEN and the state for the tool was 0, this device will
	 *   never send the event.
	 * We don't do this for pure button events because we discard those.
	 *
	 * But: on some devices the proximity out is delayed by the kernel,
	 * so we get it after our forced prox-out has triggered. In that
	 * case we need to just ignore the change.
	 */
	if (tablet_has_status(tablet, TABLET_AXES_UPDATED)) {
		if (tablet->quirks.proximity_out_forced) {
			if (!tablet_has_status(tablet, TABLET_TOOL_UPDATED)  &&
			    !tablet->tool_state)
				tablet->tool_state = bit(LIBINPUT_TABLET_TOOL_TYPE_PEN);
			tablet->quirks.proximity_out_forced = false;
		} else if (tablet->tool_state == 0 &&
			    tablet->current_tool.type == LIBINPUT_TOOL_NONE) {
			tablet->tool_state = bit(LIBINPUT_TABLET_TOOL_TYPE_PEN);
			tablet->quirks.proximity_out_forced = false;
		}
	}

	if (tablet->tool_state == tablet->prev_tool_state)
		return false;

	/* Kernel tools are supposed to be mutually exclusive, but we may have
	 * two bits set due to firmware/kernel bugs.
	 * Two cases that have been seen in the wild:
	 * - BTN_TOOL_PEN on proximity in, followed by
	 *   BTN_TOOL_RUBBER later, see #259
	 *   -> We force a prox-out of the pen, trigger prox-in for eraser
	 * - BTN_TOOL_RUBBER on proximity in, but BTN_TOOL_PEN when
	 *   the tip is down, see #702.
	 *   -> We ignore BTN_TOOL_PEN
	 * In both cases the eraser is what we want, so we bias
	 * towards that.
	 */
	if (tablet->tool_state & (tablet->tool_state - 1)) {
		doubled_up_new_tool_bit = tablet->tool_state ^ tablet->prev_tool_state;

		/* The new tool is the pen. Ignore it */
		if (doubled_up_new_tool_bit == bit(LIBINPUT_TABLET_TOOL_TYPE_PEN)) {
			tablet->tool_state &= ~bit(LIBINPUT_TABLET_TOOL_TYPE_PEN);
			return false;
		}

		/* The new tool is some tool other than pen (usually eraser).
		 * We set the current tool state to zero, thus setting
		 * everything up for a prox out on the tool. Once that is set
		 * up, we change the tool state to be the new one we just got.
		 * When we re-process this function we now get the new tool
		 * as prox in. Importantly, we basically rely on nothing else
		 * happening in the meantime.
		 */
		tablet->tool_state = 0;
	}

	changed = tablet->tool_state ^ tablet->prev_tool_state;
	type = ffs(changed) - 1;
	state = !!(tablet->tool_state & bit(type));

	tablet_update_tool(tablet, device, type, state);

	/* The proximity timeout is only needed for BTN_TOOL_PEN, devices
	 * that require it don't do erasers */
	if (type == LIBINPUT_TABLET_TOOL_TYPE_PEN) {
		if (state) {
			tablet_proximity_out_quirk_set_timer(tablet, time);
		} else {
			/* If we get a BTN_TOOL_PEN 0 when *not* injecting
			 * events it means the tablet will give us the right
			 * events after all and we can disable our
			 * timer-based proximity out.
			 */
			if (!tablet->quirks.proximity_out_in_progress)
				tablet->quirks.need_to_force_prox_out = false;

			libinput_timer_cancel(&tablet->quirks.prox_out_timer);
		}
	}

	tablet->prev_tool_state = tablet->tool_state;

	if (doubled_up_new_tool_bit) {
		tablet->tool_state = doubled_up_new_tool_bit;
		return true; /* need to re-process */
	}
	return false;
}

static struct libinput_tablet_tool *
tablet_get_current_tool(struct tablet_dispatch *tablet)
{
	if (tablet->current_tool.type == LIBINPUT_TOOL_NONE)
		return NULL;

	return tablet_get_tool(tablet,
			       tablet->current_tool.type,
			       tablet->current_tool.id,
			       tablet->current_tool.serial);
}

static void
tablet_flush(struct tablet_dispatch *tablet,
	     struct evdev_device *device,
	     uint64_t time)
{
	struct libinput_tablet_tool *tool;
	bool process_tool_twice;

reprocess:
	process_tool_twice = tablet_update_tool_state(tablet, device, time);

	tool = tablet_get_current_tool(tablet);
	if (!tool)
		return; /* OOM */

	if (tool->type == LIBINPUT_TABLET_TOOL_TYPE_MOUSE ||
	    tool->type == LIBINPUT_TABLET_TOOL_TYPE_LENS)
		tablet_update_proximity_state(tablet, device, tool);

	if (tablet_has_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY) ||
	    tablet_has_status(tablet, TABLET_TOOL_OUT_OF_RANGE))
		return;

	if (tablet_has_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY)) {
		/* Release all stylus buttons */
		memset(tablet->button_state.bits,
		       0,
		       sizeof(tablet->button_state.bits));
		tablet_set_status(tablet, TABLET_BUTTONS_RELEASED);
		if (tablet_has_status(tablet, TABLET_TOOL_IN_CONTACT))
			tablet_set_status(tablet, TABLET_TOOL_LEAVING_CONTACT);
	} else if (tablet_has_status(tablet, TABLET_TOOL_ENTERING_PROXIMITY)) {
		tablet_mark_all_axes_changed(tablet, tool);
		update_pressure_offset(tablet, device, tool);
		detect_pressure_offset(tablet, device, tool);
		detect_tool_contact(tablet, device, tool);
		sanitize_tablet_axes(tablet, tool);
	} else if (tablet_has_status(tablet, TABLET_AXES_UPDATED)) {
		update_pressure_offset(tablet, device, tool);
		detect_tool_contact(tablet, device, tool);
		sanitize_tablet_axes(tablet, tool);
	}

	tablet_send_events(tablet, tool, device, time);

	if (process_tool_twice)
		goto reprocess;
}

static inline void
tablet_set_touch_device_enabled(struct tablet_dispatch *tablet,
				enum evdev_arbitration_state which,
				const struct phys_rect *rect,
				uint64_t time)
{
	struct evdev_device *touch_device = tablet->touch_device;
	struct evdev_dispatch *dispatch;

	if (touch_device == NULL)
		return;

	tablet->arbitration = which;

	dispatch = touch_device->dispatch;
	if (dispatch->interface->touch_arbitration_toggle)
		dispatch->interface->touch_arbitration_toggle(dispatch,
							      touch_device,
							      which,
							      rect,
							      time);
}

static inline void
tablet_toggle_touch_device(struct tablet_dispatch *tablet,
			   struct evdev_device *tablet_device,
			   uint64_t time)
{
	enum evdev_arbitration_state which;
	struct phys_rect r = {0};
	struct phys_rect *rect = NULL;

	if (tablet_has_status(tablet,
			      TABLET_TOOL_OUT_OF_RANGE) ||
	    tablet_has_status(tablet, TABLET_NONE) ||
	    tablet_has_status(tablet,
			      TABLET_TOOL_LEAVING_PROXIMITY) ||
	    tablet_has_status(tablet,
			      TABLET_TOOL_OUT_OF_PROXIMITY)) {
		which = ARBITRATION_NOT_ACTIVE;
	} else if (tablet->axes.tilt.x == 0) {
		which = ARBITRATION_IGNORE_ALL;
	} else if (tablet->arbitration != ARBITRATION_IGNORE_RECT) {
		/* This enables rect-based arbitration, updates are sent
		 * elsewhere */
		r = tablet_calculate_arbitration_rect(tablet);
		rect = &r;
		which = ARBITRATION_IGNORE_RECT;
	} else {
		return;
	}

	tablet_set_touch_device_enabled(tablet,
					which,
					rect,
					time);
}

static inline void
tablet_reset_state(struct tablet_dispatch *tablet)
{
	struct button_state zero = {0};

	/* Update state */
	memcpy(&tablet->prev_button_state,
	       &tablet->button_state,
	       sizeof(tablet->button_state));
	tablet_unset_status(tablet, TABLET_TOOL_UPDATED);

	if (memcmp(&tablet->button_state, &zero, sizeof(zero)) == 0)
		tablet_unset_status(tablet, TABLET_BUTTONS_DOWN);
	else
		tablet_set_status(tablet, TABLET_BUTTONS_DOWN);
}

static void
tablet_proximity_out_quirk_timer_func(uint64_t now, void *data)
{
	struct tablet_dispatch *tablet = data;
	struct timeval tv = us2tv(now);
	struct input_event events[2] = {
		{ .input_event_sec = tv.tv_sec,
		  .input_event_usec = tv.tv_usec,
		  .type = EV_KEY,
		  .code = BTN_TOOL_PEN,
		  .value = 0 },
		{ .input_event_sec = tv.tv_sec,
		  .input_event_usec = tv.tv_usec,
		  .type = EV_SYN,
		  .code = SYN_REPORT,
		  .value = 0 },
	};

	if (tablet_has_status(tablet, TABLET_TOOL_IN_CONTACT) ||
	    tablet_has_status(tablet, TABLET_BUTTONS_DOWN)) {
		tablet_proximity_out_quirk_set_timer(tablet, now);
		return;
	}

	if (tablet->quirks.last_event_time > now - FORCED_PROXOUT_TIMEOUT) {
		tablet_proximity_out_quirk_set_timer(tablet,
						     tablet->quirks.last_event_time);
		return;
	}

	evdev_log_debug(tablet->device, "tablet: forcing proximity after timeout\n");

	tablet->quirks.proximity_out_in_progress = true;
	ARRAY_FOR_EACH(events, e) {
		tablet->base.interface->process(&tablet->base,
						 tablet->device,
						 e,
						 now);
	}
	tablet->quirks.proximity_out_in_progress = false;

	tablet->quirks.proximity_out_forced = true;
}

static void
tablet_process(struct evdev_dispatch *dispatch,
	       struct evdev_device *device,
	       struct input_event *e,
	       uint64_t time)
{
	struct tablet_dispatch *tablet = tablet_dispatch(dispatch);

	switch (e->type) {
	case EV_ABS:
		tablet_process_absolute(tablet, device, e, time);
		break;
	case EV_REL:
		tablet_process_relative(tablet, device, e, time);
		break;
	case EV_KEY:
		tablet_process_key(tablet, device, e, time);
		break;
	case EV_MSC:
		tablet_process_misc(tablet, device, e, time);
		break;
	case EV_SYN:
		tablet_flush(tablet, device, time);
		tablet_toggle_touch_device(tablet, device, time);
		tablet_reset_state(tablet);
		tablet->quirks.last_event_time = time;
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
tablet_suspend(struct evdev_dispatch *dispatch,
	       struct evdev_device *device)
{
	struct tablet_dispatch *tablet = tablet_dispatch(dispatch);
	struct libinput *li = tablet_libinput_context(tablet);
	uint64_t now = libinput_now(li);

	tablet_set_touch_device_enabled(tablet,
					ARBITRATION_NOT_ACTIVE,
					NULL,
					now);

	if (!tablet_has_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY)) {
		tablet_set_status(tablet, TABLET_TOOL_LEAVING_PROXIMITY);
		tablet_flush(tablet, device, libinput_now(li));
	}
}

static void
tablet_destroy(struct evdev_dispatch *dispatch)
{
	struct tablet_dispatch *tablet = tablet_dispatch(dispatch);
	struct libinput_tablet_tool *tool;
	struct libinput *li = tablet_libinput_context(tablet);

	libinput_timer_cancel(&tablet->quirks.prox_out_timer);
	libinput_timer_destroy(&tablet->quirks.prox_out_timer);

	list_for_each_safe(tool, &tablet->tool_list, link) {
		libinput_tablet_tool_unref(tool);
	}

	libinput_libwacom_unref(li);

	free(tablet);
}

static void
tablet_setup_touch_arbitration(struct evdev_device *device,
			       struct evdev_device *new_device)
{
	struct tablet_dispatch *tablet = tablet_dispatch(device->dispatch);

        /* We enable touch arbitration with the first touch screen/external
         * touchpad we see. This may be wrong in some cases, so we have some
         * heuristics in case we find a "better" device.
         */
        if (tablet->touch_device != NULL) {
		struct libinput_device_group *group1 = libinput_device_get_device_group(&device->base);
		struct libinput_device_group *group2 = libinput_device_get_device_group(&new_device->base);

		/* same phsical device? -> better, otherwise keep the one we have */
		if (group1 != group2)
			return;

		/* We found a better device, let's swap it out */
		struct libinput *li = tablet_libinput_context(tablet);
		tablet_set_touch_device_enabled(tablet,
						ARBITRATION_NOT_ACTIVE,
						NULL,
						libinput_now(li));
		evdev_log_debug(device,
				"touch-arbitration: removing pairing for %s<->%s\n",
				device->devname,
				tablet->touch_device->devname);
	}

	evdev_log_debug(device,
			"touch-arbitration: activated for %s<->%s\n",
			device->devname,
			new_device->devname);
	tablet->touch_device = new_device;
}

static void
tablet_setup_rotation(struct evdev_device *device,
		      struct evdev_device *new_device)
{
	struct tablet_dispatch *tablet = tablet_dispatch(device->dispatch);
	struct libinput_device_group *group1 = libinput_device_get_device_group(&device->base);
	struct libinput_device_group *group2 = libinput_device_get_device_group(&new_device->base);

	if (tablet->rotation.touch_device == NULL && (group1 == group2)) {
		evdev_log_debug(device,
				"tablet-rotation: %s will rotate %s\n",
				device->devname,
				new_device->devname);
		tablet->rotation.touch_device = new_device;

		if (libinput_device_config_left_handed_get(&new_device->base)) {
			tablet->rotation.touch_device_left_handed_state = true;
			tablet_change_rotation(device, DO_NOTIFY);
		}
	}
}

static void
tablet_device_added(struct evdev_device *device,
		    struct evdev_device *added_device)
{
	bool is_touchscreen, is_ext_touchpad;

	is_touchscreen = evdev_device_has_capability(added_device,
						     LIBINPUT_DEVICE_CAP_TOUCH);
	is_ext_touchpad = evdev_device_has_capability(added_device,
						      LIBINPUT_DEVICE_CAP_POINTER) &&
			  (added_device->tags & EVDEV_TAG_EXTERNAL_TOUCHPAD);

	if (is_touchscreen || is_ext_touchpad)
		tablet_setup_touch_arbitration(device, added_device);

	if (is_ext_touchpad)
		tablet_setup_rotation(device, added_device);
}

static void
tablet_device_removed(struct evdev_device *device,
		      struct evdev_device *removed_device)
{
	struct tablet_dispatch *tablet = tablet_dispatch(device->dispatch);

	if (tablet->touch_device == removed_device)
		tablet->touch_device = NULL;

	if (tablet->rotation.touch_device == removed_device) {
		tablet->rotation.touch_device = NULL;
		tablet->rotation.touch_device_left_handed_state = false;
		tablet_change_rotation(device, DO_NOTIFY);
	}
}

static void
tablet_check_initial_proximity(struct evdev_device *device,
			       struct evdev_dispatch *dispatch)
{
	struct tablet_dispatch *tablet = tablet_dispatch(dispatch);
	struct libinput *li = tablet_libinput_context(tablet);
	int code, state;
	enum libinput_tablet_tool_type tool;

	for (tool = LIBINPUT_TABLET_TOOL_TYPE_PEN;
	     tool <= LIBINPUT_TABLET_TOOL_TYPE_MAX;
	     tool++) {
		code = tablet_tool_to_evcode(tool);

		/* we only expect one tool to be in proximity at a time */
		if (libevdev_fetch_event_value(device->evdev,
						EV_KEY,
						code,
						&state) && state) {
			tablet->tool_state = bit(tool);
			tablet->prev_tool_state = bit(tool);
			break;
		}
	}

	if (!tablet->tool_state)
		return;

	tablet_update_tool(tablet, device, tool, state);
	if (tablet->quirks.need_to_force_prox_out)
		tablet_proximity_out_quirk_set_timer(tablet, libinput_now(li));

	tablet->current_tool.id =
		libevdev_get_event_value(device->evdev,
					 EV_ABS,
					 ABS_MISC);

	/* we can't fetch MSC_SERIAL from the kernel, so we set the serial
	 * to 0 for now. On the first real event from the device we get the
	 * serial (if any) and that event will be converted into a proximity
	 * event */
	tablet->current_tool.serial = 0;
}

/* Called when the touchpad toggles to left-handed */
static void
tablet_left_handed_toggled(struct evdev_dispatch *dispatch,
			   struct evdev_device *device,
			   bool left_handed_enabled)
{
	struct tablet_dispatch *tablet = tablet_dispatch(dispatch);

	if (!tablet->rotation.touch_device)
		return;

	evdev_log_debug(device,
			"tablet-rotation: touchpad is %s\n",
			left_handed_enabled ? "left-handed" : "right-handed");

	/* Our left-handed config is independent even though rotation is
	 * locked. So we rotate when either device is left-handed. But it
	 * can only be actually changed when the device is in a neutral
	 * state, hence the want_rotate.
	 */
	tablet->rotation.touch_device_left_handed_state = left_handed_enabled;
	tablet_change_rotation(device, DONT_NOTIFY);
}

static struct evdev_dispatch_interface tablet_interface = {
	.process = tablet_process,
	.suspend = tablet_suspend,
	.remove = NULL,
	.destroy = tablet_destroy,
	.device_added = tablet_device_added,
	.device_removed = tablet_device_removed,
	.device_suspended = NULL,
	.device_resumed = NULL,
	.post_added = tablet_check_initial_proximity,
	.touch_arbitration_toggle = NULL,
	.touch_arbitration_update_rect = NULL,
	.get_switch_state = NULL,
	.left_handed_toggle = tablet_left_handed_toggled,
};

static void
tablet_init_calibration(struct tablet_dispatch *tablet,
			struct evdev_device *device)
{
	if (libevdev_has_property(device->evdev, INPUT_PROP_DIRECT))
		evdev_init_calibration(device, &tablet->calibration);
}

static void
tablet_init_proximity_threshold(struct tablet_dispatch *tablet,
				struct evdev_device *device)
{
	/* This rules out most of the bamboos and other devices, we're
	 * pretty much down to
	 */
	if (!libevdev_has_event_code(device->evdev, EV_KEY, BTN_TOOL_MOUSE) &&
	    !libevdev_has_event_code(device->evdev, EV_KEY, BTN_TOOL_LENS))
		return;

	/* 42 is the default proximity threshold the xf86-input-wacom driver
	 * uses for Intuos/Cintiq models. Graphire models have a threshold
	 * of 10 but since they haven't been manufactured in ages and the
	 * intersection of users having a graphire, running libinput and
	 * wanting to use the mouse/lens cursor tool is small enough to not
	 * worry about it for now. If we need to, we can introduce a udev
	 * property later.
	 *
	 * Value is in device coordinates.
	 */
	tablet->cursor_proximity_threshold = 42;
}

static uint32_t
tablet_accel_config_get_profiles(struct libinput_device *libinput_device)
{
	return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;
}

static enum libinput_config_status
tablet_accel_config_set_profile(struct libinput_device *libinput_device,
			    enum libinput_config_accel_profile profile)
{
	return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
}

static enum libinput_config_accel_profile
tablet_accel_config_get_profile(struct libinput_device *libinput_device)
{
	return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;
}

static enum libinput_config_accel_profile
tablet_accel_config_get_default_profile(struct libinput_device *libinput_device)
{
	return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;
}

static int
tablet_init_accel(struct tablet_dispatch *tablet, struct evdev_device *device)
{
	const struct input_absinfo *x, *y;
	struct motion_filter *filter;

	x = device->abs.absinfo_x;
	y = device->abs.absinfo_y;

	filter = create_pointer_accelerator_filter_tablet(x->resolution,
							  y->resolution);
	if (!filter)
		return -1;

	evdev_device_init_pointer_acceleration(device, filter);

	/* we override the profile hooks for accel configuration with hooks
	 * that don't allow selection of profiles */
	device->pointer.config.get_profiles = tablet_accel_config_get_profiles;
	device->pointer.config.set_profile = tablet_accel_config_set_profile;
	device->pointer.config.get_profile = tablet_accel_config_get_profile;
	device->pointer.config.get_default_profile = tablet_accel_config_get_default_profile;

	return 0;
}

static void
tablet_init_left_handed(struct evdev_device *device)
{
	if (evdev_tablet_has_left_handed(device))
		evdev_init_left_handed(device,
				       tablet_change_to_left_handed);
}

static bool
tablet_is_aes(struct evdev_device *device,
	      struct tablet_dispatch *tablet)
{
	bool is_aes = false;
#if HAVE_LIBWACOM
	const char *devnode;
	WacomDeviceDatabase *db;
	WacomDevice *libwacom_device = NULL;
	const int *stylus_ids;
	int nstyli;
	int vid = evdev_device_get_id_vendor(device);

	/* Wacom-specific check for whether smoothing is required:
	 * libwacom keeps all the AES pens in a single group, so any device
	 * that supports AES pens will list all AES pens. 0x11 is one of the
	 * lenovo pens so we use that as the flag of whether the tablet
	 * is an AES tablet
	 */
	if (vid != VENDOR_ID_WACOM)
		goto out;

	db = tablet_libinput_context(tablet)->libwacom.db;
	if (!db)
		goto out;

	devnode = udev_device_get_devnode(device->udev_device);
	libwacom_device = libwacom_new_from_path(db, devnode, WFALLBACK_NONE, NULL);
	if (!libwacom_device)
		goto out;

	stylus_ids = libwacom_get_supported_styli(libwacom_device, &nstyli);
	for (int i = 0; i < nstyli; i++) {
		if (stylus_ids[i] == 0x11) {
			is_aes = true;
			break;
		}
	}

	libwacom_destroy(libwacom_device);

out:
#endif
	return is_aes;
}

static void
tablet_init_smoothing(struct evdev_device *device,
		      struct tablet_dispatch *tablet)
{
	size_t history_size = ARRAY_LENGTH(tablet->history.samples);
	struct quirks_context *quirks = NULL;
	struct quirks *q = NULL;
	bool use_smoothing = true;

	quirks = evdev_libinput_context(device)->quirks;
	q = quirks_fetch_for_device(quirks, device->udev_device);

	/* By default, always enable smoothing except on AES devices.
	 * AttrTabletSmoothing can override this, if necessary.
	 */
	if (!q || !quirks_get_bool(q, QUIRK_ATTR_TABLET_SMOOTHING, &use_smoothing))
		use_smoothing = !tablet_is_aes(device, tablet);

	/* Setting the history size to 1 means we never do any actual smoothing. */
	if (!use_smoothing)
		history_size = 1;

	quirks_unref(q);
	tablet->history.size = history_size;
}

static bool
tablet_reject_device(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;
	double w, h;
	bool has_xy, has_pen, has_btn_stylus, has_size;

	has_xy = libevdev_has_event_code(evdev, EV_ABS, ABS_X) &&
	         libevdev_has_event_code(evdev, EV_ABS, ABS_Y);
	has_pen = libevdev_has_event_code(evdev, EV_KEY, BTN_TOOL_PEN);
	has_btn_stylus = libevdev_has_event_code(evdev, EV_KEY, BTN_STYLUS);
	has_size = evdev_device_get_size(device, &w, &h) == 0;

	if (has_xy && (has_pen || has_btn_stylus) && has_size)
		return false;

	evdev_log_bug_libinput(device,
			       "missing tablet capabilities:%s%s%s%s. "
			       "Ignoring this device.\n",
			       has_xy ? "" : " xy",
			       has_pen ? "" : " pen",
			       has_btn_stylus ? "" : " btn-stylus",
			       has_size ? "" : " resolution");
	return true;
}

static int
tablet_init(struct tablet_dispatch *tablet,
	    struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;
	enum libinput_tablet_tool_axis axis;
	int rc;

	tablet->base.dispatch_type = DISPATCH_TABLET;
	tablet->base.interface = &tablet_interface;
	tablet->device = device;
	tablet->status = TABLET_NONE;
	tablet->current_tool.type = LIBINPUT_TOOL_NONE;
	list_init(&tablet->tool_list);

	if (tablet_reject_device(device))
		return -1;

	if (!libevdev_has_event_code(evdev, EV_KEY, BTN_TOOL_PEN)) {
		libevdev_enable_event_code(evdev, EV_KEY, BTN_TOOL_PEN, NULL);
		tablet->quirks.proximity_out_forced = true;
	}

	/* Our rotation code only works with Wacoms, let's wait until
	 * someone shouts */
	if (evdev_device_get_id_vendor(device) != VENDOR_ID_WACOM) {
		libevdev_disable_event_code(evdev, EV_KEY, BTN_TOOL_MOUSE);
		libevdev_disable_event_code(evdev, EV_KEY, BTN_TOOL_LENS);
	}

	tablet_init_calibration(tablet, device);
	tablet_init_proximity_threshold(tablet, device);
	rc = tablet_init_accel(tablet, device);
	if (rc != 0)
		return rc;

	evdev_init_sendevents(device, &tablet->base);
	tablet_init_left_handed(device);
	tablet_init_smoothing(device, tablet);

	for (axis = LIBINPUT_TABLET_TOOL_AXIS_X;
	     axis <= LIBINPUT_TABLET_TOOL_AXIS_MAX;
	     axis++) {
		if (tablet_device_has_axis(tablet, axis))
			set_bit(tablet->axis_caps, axis);
	}

	tablet_set_status(tablet, TABLET_TOOL_OUT_OF_PROXIMITY);

	/* We always enable the proximity out quirk, but disable it once a
	   device gives us the right event sequence */
	tablet->quirks.need_to_force_prox_out = true;

	libinput_timer_init(&tablet->quirks.prox_out_timer,
			    tablet_libinput_context(tablet),
			    "proxout",
			    tablet_proximity_out_quirk_timer_func,
			    tablet);

	return 0;
}

struct evdev_dispatch *
evdev_tablet_create(struct evdev_device *device)
{
	struct tablet_dispatch *tablet;
	struct libinput *li = evdev_libinput_context(device);

	libinput_libwacom_ref(li);

	/* Stop false positives caused by the forced proximity code */
	if (getenv("LIBINPUT_RUNNING_TEST_SUITE"))
		FORCED_PROXOUT_TIMEOUT = 150 * 1000; /* µs */

	tablet = zalloc(sizeof *tablet);

	if (tablet_init(tablet, device) != 0) {
		tablet_destroy(&tablet->base);
		return NULL;
	}

	return &tablet->base;
}
