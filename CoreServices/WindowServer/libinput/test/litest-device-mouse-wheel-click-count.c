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

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x1234,
	.product = 0x5678,
};

static int events[] = {
	EV_KEY, BTN_LEFT,
	EV_KEY, BTN_RIGHT,
	EV_KEY, BTN_MIDDLE,
	EV_REL, REL_X,
	EV_REL, REL_Y,
	EV_REL, REL_WHEEL,
	-1 , -1,
};

TEST_DEVICE("mouse-wheelclickcount",
	.type = LITEST_MOUSE_WHEEL_CLICK_COUNT,
	.features = LITEST_RELATIVE | LITEST_BUTTON | LITEST_WHEEL,
	.interface = NULL,

	.name = "Wheel Click Count Mouse",
	.id = &input_id,
	.absinfo = NULL,
	.events = events,
	.udev_properties = {
		{ "MOUSE_WHEEL_CLICK_ANGLE", "-15" },
		{ "MOUSE_WHEEL_CLICK_ANGLE_HORIZONTAL", "13" },
		{ "MOUSE_WHEEL_CLICK_COUNT", "-14" },
		{ "MOUSE_WHEEL_CLICK_COUNT_HORIZONTAL", "27" },
		{ NULL },
	}
)
