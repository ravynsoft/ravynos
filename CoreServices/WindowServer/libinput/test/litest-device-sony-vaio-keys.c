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

#include <assert.h>

#include "libinput-util.h"

#include "litest.h"
#include "litest-int.h"

/* Description taken from
 * https://gitlab.freedesktop.org/libinput/libinput/-/issues/515
 */

static struct input_id input_id = {
	.bustype = 0x10,
	.vendor = 0x104d,
	.product = 0x00,
};

static int events[] = {
	EV_KEY, KEY_UP,
	EV_KEY, KEY_DOWN,
	EV_KEY, KEY_MUTE,
	EV_KEY, KEY_VOLUMEDOWN,
	EV_KEY, KEY_VOLUMEUP,
	EV_KEY, KEY_HELP,
	EV_KEY, KEY_PROG1,
	EV_KEY, KEY_PROG2,
	EV_KEY, KEY_BACK,
	EV_KEY, KEY_EJECTCD,
	EV_KEY, KEY_F13,
	EV_KEY, KEY_F14,
	EV_KEY, KEY_F15,
	EV_KEY, KEY_F21,
	EV_KEY, KEY_PROG3,
	EV_KEY, KEY_PROG4,
	EV_KEY, KEY_SUSPEND,
	EV_KEY, KEY_CAMERA,
	EV_KEY, KEY_BRIGHTNESSDOWN,
	EV_KEY, KEY_BRIGHTNESSUP,
	EV_KEY, KEY_MEDIA,
	EV_KEY, KEY_SWITCHVIDEOMODE,
	EV_KEY, KEY_BLUETOOTH,
	EV_KEY, KEY_WLAN,
	EV_KEY, BTN_THUMB,
	EV_KEY, KEY_VENDOR,
	EV_KEY, KEY_FULL_SCREEN,
	EV_KEY, KEY_ZOOMIN,
	EV_KEY, KEY_ZOOMOUT,
	EV_KEY, KEY_FN,
	EV_KEY, KEY_FN_ESC,
	EV_KEY, KEY_FN_F8,
	EV_KEY, KEY_FN_F11,
	EV_KEY, KEY_FN_1,
	EV_KEY, KEY_FN_2,
	EV_KEY, KEY_FN_D,
	EV_KEY, KEY_FN_E,
	EV_KEY, KEY_FN_F,
	EV_KEY, KEY_FN_S,
	EV_KEY, KEY_FN_B,

	EV_MSC, MSC_SCAN,
	-1, -1,
};

TEST_DEVICE("sony-vaio-keys",
	.type = LITEST_SONY_VAIO_KEYS,
	.features = LITEST_KEYS,
	.interface = NULL,

	.name = "Sony Vaio Keys",
	.id = &input_id,
	.events = events,
	.absinfo = NULL,
)
