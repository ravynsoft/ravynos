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

static struct input_event proximity_in_events[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_KEY, .code = LITEST_BTN_TOOL_AUTO, .value = 1 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event proximity_out_events[] = {
	{ .type = EV_KEY, .code = LITEST_BTN_TOOL_AUTO, .value = 0 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event motion_events[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static bool
proximity_in(struct litest_device *d,
	     unsigned int tool_type,
	     double *x, double *y,
	     struct axis_replacement *axes)
{
	/* nothing special needed for the pen tool, so let litest handle
	 * this */
	if (tool_type == BTN_TOOL_PEN)
		return false;

	/* a non-pen tool requires the pen to be in proximity as well.  */
	int sx = litest_scale(d, ABS_X, *x);
	int sy = litest_scale(d, ABS_Y, *y);
	litest_event(d, EV_ABS, ABS_X, sx);
	litest_event(d, EV_ABS, ABS_X, sy);
	litest_event(d, EV_KEY, BTN_TOOL_PEN, 1);
	litest_event(d, EV_SYN, SYN_REPORT, 0);

	/* litest will append the proximity_in_events if we return false,
	 * including the right tool event */
	return false;
}

static bool
proximity_out(struct litest_device *d, unsigned int tool_type)
{
	/* a non-pen tool requires the pen to go out of proximity as well.
	 * litest will append the proximity_out_events if we return false
	 */
	if (tool_type != BTN_TOOL_PEN)
		litest_event(d, EV_KEY, BTN_TOOL_PEN, 0);

	return false;
}

static int
get_axis_default(struct litest_device *d, unsigned int evcode, int32_t *value)
{
	switch (evcode) {
	case ABS_PRESSURE:
		*value = 100;
		return 0;
	}
	return 1;
}

static struct litest_device_interface interface = {
	.tablet_proximity_in_events = proximity_in_events,
	.tablet_proximity_out_events = proximity_out_events,
	.tablet_motion_events = motion_events,
	.tablet_proximity_in = proximity_in,
	.tablet_proximity_out = proximity_out,

	.get_axis_default = get_axis_default,
};

static struct input_absinfo absinfo[] = {
	{ ABS_X, 0, 18176, 0, 0, 62 },
	{ ABS_Y, 0, 10240, 0, 0, 62 },
	{ ABS_PRESSURE, 0, 4096, 0, 0, 0 },
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x18,
	.vendor = 0x4f3,
	.product = 0x23b9,
	.version = 0x100,
};

/* Note: this tablet is one that sets both BTN_TOOL_PEN and BTN_TOOL_RUBBER,
 * see https://gitlab.freedesktop.org/libinput/libinput/-/issues/259
 * The one in the issue isn't the exact same model, but only the pid and x/y
 * axis max differs differs.
 */
static int events[] = {
	EV_KEY, BTN_TOOL_PEN,
	EV_KEY, BTN_TOOL_RUBBER,
	EV_KEY, BTN_TOUCH,
	EV_KEY, BTN_STYLUS,
	EV_MSC, MSC_SCAN,
	-1, -1,
};

TEST_DEVICE("elan-tablet",
	.type = LITEST_ELAN_TABLET,
	.features = LITEST_TABLET,
	.interface = &interface,

	.name = "ELAN2514:00 04F3:23B9",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
)
