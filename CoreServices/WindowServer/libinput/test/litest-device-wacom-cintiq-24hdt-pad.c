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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "litest.h"
#include "litest-int.h"

static void
litest_wacom_cintiq_pad_teardown(void)
{
	litest_generic_device_teardown();
}

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
	{ ABS_X, 0, 1, 0, 0, 0 },
	{ ABS_Y, 0, 1, 0, 0, 0 },
	{ ABS_WHEEL, 0, 71, 0, 0, 0 },
	{ ABS_THROTTLE, 0, 71, 0, 0, 0 },
	{ ABS_MISC, 0, 0, 0, 0, 0 },
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x56a,
	.product = 0xf8,
	.version = 0x110,
};

static int events[] = {
	EV_KEY, KEY_PROG1,
	EV_KEY, KEY_PROG2,
	EV_KEY, KEY_PROG3,
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
	EV_KEY, BTN_NORTH,
	EV_KEY, BTN_WEST,
	EV_KEY, BTN_Z,
	EV_KEY, BTN_STYLUS,
	-1, -1,
};

TEST_DEVICE("wacom-cintiq-24hdt-pad",
	.type = LITEST_WACOM_CINTIQ_24HDT_PAD,
	.features = LITEST_TABLET_PAD | LITEST_RING,
	.teardown = litest_wacom_cintiq_pad_teardown,
	.interface = &interface,

	.name = "Wacom Cintiq 24 HD touch Pad",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
	.udev_properties = {
		{ "ID_INPUT_TABLET_PAD", "1" },
		{ "LIBINPUT_DEVICE_GROUP", "wacom-24hdt-group" },
		{ NULL },
	},
)
