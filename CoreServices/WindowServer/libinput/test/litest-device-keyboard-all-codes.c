/*
 * Copyright © 2013 Red Hat, Inc.
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

#define NAME "All event codes keyboard"

static bool all_codes_create(struct litest_device *d);

static struct input_id input_id = {
	.bustype = 0x11,
	.vendor = 0x1,
	.product = 0x1,
};

TEST_DEVICE("keyboard-all-codes",
	.type = LITEST_KEYBOARD_ALL_CODES,
	.features = LITEST_KEYS,
	.interface = NULL,
	.create = all_codes_create,

	.name = NAME,
	.id = &input_id,
	.events = NULL,
	.absinfo = NULL,
)

static bool
all_codes_create(struct litest_device *d)
{
	int events[KEY_MAX * 2 + 2];
	int code, idx;

	for (idx = 0, code = 0; code < KEY_MAX; code++) {
		const char *name = libevdev_event_code_get_name(EV_KEY, code);

		if (name && strneq(name, "BTN_", 4))
			continue;

		events[idx++] = EV_KEY;
		events[idx++] = code;
	}
	events[idx++] = -1;
	events[idx++] = -1;

	d->uinput = litest_create_uinput_device_from_description(NAME,
								 &input_id,
								 NULL,
								 events);
	return false;
}
