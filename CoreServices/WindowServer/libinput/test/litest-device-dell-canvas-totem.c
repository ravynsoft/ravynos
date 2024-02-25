/*
 * Copyright © 2018 Red Hat, Inc.
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

/* We don't expect anything but slot 0 to be used, ever */
#define TOTEM_SLOT 0

static struct input_event down[] = {
	{ .type = EV_ABS, .code = ABS_MT_SLOT, .value = TOTEM_SLOT },
	{ .type = EV_ABS, .code = ABS_MT_TOOL_TYPE, .value = MT_TOOL_DIAL }, /* fixed value in device */
	{ .type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_ORIENTATION, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MAJOR, .value = 718 }, /* fixed value in device */
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MINOR, .value = 718 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event move[] = {
	{ .type = EV_ABS, .code = ABS_MT_SLOT, .value = TOTEM_SLOT },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_ORIENTATION, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MAJOR, .value = 718 }, /* fixed value in device */
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MINOR, .value = 718 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event up[] = {
	{ .type = EV_ABS, .code = ABS_MT_SLOT, .value = TOTEM_SLOT },
	{ .type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = -1 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static int
get_axis_default(struct litest_device *d, unsigned int evcode, int32_t *value)
{
	switch (evcode) {
	case ABS_MT_ORIENTATION:
		*value = 0;
		return 0;
	}
	return 1;
}

static struct litest_device_interface interface = {
	.tablet_proximity_in_events = down,
	.tablet_proximity_out_events = up,
	.tablet_motion_events = move,

	.get_axis_default = get_axis_default,
};

static struct input_absinfo absinfo[] = {
	{ ABS_MT_SLOT, 0, 4, 0, 0, 0 },
	{ ABS_MT_TOUCH_MAJOR, 0, 32767, 0, 0, 10 },
	{ ABS_MT_TOUCH_MINOR, 0, 32767, 0, 0, 10 },
	{ ABS_MT_ORIENTATION, -89, 89, 0, 0, 0 },
	{ ABS_MT_POSITION_X, 0, 32767, 0, 0, 55 },
	{ ABS_MT_POSITION_Y, 0, 32767, 0, 0, 98 },
	/* The real device has a min/max of 10/10 but uinput didn't allow
	 * this */
	{ ABS_MT_TOOL_TYPE, 9, 10, 0, 0, 0 },
	{ ABS_MT_TRACKING_ID, 0, 65535, 0, 0, 0 },
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x2575,
	.product = 0x0204,
	.version = 0x111,
};

static int events[] = {
	EV_KEY, BTN_0,
	EV_MSC, MSC_TIMESTAMP,
	INPUT_PROP_MAX, INPUT_PROP_DIRECT,
	-1, -1,
};

TEST_DEVICE("dell-canvas-totem",
	.type = LITEST_DELL_CANVAS_TOTEM,
	.features = LITEST_TOTEM | LITEST_TABLET,
	.interface = &interface,

	.name = "Advanced Silicon S.A. CoolTouch® System System Multi Axis",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
	.udev_properties = {
	 { "LIBINPUT_DEVICE_GROUP", "dell-canvas-totem-group" },
	 { NULL },
	},
)
