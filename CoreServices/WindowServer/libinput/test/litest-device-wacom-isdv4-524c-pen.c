/*
 * Copyright © 2019 Red Hat, Inc.
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

struct priv {
	unsigned int tool;
};

static bool
create(struct litest_device *d)
{
	d->private = zalloc(sizeof(struct priv));
	return true; /* we want litest to create our device */
}

static struct input_event proximity_in[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_KEY, .code = LITEST_BTN_TOOL_AUTO, .value = 1 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event proximity_out[] = {
	{ .type = EV_KEY, .code = LITEST_BTN_TOOL_AUTO, .value = 0 },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static struct input_event motion[] = {
	{ .type = EV_ABS, .code = ABS_X, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_Y, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_ABS, .code = ABS_PRESSURE, .value = LITEST_AUTO_ASSIGN },
	{ .type = EV_SYN, .code = SYN_REPORT, .value = 0 },
	{ .type = -1, .code = -1 },
};

static int
get_axis_default(struct litest_device *d, unsigned int evcode, int32_t *value)
{
	switch (evcode) {
	case ABS_PRESSURE:
		*value = 4000;
		return 0;
	case ABS_TILT_X:
	case ABS_TILT_Y:
		abort();
	}
	return 1;
}

static bool prox_in(struct litest_device *d,
		  unsigned int tool_type,
		  double *x, double *y,
		  struct axis_replacement *axes)
{
	struct priv *priv = d->private;
	priv->tool = tool_type;

	return false;
}

static bool prox_out(struct litest_device *d, unsigned int tool_type)
{
	struct priv *priv = d->private;
	priv->tool = 0;

	return false;
}

static bool
tip_down(struct litest_device *d,
	 double *x, double *y,
	 struct axis_replacement *axes)
{
	litest_event(d, EV_KEY, BTN_TOOL_PEN, 1);
	return false; /* use the default behavior otherwise */
}

static bool
tip_up(struct litest_device *d,
	 double* x, double *y,
	 struct axis_replacement *axes)
{
	struct priv *priv = d->private;
	if (priv->tool != BTN_TOOL_PEN)
		litest_event(d, EV_KEY, BTN_TOOL_PEN, 0);
	return false; /* use the default behavior otherwise */
}

static struct litest_device_interface interface = {
	.tablet_proximity_in_events = proximity_in,
	.tablet_proximity_out_events = proximity_out,
	.tablet_motion_events = motion,

	.tablet_proximity_in = prox_in,
	.tablet_proximity_out = prox_out,
	.tablet_tip_down = tip_down,
	.tablet_tip_up = tip_up,

	.get_axis_default = get_axis_default,
};

static struct input_absinfo absinfo[] = {
	{ ABS_X, 0, 30931, 0, 0, 100 },
	{ ABS_Y, 0, 17399, 0, 0, 100 },
	/* This pen has tilt, but doesn't send events */
	{ ABS_TILT_X, -90, 90, 0, 0, 57 },
	{ ABS_TILT_Y, -90, 90, 0, 0, 57 },
	{ ABS_PRESSURE, 0, 4095, 0, 0, 0 },
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x3,
	.vendor = 0x2d1f, /* Note: this is Wacom's Android VID */
	.product = 0x524c,
};

static int events[] = {
	EV_KEY, BTN_TOOL_PEN,
	EV_KEY, BTN_TOOL_RUBBER,
	EV_KEY, BTN_TOUCH,
	EV_KEY, BTN_STYLUS,
	EV_KEY, BTN_STYLUS2,
	INPUT_PROP_MAX, INPUT_PROP_DIRECT,
	-1, -1,
};

TEST_DEVICE("wacom-isdv4-524c-tablet",
	.type = LITEST_WACOM_ISDV4_524C_PEN,
	.features = LITEST_TABLET|LITEST_HOVER,
	.interface = &interface,

	.name = "Wacom Co.,Ltd. Pen and multitouch sensor Stylus",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
	.create = create,
)
