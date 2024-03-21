/*
 * Copyright © 2017 Red Hat, Inc
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
	.vendor = 0x1,
	.product = 0x1,
};

static int events[] = {
	EV_SW, SW_LID,
	EV_SW, SW_TABLET_MODE,
	EV_KEY, KEY_POWER,
	EV_KEY, KEY_VOLUMEUP,
	EV_KEY, KEY_VOLUMEDOWN,
	EV_KEY, KEY_POWER,
	-1, -1,
};

static const char quirk_file[] =
"[litest gpio quirk]\n"
"MatchName=litest gpio-keys\n"
"AttrLidSwitchReliability=reliable\n";

TEST_DEVICE("gpio-keys",
	.type = LITEST_GPIO_KEYS,
	.features = LITEST_SWITCH,
	.interface = NULL,

	.name = "gpio-keys",
	.id = &input_id,
	.events = events,
	.absinfo = NULL,

	.quirk_file = quirk_file,
	.udev_properties = {
		{ "ID_INPUT_SWITCH", "1" },
		{ NULL },
	}
)
