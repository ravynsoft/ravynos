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
	.bustype = 0x18,
	.vendor = 0x123,
	.product = 0x456,
};

static int events[] = {
	/* buttons are needed - the unreliable quirk removes SW_TABLET_MODE
	 * so we'd end up with a device with no seat caps and that won't get
	 * added */
	EV_KEY, BTN_LEFT,
	EV_KEY, BTN_RIGHT,
	EV_SW, SW_TABLET_MODE,
	-1, -1,
};

static const char quirk_file[] =
"[litest unreliable tablet mode switch]\n"
"MatchName=litest Unreliable Tablet Mode Switch device\n"
"ModelTabletModeSwitchUnreliable=1\n";

TEST_DEVICE("tablet-mode-switch-unreliable",
	.type = LITEST_TABLET_MODE_UNRELIABLE,
	.features = LITEST_SWITCH,
	.interface = NULL,

	.name = "Unreliable Tablet Mode Switch device",
	.id = &input_id,
	.events = events,
	.absinfo = NULL,

	.quirk_file = quirk_file,
	.udev_properties = {
		{ "ID_INPUT_SWITCH", "1" },
		{ NULL },
	}
)

