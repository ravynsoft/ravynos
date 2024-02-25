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

static struct input_id input_id = {
	.bustype = 0x19,
	.vendor = 0x000,
	.product = 0x000,
};

static int events[] = {
	EV_KEY, KEY_HELP,
	EV_KEY, KEY_SETUP,
	EV_KEY, KEY_PROG1,
	EV_KEY, KEY_ROTATE_DISPLAY,
	EV_KEY, KEY_BRIGHTNESSDOWN,
	EV_KEY, KEY_BRIGHTNESSUP,
	EV_KEY, KEY_MEDIA,
	EV_KEY, KEY_UNKNOWN,
	EV_KEY, KEY_INFO,
	EV_SW, SW_TABLET_MODE,
	EV_SW, SW_DOCK,
	-1, -1,
};

TEST_DEVICE("wmi-hotkeys",
	.type = LITEST_HP_WMI_HOTKEYS,
	.features = LITEST_SWITCH,
	.interface = NULL,

	.name = "HP WMI hotkeys",
	.id = &input_id,
	.events = events,
	.absinfo = NULL,

	.udev_properties = {
		{ "ID_INPUT_SWITCH", "1" },
		{ NULL },
	}
)
