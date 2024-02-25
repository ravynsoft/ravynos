/*
 * Copyright © 2015 Red Hat, Inc.
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

#define PROTOCOL_A_MAX_SLOTS 10

struct protocolA_device {
	struct slot {
		bool active;
		int x, y;
		int tracking_id;
	} slots[PROTOCOL_A_MAX_SLOTS];
	unsigned int nslots;
};

static bool
protocolA_create(struct litest_device *d)
{
	struct protocolA_device *dev = zalloc(sizeof(*dev));

	dev->nslots = PROTOCOL_A_MAX_SLOTS;

	d->private = dev;

	return true; /* we want litest to create our device */
}

static bool
protocolA_down(struct litest_device *d, unsigned int slot, double x, double y)
{
	struct protocolA_device *dev = d->private;
	static int tracking_id;
	bool first = true;

	assert(slot <= PROTOCOL_A_MAX_SLOTS);

	x = litest_scale(d, ABS_X, x);
	y = litest_scale(d, ABS_Y, y);

	for (unsigned int i = 0; i < dev->nslots; i++) {
		struct slot *s = &dev->slots[i];

		if (slot == i) {
			assert(!s->active);
			s->active = true;
			s->x = x;
			s->y = y;
			s->tracking_id = ++tracking_id;
		}
		if (!s->active)
			continue;

		if (first) {
			litest_event(d, EV_ABS, ABS_X, s->x);
			litest_event(d, EV_ABS, ABS_Y, s->y);
			first = false;
		}

		litest_event(d, EV_ABS, ABS_MT_TRACKING_ID, s->tracking_id);
		litest_event(d, EV_ABS, ABS_MT_POSITION_X, s->x);
		litest_event(d, EV_ABS, ABS_MT_POSITION_Y, s->y);
		litest_event(d, EV_SYN, SYN_MT_REPORT, 0);
	}

	if (!first) {
		litest_event(d, EV_KEY, BTN_TOUCH, 1);
		litest_event(d, EV_SYN, SYN_REPORT, 0);
	}

	return true; /* we handled the event */
}

static bool
protocolA_move(struct litest_device *d, unsigned int slot, double x, double y)
{
	struct protocolA_device *dev = d->private;
	bool first = true;

	assert(slot <= PROTOCOL_A_MAX_SLOTS);

	x = litest_scale(d, ABS_X, x);
	y = litest_scale(d, ABS_Y, y);

	for (unsigned int i = 0; i < dev->nslots; i++) {
		struct slot *s = &dev->slots[i];

		if (slot == i) {
			assert(s->active);
			s->x = x;
			s->y = y;
		}
		if (!s->active)
			continue;

		if (first) {
			litest_event(d, EV_ABS, ABS_X, s->x);
			litest_event(d, EV_ABS, ABS_X, s->y);
			first = false;
		}

		litest_event(d, EV_ABS, ABS_MT_TRACKING_ID, s->tracking_id);
		litest_event(d, EV_ABS, ABS_MT_POSITION_X, s->x);
		litest_event(d, EV_ABS, ABS_MT_POSITION_Y, s->y);
		litest_event(d, EV_SYN, SYN_MT_REPORT, 0);
	}

	if (!first)
		litest_event(d, EV_SYN, SYN_REPORT, 0);

	return true; /* we handled the event */
}

static bool
protocolA_up(struct litest_device *d, unsigned int slot)
{
	struct protocolA_device *dev = d->private;
	bool first = true;

	assert(slot <= PROTOCOL_A_MAX_SLOTS);

	for (unsigned int i = 0; i < dev->nslots; i++) {
		struct slot *s = &dev->slots[i];

		if (slot == i) {
			assert(s->active);
			s->active = false;
			litest_event(d, EV_ABS, ABS_MT_TRACKING_ID, s->tracking_id);
			litest_event(d, EV_SYN, SYN_MT_REPORT, 0);
		}
		if (!s->active)
			continue;

		if (first) {
			litest_event(d, EV_ABS, ABS_X, s->x);
			litest_event(d, EV_ABS, ABS_X, s->y);
			first = false;
		}

		litest_event(d, EV_ABS, ABS_MT_TRACKING_ID, s->tracking_id);
		litest_event(d, EV_ABS, ABS_MT_POSITION_X, s->x);
		litest_event(d, EV_ABS, ABS_MT_POSITION_Y, s->y);
		litest_event(d, EV_SYN, SYN_MT_REPORT, 0);

	}

	if (first)
		litest_event(d, EV_KEY, BTN_TOUCH, 0);
	litest_event(d, EV_SYN, SYN_REPORT, 0);

	return true; /* we handled the event */
}

static struct litest_device_interface interface = {
	.touch_down = protocolA_down,
	.touch_move = protocolA_move,
	.touch_up = protocolA_up,
};

static struct input_absinfo absinfo[] = {
	{ ABS_X, 0, 32767, 0, 0, 0 },
	{ ABS_Y, 0, 32767, 0, 0, 0 },
	{ ABS_MT_POSITION_X, 0, 32767, 0, 0, 0 },
	{ ABS_MT_POSITION_Y, 0, 32767, 0, 0, 0 },
	{ ABS_MT_PRESSURE, 0, 1, 0, 0, 0 },
	{ .value = -1 },
};

static struct input_id input_id = {
	.bustype = 0x18,
	.vendor = 0xeef,
	.product = 0x20,
};

static int events[] = {
	EV_KEY, BTN_TOUCH,
	INPUT_PROP_MAX, INPUT_PROP_DIRECT,
	-1, -1,
};

TEST_DEVICE("protocol-a",
	.type = LITEST_PROTOCOL_A_SCREEN,
	.features = LITEST_PROTOCOL_A|LITEST_TOUCH,
	.create = protocolA_create,

	.interface = &interface,

	.name = "Protocol A touch screen",
	.id = &input_id,
	.events = events,
	.absinfo = absinfo,
)
