/*
 * Copyright © 2020 Red Hat, Inc.
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

static struct input_id input_id = {
	.bustype = 0x11,
	.vendor = 0x1234,
	.product = 0x4567,
};

static int events[] = {
	EV_KEY, BTN_LEFT,
	EV_KEY, BTN_RIGHT,
	EV_KEY, BTN_MIDDLE,
	EV_KEY, BTN_TOOL_FINGER,
	EV_KEY, BTN_TOUCH,
	EV_KEY, BTN_TOOL_DOUBLETAP,
	EV_KEY, BTN_TOOL_TRIPLETAP,
	EV_KEY, BTN_TOOL_QUADTAP,
	INPUT_PROP_MAX, INPUT_PROP_POINTER,
	-1 , -1,
};

static struct input_absinfo absinfo[] = {
	{ ABS_X, 0, 2000, 0, 0, 0 },
	{ ABS_Y, 0, 1400, 0, 0, 0 },
	{ ABS_PRESSURE, 0, 127, 0, 0, 0 },
	{ ABS_MT_SLOT, 0, 1, 0, 0, 0 },
	{ ABS_MT_POSITION_X, 0, 2000, 0, 0, 0 },
	{ ABS_MT_POSITION_Y, 0, 1400, 0, 0, 0 },
	{ ABS_MT_TRACKING_ID, 0, 65535, 0, 0, 0 },
	{ .value = -1 }
};

/* This device only exists to verify that the EVDEV_ABS override bits work
 * correctly */
TEST_DEVICE("absinfo-override",
	.type = LITEST_ABSINFO_OVERRIDE,
	.features = LITEST_IGNORED,
	.interface = NULL,

	.name = "absinfo override",
	.id = &input_id,
	.absinfo = absinfo,
	.events = events,
	.udev_properties = {
	  { "EVDEV_ABS_00", "1:1000:100:10" },
	  { "EVDEV_ABS_01", "2:2000:200:20" },
	  { "EVDEV_ABS_35", "3:3000:300:30" },
	  { "EVDEV_ABS_36", "4:4000:400:40" },
	  { NULL },
	},
)
