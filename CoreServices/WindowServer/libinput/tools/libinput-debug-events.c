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

#include "config.h"

#include <errno.h>
#include <inttypes.h>
#include <getopt.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "linux/input.h"

#include <libinput.h>
#include <libevdev/libevdev.h>

#include "libinput-version.h"
#include "util-strings.h"
#include "util-macros.h"
#include "shared.h"

static uint32_t start_time;
static const uint32_t screen_width = 100;
static const uint32_t screen_height = 100;
static struct tools_options options;
static bool show_keycodes;
static volatile sig_atomic_t stop = 0;
static bool be_quiet = false;

#define printq(...) ({ if (!be_quiet)  printf(__VA_ARGS__); })

static void
print_event_header(struct libinput_event *ev)
{
	/* use for pointer value only, do not dereference */
	static void *last_device = NULL;
	struct libinput_device *dev = libinput_event_get_device(ev);
	const char *type = NULL;
	char prefix;

	switch(libinput_event_get_type(ev)) {
	case LIBINPUT_EVENT_NONE:
		abort();
	case LIBINPUT_EVENT_DEVICE_ADDED:
		type = "DEVICE_ADDED";
		break;
	case LIBINPUT_EVENT_DEVICE_REMOVED:
		type = "DEVICE_REMOVED";
		break;
	case LIBINPUT_EVENT_KEYBOARD_KEY:
		type = "KEYBOARD_KEY";
		break;
	case LIBINPUT_EVENT_POINTER_MOTION:
		type = "POINTER_MOTION";
		break;
	case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
		type = "POINTER_MOTION_ABSOLUTE";
		break;
	case LIBINPUT_EVENT_POINTER_BUTTON:
		type = "POINTER_BUTTON";
		break;
	case LIBINPUT_EVENT_POINTER_AXIS:
		type = "POINTER_AXIS";
		break;
	case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
		type = "POINTER_SCROLL_WHEEL";
		break;
	case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
		type = "POINTER_SCROLL_FINGER";
		break;
	case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
		type = "POINTER_SCROLL_CONTINUOUS";
		break;
	case LIBINPUT_EVENT_TOUCH_DOWN:
		type = "TOUCH_DOWN";
		break;
	case LIBINPUT_EVENT_TOUCH_MOTION:
		type = "TOUCH_MOTION";
		break;
	case LIBINPUT_EVENT_TOUCH_UP:
		type = "TOUCH_UP";
		break;
	case LIBINPUT_EVENT_TOUCH_CANCEL:
		type = "TOUCH_CANCEL";
		break;
	case LIBINPUT_EVENT_TOUCH_FRAME:
		type = "TOUCH_FRAME";
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
		type = "GESTURE_SWIPE_BEGIN";
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
		type = "GESTURE_SWIPE_UPDATE";
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_END:
		type = "GESTURE_SWIPE_END";
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
		type = "GESTURE_PINCH_BEGIN";
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
		type = "GESTURE_PINCH_UPDATE";
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_END:
		type = "GESTURE_PINCH_END";
		break;
	case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
		type = "GESTURE_HOLD_BEGIN";
		break;
	case LIBINPUT_EVENT_GESTURE_HOLD_END:
		type = "GESTURE_HOLD_END";
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
		type = "TABLET_TOOL_AXIS";
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
		type = "TABLET_TOOL_PROXIMITY";
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_TIP:
		type = "TABLET_TOOL_TIP";
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
		type = "TABLET_TOOL_BUTTON";
		break;
	case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
		type = "TABLET_PAD_BUTTON";
		break;
	case LIBINPUT_EVENT_TABLET_PAD_RING:
		type = "TABLET_PAD_RING";
		break;
	case LIBINPUT_EVENT_TABLET_PAD_STRIP:
		type = "TABLET_PAD_STRIP";
		break;
	case LIBINPUT_EVENT_TABLET_PAD_KEY:
		type = "TABLET_PAD_KEY";
		break;
	case LIBINPUT_EVENT_SWITCH_TOGGLE:
		type = "SWITCH_TOGGLE";
		break;
	}

	prefix = (last_device != dev) ? '-' : ' ';

	printq("%c%-7s  %-23s ",
	       prefix,
	       libinput_device_get_sysname(dev),
	       type);

	last_device = dev;
}

static void
print_event_time(uint32_t time)
{
	printq("%+6.3fs	", start_time ? (time - start_time) / 1000.0 : 0);
}

static inline void
print_device_options(struct libinput_device *dev)
{
	uint32_t scroll_methods, click_methods;

	if (libinput_device_config_tap_get_finger_count(dev)) {
	    printq(" tap");
	    if (libinput_device_config_tap_get_drag_lock_enabled(dev))
		    printq("(dl on)");
	    else
		    printq("(dl off)");
	}
	if (libinput_device_config_left_handed_is_available(dev))
	    printq(" left");
	if (libinput_device_config_scroll_has_natural_scroll(dev))
	    printq(" scroll-nat");
	if (libinput_device_config_calibration_has_matrix(dev))
	    printq(" calib");

	scroll_methods = libinput_device_config_scroll_get_methods(dev);
	if (scroll_methods != LIBINPUT_CONFIG_SCROLL_NO_SCROLL) {
		printq(" scroll");
		if (scroll_methods & LIBINPUT_CONFIG_SCROLL_2FG)
			printq("-2fg");
		if (scroll_methods & LIBINPUT_CONFIG_SCROLL_EDGE)
			printq("-edge");
		if (scroll_methods & LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN)
			printq("-button");
	}

	click_methods = libinput_device_config_click_get_methods(dev);
	if (click_methods != LIBINPUT_CONFIG_CLICK_METHOD_NONE) {
		printq(" click");
		if (click_methods & LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS)
			printq("-buttonareas");
		if (click_methods & LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER)
			printq("-clickfinger");
	}

	if (libinput_device_config_dwt_is_available(dev)) {
		if (libinput_device_config_dwt_get_enabled(dev) ==
		    LIBINPUT_CONFIG_DWT_ENABLED)
			printq(" dwt-on");
		else
			printq(" dwt-off)");
	}

	if (libinput_device_config_dwtp_is_available(dev)) {
		if (libinput_device_config_dwtp_get_enabled(dev) ==
		    LIBINPUT_CONFIG_DWTP_ENABLED)
			printq(" dwtp-on");
		else
			printq(" dwtp-off)");
	}

	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_TABLET_PAD)) {
		int nbuttons, nstrips, nrings, ngroups;

		nbuttons = libinput_device_tablet_pad_get_num_buttons(dev);
		nstrips = libinput_device_tablet_pad_get_num_strips(dev);
		nrings = libinput_device_tablet_pad_get_num_rings(dev);
		ngroups = libinput_device_tablet_pad_get_num_mode_groups(dev);

		printq(" buttons:%d strips:%d rings:%d mode groups:%d",
		       nbuttons,
		       nstrips,
		       nrings,
		       ngroups);
	}
}

static void
print_device_notify(struct libinput_event *ev)
{
	struct libinput_device *dev = libinput_event_get_device(ev);
	struct libinput_seat *seat = libinput_device_get_seat(dev);
	struct libinput_device_group *group;
	double w, h;
	static int next_group_id = 0;
	intptr_t group_id;

	group = libinput_device_get_device_group(dev);
	group_id = (intptr_t)libinput_device_group_get_user_data(group);
	if (!group_id) {
		group_id = ++next_group_id;
		libinput_device_group_set_user_data(group, (void*)group_id);
	}

	printq("%-33s %5s %7s group%-2d",
	       libinput_device_get_name(dev),
	       libinput_seat_get_physical_name(seat),
	       libinput_seat_get_logical_name(seat),
	       (int)group_id);

	printq(" cap:");
	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_KEYBOARD))
		printq("k");
	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_POINTER))
		printq("p");
	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_TOUCH))
		printq("t");
	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_GESTURE))
		printq("g");
	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_TABLET_TOOL))
		printq("T");
	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_TABLET_PAD))
		printq("P");
	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_SWITCH))
		printq("S");

	if (libinput_device_get_size(dev, &w, &h) == 0)
		printq("  size %.0fx%.0fmm", w, h);

	if (libinput_device_has_capability(dev,
					   LIBINPUT_DEVICE_CAP_TOUCH))
		printq(" ntouches %d", libinput_device_touch_get_touch_count(dev));

	if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED)
		print_device_options(dev);

	printq("\n");

}

static void
print_key_event(struct libinput_event *ev)
{
	struct libinput_event_keyboard *k = libinput_event_get_keyboard_event(ev);
	enum libinput_key_state state;
	uint32_t key;
	const char *keyname;

	print_event_time(libinput_event_keyboard_get_time(k));
	state = libinput_event_keyboard_get_key_state(k);

	key = libinput_event_keyboard_get_key(k);
	if (!show_keycodes && (key >= KEY_ESC && key < KEY_ZENKAKUHANKAKU)) {
		keyname = "***";
		key = -1;
	} else {
		keyname = libevdev_event_code_get_name(EV_KEY, key);
		keyname = keyname ? keyname : "???";
	}
	printq("%s (%d) %s\n",
	       keyname,
	       key,
	       state == LIBINPUT_KEY_STATE_PRESSED ? "pressed" : "released");
}

static void
print_motion_event(struct libinput_event *ev)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	double x = libinput_event_pointer_get_dx(p);
	double y = libinput_event_pointer_get_dy(p);
	double ux = libinput_event_pointer_get_dx_unaccelerated(p);
	double uy = libinput_event_pointer_get_dy_unaccelerated(p);

	print_event_time(libinput_event_pointer_get_time(p));

	printq("%6.2f/%6.2f (%+6.2f/%+6.2f)\n", x, y, ux, uy);
}

static void
print_absmotion_event(struct libinput_event *ev)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	double x = libinput_event_pointer_get_absolute_x_transformed(
		p, screen_width);
	double y = libinput_event_pointer_get_absolute_y_transformed(
		p, screen_height);

	print_event_time(libinput_event_pointer_get_time(p));
	printq("%6.2f/%6.2f\n", x, y);
}

static void
print_pointer_button_event(struct libinput_event *ev)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	enum libinput_button_state state;
	const char *buttonname;
	int button;

	print_event_time(libinput_event_pointer_get_time(p));

	button = libinput_event_pointer_get_button(p);
	buttonname = libevdev_event_code_get_name(EV_KEY, button);

	state = libinput_event_pointer_get_button_state(p);
	printq("%s (%d) %s, seat count: %u\n",
	       buttonname ? buttonname : "???",
	       button,
	       state == LIBINPUT_BUTTON_STATE_PRESSED ? "pressed" : "released",
	       libinput_event_pointer_get_seat_button_count(p));
}

static void
print_tablet_axes(struct libinput_event_tablet_tool *t)
{
	struct libinput_tablet_tool *tool = libinput_event_tablet_tool_get_tool(t);
	double x, y;
	double dist, pressure;
	double rotation, slider, wheel;
	double delta;
	double major, minor;

#define changed_sym(ev, ax) \
	(libinput_event_tablet_tool_##ax##_has_changed(ev) ? "*" : "")

	x = libinput_event_tablet_tool_get_x(t);
	y = libinput_event_tablet_tool_get_y(t);
	printq("\t%.2f%s/%.2f%s",
	       x, changed_sym(t, x),
	       y, changed_sym(t, y));

	if (libinput_tablet_tool_has_tilt(tool)) {
		x = libinput_event_tablet_tool_get_tilt_x(t);
		y = libinput_event_tablet_tool_get_tilt_y(t);
		printq("\ttilt: %.2f%s/%.2f%s",
		       x, changed_sym(t, tilt_x),
		       y, changed_sym(t, tilt_y));
	}

	if (libinput_tablet_tool_has_distance(tool) ||
	    libinput_tablet_tool_has_pressure(tool)) {
		dist = libinput_event_tablet_tool_get_distance(t);
		pressure = libinput_event_tablet_tool_get_pressure(t);
		if (dist)
			printq("\tdistance: %.2f%s",
			       dist, changed_sym(t, distance));
		else
			printq("\tpressure: %.2f%s",
			       pressure, changed_sym(t, pressure));
	}

	if (libinput_tablet_tool_has_rotation(tool)) {
		rotation = libinput_event_tablet_tool_get_rotation(t);
		printq("\trotation: %6.2f%s",
		       rotation, changed_sym(t, rotation));
	}

	if (libinput_tablet_tool_has_slider(tool)) {
		slider = libinput_event_tablet_tool_get_slider_position(t);
		printq("\tslider: %.2f%s",
		       slider, changed_sym(t, slider));
	}

	if (libinput_tablet_tool_has_wheel(tool)) {
		wheel = libinput_event_tablet_tool_get_wheel_delta(t);
		delta = libinput_event_tablet_tool_get_wheel_delta_discrete(t);
		printq("\twheel: %.2f%s (%d)",
		       wheel, changed_sym(t, wheel),
		       (int)delta);
	}

	if (libinput_tablet_tool_has_size(tool)) {
		major = libinput_event_tablet_tool_get_size_major(t);
		minor = libinput_event_tablet_tool_get_size_minor(t);
		printq("\tsize: %.2f%s/%.2f%s",
		       major, changed_sym(t, size_major),
		       minor, changed_sym(t, size_minor));
	}
}

static void
print_tablet_tip_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);
	enum libinput_tablet_tool_tip_state state;

	print_event_time(libinput_event_tablet_tool_get_time(t));

	print_tablet_axes(t);

	state = libinput_event_tablet_tool_get_tip_state(t);
	printq(" %s\n", state == LIBINPUT_TABLET_TOOL_TIP_DOWN ? "down" : "up");
}

static void
print_tablet_button_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *p = libinput_event_get_tablet_tool_event(ev);
	enum libinput_button_state state;
	const char *buttonname;
	int button;

	print_event_time(libinput_event_tablet_tool_get_time(p));

	button = libinput_event_tablet_tool_get_button(p);
	buttonname = libevdev_event_code_get_name(EV_KEY, button);

	state = libinput_event_tablet_tool_get_button_state(p);
	printq("%3d (%s) %s, seat count: %u\n",
	       button,
	       buttonname ? buttonname : "???",
	       state == LIBINPUT_BUTTON_STATE_PRESSED ? "pressed" : "released",
	       libinput_event_tablet_tool_get_seat_button_count(p));
}

static void
print_pointer_axis_event(struct libinput_event *ev)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	double v = 0, h = 0, v120 = 0, h120 = 0;
	const char *have_vert = "",
		   *have_horiz = "";
	const char *source = NULL;
	enum libinput_pointer_axis axis;
	enum libinput_event_type type;

	type = libinput_event_get_type(ev);

	switch (type) {
	case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
		source = "wheel";
		break;
	case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
		source = "finger";
		break;
	case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
		source = "continuous";
		break;
	default:
		abort();
		break;
	}

	axis = LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL;
	if (libinput_event_pointer_has_axis(p, axis)) {
		v = libinput_event_pointer_get_scroll_value(p, axis);
		if (type == LIBINPUT_EVENT_POINTER_SCROLL_WHEEL)
			v120 = libinput_event_pointer_get_scroll_value_v120(p, axis);
		have_vert = "*";
	}
	axis = LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL;
	if (libinput_event_pointer_has_axis(p, axis)) {
		h = libinput_event_pointer_get_scroll_value(p, axis);
		if (type == LIBINPUT_EVENT_POINTER_SCROLL_WHEEL)
			h120 = libinput_event_pointer_get_scroll_value_v120(p, axis);
		have_horiz = "*";
	}

	print_event_time(libinput_event_pointer_get_time(p));
	printq("vert %.2f/%.1f%s horiz %.2f/%.1f%s (%s)\n",
	       v, v120, have_vert,
	       h, h120, have_horiz, source);
}

static void
print_tablet_axis_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);

	print_event_time(libinput_event_tablet_tool_get_time(t));
	print_tablet_axes(t);
	printq("\n");
}

static void
print_proximity_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);
	struct libinput_tablet_tool *tool = libinput_event_tablet_tool_get_tool(t);
	enum libinput_tablet_tool_proximity_state state;
	const char *tool_str,
	           *state_str;

	switch (libinput_tablet_tool_get_type(tool)) {
	case LIBINPUT_TABLET_TOOL_TYPE_PEN:
		tool_str = "pen";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_ERASER:
		tool_str = "eraser";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:
		tool_str = "brush";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:
		tool_str = "pencil";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:
		tool_str = "airbrush";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:
		tool_str = "mouse";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:
		tool_str = "lens";
		break;
	case LIBINPUT_TABLET_TOOL_TYPE_TOTEM:
		tool_str = "totem";
		break;
	default:
		abort();
	}

	state = libinput_event_tablet_tool_get_proximity_state(t);

	print_event_time(libinput_event_tablet_tool_get_time(t));

	if (state == LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN) {
		print_tablet_axes(t);
		state_str = "proximity-in";
	} else if (state == LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT) {
		print_tablet_axes(t);
		state_str = "proximity-out";
	} else {
		abort();
	}

	printq("\t%-8s (%#" PRIx64 ", id %#" PRIx64 ") %s ",
	       tool_str,
	       libinput_tablet_tool_get_serial(tool),
	       libinput_tablet_tool_get_tool_id(tool),
	       state_str);

	if (state == LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN) {
		printq("\taxes:");
		if (libinput_tablet_tool_has_distance(tool))
			printq("d");
		if (libinput_tablet_tool_has_pressure(tool))
			printq("p");
		if (libinput_tablet_tool_has_tilt(tool))
			printq("t");
		if (libinput_tablet_tool_has_rotation(tool))
			printq("r");
		if (libinput_tablet_tool_has_slider(tool))
			printq("s");
		if (libinput_tablet_tool_has_wheel(tool))
			printq("w");
		if (libinput_tablet_tool_has_size(tool))
			printq("S");

		printq("\tbtn:");
		if (libinput_tablet_tool_has_button(tool, BTN_TOUCH))
			printq("T");
		if (libinput_tablet_tool_has_button(tool, BTN_STYLUS))
			printq("S");
		if (libinput_tablet_tool_has_button(tool, BTN_STYLUS2))
			printq("S2");
		if (libinput_tablet_tool_has_button(tool, BTN_LEFT))
			printq("L");
		if (libinput_tablet_tool_has_button(tool, BTN_MIDDLE))
			printq("M");
		if (libinput_tablet_tool_has_button(tool, BTN_RIGHT))
			printq("R");
		if (libinput_tablet_tool_has_button(tool, BTN_SIDE))
			printq("Sd");
		if (libinput_tablet_tool_has_button(tool, BTN_EXTRA))
			printq("Ex");
		if (libinput_tablet_tool_has_button(tool, BTN_0))
			printq("0");
	}

	printq("\n");
}

static void
print_touch_event(struct libinput_event *ev)
{
	struct libinput_event_touch *t = libinput_event_get_touch_event(ev);
	enum libinput_event_type type = libinput_event_get_type(ev);

	print_event_time(libinput_event_touch_get_time(t));

	if (type != LIBINPUT_EVENT_TOUCH_FRAME) {
		printq("%d (%d)",
		       libinput_event_touch_get_slot(t),
		       libinput_event_touch_get_seat_slot(t));
	}

	if (type == LIBINPUT_EVENT_TOUCH_DOWN ||
	    type == LIBINPUT_EVENT_TOUCH_MOTION) {
		double x = libinput_event_touch_get_x_transformed(t, screen_width);
		double y = libinput_event_touch_get_y_transformed(t, screen_height);
		double xmm = libinput_event_touch_get_x(t);
		double ymm = libinput_event_touch_get_y(t);

		printq(" %5.2f/%5.2f (%5.2f/%5.2fmm)", x, y, xmm, ymm);
	}

	printq("\n");
}

static void
print_gesture_event_without_coords(struct libinput_event *ev)
{
	struct libinput_event_gesture *t = libinput_event_get_gesture_event(ev);
	int finger_count = libinput_event_gesture_get_finger_count(t);
	int cancelled = 0;
	enum libinput_event_type type;

	type = libinput_event_get_type(ev);

	if (type == LIBINPUT_EVENT_GESTURE_SWIPE_END ||
	    type == LIBINPUT_EVENT_GESTURE_PINCH_END ||
	    type == LIBINPUT_EVENT_GESTURE_HOLD_END)
	    cancelled = libinput_event_gesture_get_cancelled(t);

	print_event_time(libinput_event_gesture_get_time(t));
	printq("%d%s\n", finger_count, cancelled ? " cancelled" : "");
}

static void
print_gesture_event_with_coords(struct libinput_event *ev)
{
	struct libinput_event_gesture *t = libinput_event_get_gesture_event(ev);
	double dx = libinput_event_gesture_get_dx(t);
	double dy = libinput_event_gesture_get_dy(t);
	double dx_unaccel = libinput_event_gesture_get_dx_unaccelerated(t);
	double dy_unaccel = libinput_event_gesture_get_dy_unaccelerated(t);

	print_event_time(libinput_event_gesture_get_time(t));

	printq("%d %5.2f/%5.2f (%5.2f/%5.2f unaccelerated)",
	       libinput_event_gesture_get_finger_count(t),
	       dx, dy, dx_unaccel, dy_unaccel);

	if (libinput_event_get_type(ev) ==
	    LIBINPUT_EVENT_GESTURE_PINCH_UPDATE) {
		double scale = libinput_event_gesture_get_scale(t);
		double angle = libinput_event_gesture_get_angle_delta(t);

		printq(" %5.2f @ %5.2f\n", scale, angle);
	} else {
		printq("\n");
	}
}

static void
print_tablet_pad_button_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_pad *p = libinput_event_get_tablet_pad_event(ev);
	struct libinput_tablet_pad_mode_group *group;
	enum libinput_button_state state;
	unsigned int button, mode;

	print_event_time(libinput_event_tablet_pad_get_time(p));

	button = libinput_event_tablet_pad_get_button_number(p),
	state = libinput_event_tablet_pad_get_button_state(p);
	mode = libinput_event_tablet_pad_get_mode(p);
	printq("%3d %s (mode %d)",
	       button,
	       state == LIBINPUT_BUTTON_STATE_PRESSED ? "pressed" : "released",
	       mode);

	group = libinput_event_tablet_pad_get_mode_group(p);
	if (libinput_tablet_pad_mode_group_button_is_toggle(group, button))
		printq(" <mode toggle>");

	printq("\n");
}

static void
print_tablet_pad_ring_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_pad *p = libinput_event_get_tablet_pad_event(ev);
	const char *source = NULL;
	unsigned int mode;

	print_event_time(libinput_event_tablet_pad_get_time(p));

	switch (libinput_event_tablet_pad_get_ring_source(p)) {
	case LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER:
		source = "finger";
		break;
	case LIBINPUT_TABLET_PAD_RING_SOURCE_UNKNOWN:
		source = "unknown";
		break;
	}

	mode = libinput_event_tablet_pad_get_mode(p);
	printq("ring %d position %.2f (source %s) (mode %d)\n",
	       libinput_event_tablet_pad_get_ring_number(p),
	       libinput_event_tablet_pad_get_ring_position(p),
	       source,
	       mode);
}

static void
print_tablet_pad_strip_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_pad *p = libinput_event_get_tablet_pad_event(ev);
	const char *source = NULL;
	unsigned int mode;

	print_event_time(libinput_event_tablet_pad_get_time(p));

	switch (libinput_event_tablet_pad_get_strip_source(p)) {
	case LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER:
		source = "finger";
		break;
	case LIBINPUT_TABLET_PAD_STRIP_SOURCE_UNKNOWN:
		source = "unknown";
		break;
	}

	mode = libinput_event_tablet_pad_get_mode(p);
	printq("strip %d position %.2f (source %s) (mode %d)\n",
	       libinput_event_tablet_pad_get_strip_number(p),
	       libinput_event_tablet_pad_get_strip_position(p),
	       source,
	       mode);
}

static void
print_tablet_pad_key_event(struct libinput_event *ev)
{
	struct libinput_event_tablet_pad *p = libinput_event_get_tablet_pad_event(ev);
	enum libinput_key_state state;
	uint32_t key;
	const char *keyname;

	print_event_time(libinput_event_tablet_pad_get_time(p));

	key = libinput_event_tablet_pad_get_key(p);
	if (!show_keycodes && (key >= KEY_ESC && key < KEY_ZENKAKUHANKAKU)) {
		keyname = "***";
		key = -1;
	} else {
		keyname = libevdev_event_code_get_name(EV_KEY, key);
		keyname = keyname ? keyname : "???";
	}
	state = libinput_event_tablet_pad_get_key_state(p);
	printq("%s (%d) %s\n",
	       keyname,
	       key,
	       state == LIBINPUT_KEY_STATE_PRESSED ? "pressed" : "released");
}

static void
print_switch_event(struct libinput_event *ev)
{
	struct libinput_event_switch *sw = libinput_event_get_switch_event(ev);
	enum libinput_switch_state state;
	const char *which;

	print_event_time(libinput_event_switch_get_time(sw));

	switch (libinput_event_switch_get_switch(sw)) {
	case LIBINPUT_SWITCH_LID:
		which = "lid";
		break;
	case LIBINPUT_SWITCH_TABLET_MODE:
		which = "tablet-mode";
		break;
	default:
		abort();
	}

	state = libinput_event_switch_get_switch_state(sw);

	printq("switch %s state %d\n", which, state);
}

static int
handle_and_print_events(struct libinput *li)
{
	int rc = -1;
	struct libinput_event *ev;

	tools_dispatch(li);
	while ((ev = libinput_get_event(li))) {
		enum libinput_event_type type = libinput_event_get_type(ev);

		if (type != LIBINPUT_EVENT_POINTER_AXIS)
			print_event_header(ev);

		switch (type) {
		case LIBINPUT_EVENT_NONE:
			abort();
		case LIBINPUT_EVENT_DEVICE_ADDED:
			print_device_notify(ev);
			tools_device_apply_config(libinput_event_get_device(ev),
						  &options);
			break;
		case LIBINPUT_EVENT_DEVICE_REMOVED:
			print_device_notify(ev);
			break;
		case LIBINPUT_EVENT_KEYBOARD_KEY:
			print_key_event(ev);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION:
			print_motion_event(ev);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
			print_absmotion_event(ev);
			break;
		case LIBINPUT_EVENT_POINTER_BUTTON:
			print_pointer_button_event(ev);
			break;
		case LIBINPUT_EVENT_POINTER_AXIS:
			/* ignore */
			break;
		case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
		case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
		case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
			print_pointer_axis_event(ev);
			break;
		case LIBINPUT_EVENT_TOUCH_DOWN:
		case LIBINPUT_EVENT_TOUCH_MOTION:
		case LIBINPUT_EVENT_TOUCH_UP:
		case LIBINPUT_EVENT_TOUCH_CANCEL:
		case LIBINPUT_EVENT_TOUCH_FRAME:
			print_touch_event(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
			print_gesture_event_with_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_SWIPE_END:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
			print_gesture_event_with_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_PINCH_END:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_HOLD_END:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
			print_tablet_axis_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
			print_proximity_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_TIP:
			print_tablet_tip_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
			print_tablet_button_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
			print_tablet_pad_button_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_RING:
			print_tablet_pad_ring_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_STRIP:
			print_tablet_pad_strip_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_KEY:
			print_tablet_pad_key_event(ev);
			break;
		case LIBINPUT_EVENT_SWITCH_TOGGLE:
			print_switch_event(ev);
			break;
		}

		libinput_event_destroy(ev);
		rc = 0;
	}

	fflush(stdout);

	return rc;
}

static void
sighandler(int signal, siginfo_t *siginfo, void *userdata)
{
	stop = 1;
}

static void
mainloop(struct libinput *li)
{
	struct pollfd fds;

	fds.fd = libinput_get_fd(li);
	fds.events = POLLIN;
	fds.revents = 0;

	/* Handle already-pending device added events */
	if (handle_and_print_events(li))
		fprintf(stderr, "Expected device added events on startup but got none. "
				"Maybe you don't have the right permissions?\n");

	/* time offset starts with our first received event */
	if (poll(&fds, 1, -1) > -1) {
		struct timespec tp;

		clock_gettime(CLOCK_MONOTONIC, &tp);
		start_time = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
		do {
			handle_and_print_events(li);
		} while (!stop && poll(&fds, 1, -1) > -1);
	}

	printf("\n");
}

static void
usage(void) {
	printf("Usage: libinput debug-events [options] [--udev <seat>|--device /dev/input/event0 ...]\n");
}

int
main(int argc, char **argv)
{
	struct libinput *li;
	enum tools_backend backend = BACKEND_NONE;
	const char *seat_or_devices[60] = {NULL};
	size_t ndevices = 0;
	bool grab = false;
	bool verbose = false;
	struct sigaction act;

	tools_init_options(&options);

	while (1) {
		int c;
		int option_index = 0;
		enum {
			OPT_DEVICE = 1,
			OPT_UDEV,
			OPT_GRAB,
			OPT_VERBOSE,
			OPT_SHOW_KEYCODES,
			OPT_QUIET,
		};
		static struct option opts[] = {
			CONFIGURATION_OPTIONS,
			{ "help",                      no_argument,       0, 'h' },
			{ "show-keycodes",             no_argument,       0, OPT_SHOW_KEYCODES },
			{ "device",                    required_argument, 0, OPT_DEVICE },
			{ "udev",                      required_argument, 0, OPT_UDEV },
			{ "grab",                      no_argument,       0, OPT_GRAB },
			{ "verbose",                   no_argument,       0, OPT_VERBOSE },
			{ "quiet",                     no_argument,       0, OPT_QUIET },
			{ 0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "h", opts, &option_index);
		if (c == -1)
			break;

		switch(c) {
		case '?':
			exit(EXIT_INVALID_USAGE);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case OPT_SHOW_KEYCODES:
			show_keycodes = true;
			break;
		case OPT_QUIET:
			be_quiet = true;
			break;
		case OPT_DEVICE:
			if (backend == BACKEND_UDEV ||
			    ndevices >= ARRAY_LENGTH(seat_or_devices)) {
				usage();
				return EXIT_INVALID_USAGE;

			}
			backend = BACKEND_DEVICE;
			seat_or_devices[ndevices++] = optarg;
			break;
		case OPT_UDEV:
			if (backend == BACKEND_DEVICE ||
			    ndevices >= ARRAY_LENGTH(seat_or_devices)) {
				usage();
				return EXIT_INVALID_USAGE;

			}
			backend = BACKEND_UDEV;
			seat_or_devices[0] = optarg;
			ndevices = 1;
			break;
		case OPT_GRAB:
			grab = true;
			break;
		case OPT_VERBOSE:
			verbose = true;
			break;
		default:
			if (tools_parse_option(c, optarg, &options) != 0) {
				usage();
				return EXIT_INVALID_USAGE;
			}
			break;
		}

	}

	if (optind < argc) {
		if (backend == BACKEND_UDEV) {
			usage();
			return EXIT_INVALID_USAGE;
		}
		backend = BACKEND_DEVICE;
		do {
			if (ndevices >= ARRAY_LENGTH(seat_or_devices)) {
				usage();
				return EXIT_INVALID_USAGE;
			}
			seat_or_devices[ndevices++] = argv[optind];
		} while(++optind < argc);
	} else if (backend == BACKEND_NONE) {
		backend = BACKEND_UDEV;
		seat_or_devices[0] = "seat0";
	}

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sighandler;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGINT, &act, NULL) == -1) {
		fprintf(stderr, "Failed to set up signal handling (%s)\n",
				strerror(errno));
		return EXIT_FAILURE;
	}

	if (verbose)
		printf("libinput version: %s\n", LIBINPUT_VERSION);

	li = tools_open_backend(backend, seat_or_devices, verbose, &grab);
	if (!li)
		return EXIT_FAILURE;

	mainloop(li);

	libinput_unref(li);

	return EXIT_SUCCESS;
}
