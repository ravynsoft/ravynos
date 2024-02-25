/*
 * Copyright © 2017 Red Hat, Inc.
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

#include "litest.h"
#include "litest-int.h"

static struct input_event proximity_in[] = {
	/* Note: this device does not send BTN_TOOL_PEN */
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	/* Note: this device does not send tilt, despite claiming it has it */
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_KEY, .code = LITEST_BTN_TOOL_AUTO, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event proximity_out[] = {
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event motion[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	/* Note: this device does not send tilt, despite claiming it has it */
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static int
get_axis_default(struct litest_device *d, unsigned int evcode, int32_t *value)
{
	switch (evcode) {
	case ABS_PRESSURE:
		*value = 100;
		return 0;
	}
	return 1;
}

static struct litest_device_interface interface = {
	.tablet_proximity_in_events = proximity_in,
	.tablet_proximity_out_events = proximity_out,
	.tablet_motion_events = motion,

	.get_axis_default = get_axis_default,
};

static struct input_absinfo absinfo[] = {
	{ ABS_X, 0, 5999, 0, 0, 26 },
	{ ABS_Y, 0, 4499, 0, 0, 15 },
	{ ABS_WHEEL, 0, 1023, 0, 0, 0 }, /* mute axis */
	{ ABS_PRESSURE, 0, 1023, 0, 0, 0 },
	{ ABS_TILT_X, -128, 127, 0, 0, 0 }, /* mute axis */
	{ ABS_TILT_Y, -128, 127, 0, 0, 0 }, /* mute axis */
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x8ca,
	.product = 0x10,
};

static int events[] = {
	EV_KEY, KEY_ESC,
	EV_KEY, KEY_F1,
	EV_KEY, KEY_F2,
	EV_KEY, KEY_F3,
	EV_KEY, KEY_F4,
	EV_KEY, KEY_F5,
	EV_KEY, KEY_F6,
	EV_KEY, KEY_F7,
	EV_KEY, KEY_F8,
	EV_KEY, KEY_F9,
	EV_KEY, KEY_F10,
	EV_KEY, KEY_F11,
	EV_KEY, KEY_F12,
	EV_KEY, KEY_STOP,
	EV_KEY, KEY_AGAIN,
	EV_KEY, KEY_PROPS,
	EV_KEY, KEY_UNDO,
	EV_KEY, KEY_FRONT,
	EV_KEY, KEY_COPY,
	EV_KEY, KEY_OPEN,
	EV_KEY, KEY_PASTE,
	EV_KEY, KEY_F13,
	EV_KEY, KEY_F14,
	EV_KEY, KEY_F15,
	EV_KEY, KEY_F16,
	EV_KEY, KEY_F17,
	EV_KEY, KEY_F18,
	EV_KEY, KEY_F19,
	EV_KEY, KEY_F20,
	EV_KEY, KEY_F21,
	EV_KEY, KEY_F22,
	EV_KEY, KEY_F23,
	EV_KEY, KEY_F24,
	EV_KEY, BTN_LEFT,
	EV_KEY, BTN_RIGHT,
	EV_KEY, BTN_MIDDLE,
	EV_KEY, BTN_TOOL_PEN,
	EV_KEY, BTN_TOOL_RUBBER,
	EV_KEY, BTN_TOOL_BRUSH,
	EV_KEY, BTN_TOOL_PENCIL,
	EV_KEY, BTN_TOOL_AIRBRUSH,
	EV_KEY, BTN_TOOL_MOUSE,
	EV_KEY, BTN_TOOL_LENS,
	EV_KEY, BTN_TOUCH,
	EV_KEY, BTN_STYLUS,
	EV_KEY, BTN_STYLUS2,
	EV_REL, REL_X,
	EV_REL, REL_Y,
	EV_REL, REL_WHEEL,
	EV_MSC, MSC_SERIAL,
	-1, -1,
};

TEST_DEVICE("aiptek-tablet",
	.type = LITEST_AIPTEK,
	.features = LITEST_TABLET | LITEST_HOVER | LITEST_FORCED_PROXOUT,
	.interface = &interface,

	.name = "Aiptek",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
)
