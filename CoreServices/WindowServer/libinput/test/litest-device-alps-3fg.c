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

struct alps {
	unsigned int first, second;
	unsigned int active_touches;
};

static bool
alps_create(struct litest_device *d)
{
	d->private = zalloc(sizeof(struct alps));
	return true; /* we want litest to create our device */
}

static bool
touch_down(struct litest_device *d, unsigned int slot, double x, double y)
{
	struct alps *alps = d->private;

	alps->active_touches++;

	if (alps->active_touches == 1)
		alps->first = slot;
	if (alps->active_touches == 2)
		alps->second = slot;

	/* This device announces 4 slots but only does two slots. So
	 * anything over 2 slots we just drop for events,
	 * litest takes care of BTN_TOOL_* for us. */
	if (alps->active_touches > 2) {
		/* Need to send SYN_REPORT to flush litest's BTN_TOOL_* updates */
		litest_event(d, EV_SYN, SYN_REPORT,  0);
		return true;
	}

	return false;
}

static bool
touch_move(struct litest_device *d, unsigned int slot, double x, double y)
{
	struct alps *alps = d->private;

	if (alps->active_touches > 2 &&
	    slot != alps->first &&
	    slot != alps->second)
		return true;

	return false;
}

static bool
touch_up(struct litest_device *d, unsigned int slot)
{
	struct alps *alps = d->private;

	assert(alps->active_touches >= 1);
	alps->active_touches--;

	/* Need to send SYN_REPORT to flush litest's BTN_TOOL_* updates */
	if (alps->active_touches > 2 &&
	    slot != alps->first &&
	    slot != alps->second) {
		litest_event(d, EV_SYN, SYN_REPORT,  0);
		return true;
	}

	if (slot == alps->first)
		alps->first = UINT_MAX;
	if (slot == alps->second)
		alps->second = UINT_MAX;

	return false;
}

static struct input_event down[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN  },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_SLOT, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event move[] = {
	{ .type = EV_ABS, .code = ABS_MT_SLOT, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN  },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct litest_device_interface interface = {
	.touch_down_events = down,
	.touch_move_events = move,

	.touch_down = touch_down,
	.touch_move = touch_move,
	.touch_up = touch_up,
};

static struct input_id input_id = {
	.bustype = 0x11,
	.vendor = 0x2,
	.product = 0x8,
	.version = 0x700,
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
	EV_KEY, BTN_TOOL_QUINTTAP,
	INPUT_PROP_MAX, INPUT_PROP_POINTER,
	-1, -1,
};

/* Note: we use the user-supplied resolution here, see #408 */
static struct input_absinfo absinfo[] = {
	{ ABS_X, 0, 4095, 0, 0, 37 },
	{ ABS_Y, 0, 2047, 0, 0, 26 },
	{ ABS_PRESSURE, 0, 127, 0, 0, 0 },
	{ ABS_MT_SLOT, 0, 3, 0, 0, 0 },
	{ ABS_MT_POSITION_X, 0, 4095, 0, 0, 37 },
	{ ABS_MT_POSITION_Y, 0, 2047, 0, 0, 26 },
	{ ABS_MT_TRACKING_ID, 0, 65535, 0, 0, 0 },
	{ .value = -1 }
};

TEST_DEVICE("alps-3fg",
	.type = LITEST_ALPS_3FG,
	.features = LITEST_TOUCHPAD | LITEST_BUTTON,
	.interface = &interface,

	.name = "AlpsPS/2 ALPS GlidePoint",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
	.create = alps_create,
)
