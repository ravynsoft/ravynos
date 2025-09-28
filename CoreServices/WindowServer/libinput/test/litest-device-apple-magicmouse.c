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

#include "litest.h"
#include "litest-int.h"

static struct input_event down[] = {
	{ .type = EV_ABS, .code = ABS_MT_SLOT, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MAJOR, .value = 272 },
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MINOR, .value = 400 },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event move[] = {
	{ .type = EV_ABS, .code = ABS_MT_SLOT, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MAJOR, .value = 272 },
	{ .type = EV_ABS, .code = ABS_MT_TOUCH_MINOR, .value = 400 },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct litest_device_interface interface = {
	.touch_down_events = down,
	.touch_move_events = move,
};

static struct input_id input_id = {
	.bustype = 0x5,
	.vendor = 0x5ac,
	.product = 0x30d,
};

static int events[] = {
	EV_KEY, BTN_LEFT,
	EV_KEY, BTN_RIGHT,
	EV_KEY, BTN_MIDDLE,
	EV_REL, REL_X,
	EV_REL, REL_Y,
	EV_REL, REL_WHEEL,
	EV_REL, REL_HWHEEL,
	-1 , -1,
};

static struct input_absinfo absinfo[] = {
	{ ABS_MT_SLOT, 0, 15, 0, 0, 0 },
	{ ABS_MT_TOUCH_MAJOR, 0, 1020, 0, 0, 0 },
	{ ABS_MT_TOUCH_MINOR, 0, 1020, 0, 0, 0 },
	{ ABS_MT_ORIENTATION, -31, 32, 1, 0, 0 },
	{ ABS_MT_POSITION_X, -1100, 1258, 4, 0, 26 },
	{ ABS_MT_POSITION_Y, -1589, 2047, 4, 0, 26 },
	{ ABS_MT_TRACKING_ID, 0, 65535, 0, 0, 0 },
	{ .value = -1 }
};

TEST_DEVICE("magicmouse",
	.type = LITEST_MAGICMOUSE,
	.features = LITEST_RELATIVE | LITEST_BUTTON | LITEST_WHEEL,
	.interface = &interface,

	.name = "Apple Magic Mouse",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,

	/* Force MOUSE_DPI to the empty string. As of systemd commit f013e99e160f
	 * ID_BUS=bluetooth now triggers the hwdb entry for this device. This causes
	 * test case failures because deltas change. Detecting old vs new systemd is
	 * hard, and because our rules are 99-prefixed we can't set ID_BUS ourselves
	 * on older systemd.
	 * So let's go the easy way and unset MOUSE_DPI so we can continue to use
	 * the current tests.
	 */
	.udev_properties = {
		{ "MOUSE_DPI", "" },
		{ NULL },
	},
)
