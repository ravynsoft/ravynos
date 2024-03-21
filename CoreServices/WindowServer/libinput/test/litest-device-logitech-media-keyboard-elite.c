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
 * https://gitlab.freedesktop.org/libinput/libinput/-/issues/514
 */

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x46d,
	.product = 0x30f,
};

static int events[] = {
	EV_KEY, KEY_MUTE,
	EV_KEY, KEY_VOLUMEDOWN,
	EV_KEY, KEY_VOLUMEUP,
	EV_KEY, KEY_UNDO,
	EV_KEY, KEY_HELP,
	EV_KEY, KEY_CALC,
	EV_KEY, KEY_MAIL,
	EV_KEY, KEY_BOOKMARKS,
	EV_KEY, KEY_BACK,
	EV_KEY, KEY_FORWARD,
	EV_KEY, KEY_NEXTSONG,
	EV_KEY, KEY_PLAYPAUSE,
	EV_KEY, KEY_PREVIOUSSONG,
	EV_KEY, KEY_STOPCD,
	EV_KEY, KEY_REWIND,
	EV_KEY, KEY_CONFIG,
	EV_KEY, KEY_HOMEPAGE,
	EV_KEY, KEY_REDO,
	EV_KEY, KEY_FASTFORWARD,
	EV_KEY, KEY_PRINT,
	EV_KEY, KEY_SEARCH,
	EV_KEY, KEY_SAVE,
	EV_KEY, 319,
	EV_KEY, BTN_TOOL_QUINTTAP,
	EV_KEY, BTN_STYLUS3,
	EV_KEY, BTN_TOUCH,
	EV_KEY, BTN_STYLUS,
	EV_KEY, KEY_ZOOMIN,
	EV_KEY, KEY_ZOOMOUT,
	EV_KEY, KEY_ZOOMRESET,
	EV_KEY, KEY_WORDPROCESSOR,
	EV_KEY, KEY_SPREADSHEET,
	EV_KEY, KEY_PRESENTATION,
	EV_KEY, KEY_MESSENGER,

	EV_MSC, MSC_SCAN,
	-1, -1,
};

TEST_DEVICE("logitech-media-keyboard-elite",
	.type = LITEST_KEYBOARD_LOGITECH_MEDIA_KEYBOARD_ELITE,
	.features = LITEST_KEYS,
	.interface = NULL,

	.name = "Logitech Logitech USB Keyboard Consumer Control",
	.id = &input_id,
	.events = events,
	.absinfo = NULL,
)
