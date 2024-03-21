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

static struct input_event down[] = {
	{ .type = -1, .code = -1 },
};

static struct input_event move[] = {
	{ .type = -1, .code = -1 },
};

static struct input_event ring_start[] = {
	{ .type = EV_ABS, .code = ABS_WHEEL, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MISC, .value = 15 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
} ;

static struct input_event ring_change[] = {
	{ .type = EV_ABS, .code = ABS_WHEEL, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
} ;

static struct input_event ring_end[] = {
	{ .type = EV_ABS, .code = ABS_WHEEL, .value = 0 },
	{ .type = EV_ABS, .code = ABS_MISC, .value = 0 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
} ;

static struct litest_device_interface interface = {
	.touch_down_events = down,
	.touch_move_events = move,
	.pad_ring_start_events = ring_start,
	.pad_ring_change_events = ring_change,
	.pad_ring_end_events = ring_end,
};
static struct input_absinfo absinfo[] = {
	{ ABS_X, -2048, 2048, 0, 0, 0 },
	{ ABS_Y, -2048, 2048, 0, 0, 0 },
	{ ABS_Z, -2048, 2048, 0, 0, 0 },
	{ ABS_WHEEL, 0, 35, 0, 0, 0 },
	{ ABS_MISC, 0, 0, 0, 0, 0 },
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x56a,
	.product = 0x34e,
	.version = 0x110,
};

static int events[] = {
	EV_KEY, BTN_0,
	EV_KEY, BTN_1,
	EV_KEY, BTN_2,
	EV_KEY, BTN_3,
	EV_KEY, BTN_4,
	EV_KEY, BTN_5,
	EV_KEY, BTN_6,
	EV_KEY, BTN_7,
	EV_KEY, BTN_8,
	EV_KEY, BTN_9,
	EV_KEY, BTN_SOUTH,
	EV_KEY, BTN_EAST,
	EV_KEY, BTN_C,
	EV_KEY, BTN_STYLUS,
	INPUT_PROP_MAX, INPUT_PROP_ACCELEROMETER,
	-1, -1,
};

TEST_DEVICE("wacom-mobilestudio-pro16-pad",
	.type = LITEST_WACOM_MOBILESTUDIO_PRO_16_PAD,
	.features = LITEST_TABLET_PAD | LITEST_RING,
	.interface = &interface,

	.name = "Wacom MobileStudio Pro 16 Pad",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
	.udev_properties = {
		{ "ID_INPUT_TABLET", "1" },
		{ "ID_INPUT_TABLET_PAD", "1" },
		{ NULL },
	},
)
