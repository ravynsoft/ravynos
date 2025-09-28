/*****************************************************************************
 *
 * mtdev - Multitouch Protocol Translation Library (MIT license)
 *
 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
 * Copyright (C) 2010 Canonical Ltd.
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
 *
 ****************************************************************************/

#ifndef MTDEV_STATE_H
#define MTDEV_STATE_H

#include "iobuf.h"
#include "evbuf.h"

/*
 * struct mtdev_slot - represents the state of an input MT slot
 * @abs: current values of ABS_MT axes for this slot
 */
struct mtdev_slot {
	int touch_major;
	int touch_minor;
	int width_major;
	int width_minor;
	int orientation;
	int position_x;
	int position_y;
	int tool_type;
	int blob_id;
	int tracking_id;
	int pressure;
	int distance;
};

static inline int get_sval(const struct mtdev_slot *slot, int ix)
{
	return (&slot->touch_major)[ix];
}

static inline void set_sval(struct mtdev_slot *slot, int ix, int value)
{
	(&slot->touch_major)[ix] = value;
}

/*
 * struct mtdev_state - MT slot parsing
 * @inbuf: input event buffer
 * @outbuf: output event buffer
 * @data: array of scratch slot data
 * @used: bitmask of currently used slots
 * @slot: slot currently being modified
 * @lastid: last used tracking id
 */
struct mtdev_state {

	int has_ext_abs[MT_ABS_SIZE - LEGACY_API_NUM_MT_AXES];
	struct input_absinfo ext_abs[MT_ABS_SIZE - LEGACY_API_NUM_MT_AXES];

	struct mtdev_iobuf iobuf;
	struct mtdev_evbuf inbuf;
	struct mtdev_evbuf outbuf;
	struct mtdev_slot data[DIM_FINGER];

	bitmask_t used;
	bitmask_t slot;
	bitmask_t lastid;
};

#endif
