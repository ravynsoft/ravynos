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

#include "litest.h"
#include "litest-int.h"

static struct input_event proximity_in[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_TILT_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_TILT_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_KEY, .code = LITEST_BTN_TOOL_AUTO, .value = 1 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event proximity_out[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = 0 },
	{ .type = EV_ABS, .code = ABS_Y, .value = 0 },
	{ .type = EV_ABS, .code = ABS_TILT_X, .value = 0 },
	{ .type = EV_ABS, .code = ABS_TILT_Y, .value = 0 },
	{ .type = EV_KEY, .code = LITEST_BTN_TOOL_AUTO, .value = 0 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event motion[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_TILT_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_TILT_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static int
get_axis_default(struct litest_device *d, unsigned int evcode, int32_t *value)
{
	switch (evcode) {
	case ABS_TILT_X:
	case ABS_TILT_Y:
		*value = 0;
		return 0;
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
	{ ABS_X, 0, 32000, 0, 0, 0 },
	{ ABS_Y, 0, 32000, 0, 0, 0 },
	{ ABS_PRESSURE, 0, 2047, 0, 0, 0 },
	{ ABS_TILT_X, -127, 127, 0, 0, 0 },
	{ ABS_TILT_Y, -127, 127, 0, 0, 0 },
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x172f,
	.product = 0x509,
};

static int events[] = {
	EV_KEY, KEY_ESC,
	EV_KEY, KEY_1,
	EV_KEY, KEY_2,
	EV_KEY, KEY_3,
	EV_KEY, KEY_4,
	EV_KEY, KEY_5,
	EV_KEY, KEY_6,
	EV_KEY, KEY_7,
	EV_KEY, KEY_8,
	EV_KEY, KEY_9,
	EV_KEY, KEY_0,
	EV_KEY, KEY_MINUS,
	EV_KEY, KEY_EQUAL,
	EV_KEY, KEY_BACKSPACE,
	EV_KEY, KEY_TAB,
	EV_KEY, KEY_Q,
	EV_KEY, KEY_W,
	EV_KEY, KEY_E,
	EV_KEY, KEY_R,
	EV_KEY, KEY_T,
	EV_KEY, KEY_Y,
	EV_KEY, KEY_U,
	EV_KEY, KEY_I,
	EV_KEY, KEY_O,
	EV_KEY, KEY_P,
	EV_KEY, KEY_LEFTBRACE,
	EV_KEY, KEY_RIGHTBRACE,
	EV_KEY, KEY_ENTER,
	EV_KEY, KEY_LEFTCTRL,
	EV_KEY, KEY_A,
	EV_KEY, KEY_S,
	EV_KEY, KEY_D,
	EV_KEY, KEY_F,
	EV_KEY, KEY_G,
	EV_KEY, KEY_H,
	EV_KEY, KEY_J,
	EV_KEY, KEY_K,
	EV_KEY, KEY_L,
	EV_KEY, KEY_SEMICOLON,
	EV_KEY, KEY_APOSTROPHE,
	EV_KEY, KEY_GRAVE,
	EV_KEY, KEY_LEFTSHIFT,
	EV_KEY, KEY_BACKSLASH,
	EV_KEY, KEY_Z,
	EV_KEY, KEY_X,
	EV_KEY, KEY_C,
	EV_KEY, KEY_V,
	EV_KEY, KEY_B,
	EV_KEY, KEY_N,
	EV_KEY, KEY_M,
	EV_KEY, KEY_COMMA,
	EV_KEY, KEY_DOT,
	EV_KEY, KEY_SLASH,
	EV_KEY, KEY_RIGHTSHIFT,
	EV_KEY, KEY_KPASTERISK,
	EV_KEY, KEY_LEFTALT,
	EV_KEY, KEY_SPACE,
	EV_KEY, KEY_CAPSLOCK,
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
	EV_KEY, KEY_NUMLOCK,
	EV_KEY, KEY_SCROLLLOCK,
	EV_KEY, KEY_KP7,
	EV_KEY, KEY_KP8,
	EV_KEY, KEY_KP9,
	EV_KEY, KEY_KPMINUS,
	EV_KEY, KEY_KP4,
	EV_KEY, KEY_KP5,
	EV_KEY, KEY_KP6,
	EV_KEY, KEY_KPPLUS,
	EV_KEY, KEY_KP1,
	EV_KEY, KEY_KP2,
	EV_KEY, KEY_KP3,
	EV_KEY, KEY_KP0,
	EV_KEY, KEY_KPDOT,
	EV_KEY, KEY_102ND,
	EV_KEY, KEY_F11,
	EV_KEY, KEY_F12,
	EV_KEY, KEY_KPENTER,
	EV_KEY, KEY_RIGHTCTRL,
	EV_KEY, KEY_KPSLASH,
	EV_KEY, KEY_SYSRQ,
	EV_KEY, KEY_RIGHTALT,
	EV_KEY, KEY_HOME,
	EV_KEY, KEY_UP,
	EV_KEY, KEY_PAGEUP,
	EV_KEY, KEY_LEFT,
	EV_KEY, KEY_RIGHT,
	EV_KEY, KEY_END,
	EV_KEY, KEY_DOWN,
	EV_KEY, KEY_PAGEDOWN,
	EV_KEY, KEY_INSERT,
	EV_KEY, KEY_DELETE,
	EV_KEY, KEY_MUTE,
	EV_KEY, KEY_VOLUMEDOWN,
	EV_KEY, KEY_VOLUMEUP,
	EV_KEY, KEY_PAUSE,
	EV_KEY, KEY_LEFTMETA,
	EV_KEY, KEY_RIGHTMETA,
	EV_KEY, KEY_COMPOSE,
	EV_KEY, BTN_0,
	EV_KEY, BTN_LEFT,
	EV_KEY, BTN_RIGHT,
	EV_KEY, BTN_MIDDLE,
	EV_KEY, BTN_SIDE,
	EV_KEY, BTN_EXTRA,
	EV_KEY, BTN_TOOL_PEN,
	EV_KEY, BTN_TOOL_RUBBER,
	EV_KEY, BTN_TOUCH,
	EV_KEY, BTN_STYLUS,
	EV_REL, REL_HWHEEL,
	EV_REL, REL_WHEEL,
	EV_MSC, MSC_SERIAL,
	-1, -1,
};

static const char quirk_file[] =
"[litest Waltop Tablet]\n"
"MatchName=litest          WALTOP     Batteryless Tablet*\n"
"AttrSizeHint=200x200\n";

TEST_DEVICE("waltop-tablet",
	.type = LITEST_WALTOP,
	.features = LITEST_TABLET | LITEST_WHEEL | LITEST_TILT | LITEST_HOVER,
	.interface = &interface,

	.name = "         WALTOP     Batteryless Tablet ", /* sic */
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
	.quirk_file = quirk_file,
)
