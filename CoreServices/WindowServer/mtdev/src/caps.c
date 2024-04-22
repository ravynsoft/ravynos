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

#include "state.h"

static const int SN_COORD = 250;	/* coordinate signal-to-noise ratio */
static const int SN_WIDTH = 100;	/* width signal-to-noise ratio */
static const int SN_ORIENT = 10;	/* orientation signal-to-noise ratio */

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)

static inline int getbit(const unsigned long *map, int key)
{
	return (map[key / LONG_BITS] >> (key % LONG_BITS)) & 0x01;
}

static int getabs(struct input_absinfo *abs, int key, int fd)
{
	int rc;
	SYSCALL(rc = ioctl(fd, EVIOCGABS(key), abs));
	return rc >= 0;
}

static struct input_absinfo *get_info(struct mtdev *dev, int code)
{
	int ix;

	if (code == ABS_MT_SLOT)
		return &dev->slot;
	if (!mtdev_is_absmt(code))
		return NULL;

	ix = mtdev_abs2mt(code);
	if (ix < LEGACY_API_NUM_MT_AXES)
		return &dev->abs[ix];
	else
		return &dev->state->ext_abs[ix - LEGACY_API_NUM_MT_AXES];
}

static void set_info(struct mtdev *dev, int code,
		     const unsigned long *bits, int fd)
{
	int has = getbit(bits, code) && getabs(get_info(dev, code), code, fd);
	mtdev_set_mt_event(dev, code, has);
}

static void default_fuzz(struct mtdev *dev, int code, int sn)
{
	struct input_absinfo *abs = get_info(dev, code);
	if (!mtdev_has_mt_event(dev, code) || abs->fuzz)
		return;
	abs->fuzz = (abs->maximum - abs->minimum) / sn;
}

static int mtdev_set_slots(struct mtdev *dev, int fd)
{
	struct { unsigned code; int values[DIM_FINGER]; } req;
	struct mtdev_state *state = dev->state;
	int rc, i, s, nslot;

	nslot = mtdev_get_abs_maximum(dev, ABS_MT_SLOT) + 1;

	for (i = 0; i < MT_ABS_SIZE; i++) {
		req.code = mtdev_mt2abs(i);
		if (!mtdev_has_mt_event(dev, req.code))
			continue;
		SYSCALL(rc = ioctl(fd, EVIOCGMTSLOTS(sizeof(req)), &req));
		if (rc < 0)
			return rc;
		for (s = 0; s < DIM_FINGER && s < nslot; s++)
			set_sval(&state->data[s], i, req.values[s]);
	}

	return 0;
}

int mtdev_configure(struct mtdev *dev, int fd)
{
	unsigned long absbits[NLONGS(ABS_MAX)];
	int rc, i;

	SYSCALL(rc = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbits)), absbits));
	if (rc < 0)
		return rc;

	set_info(dev, ABS_MT_SLOT, absbits, fd);
	for (i = 0; i < MT_ABS_SIZE; i++)
		set_info(dev, mtdev_mt2abs(i), absbits, fd);

	dev->has_mtdata = mtdev_has_mt_event(dev, ABS_MT_POSITION_X) &&
		mtdev_has_mt_event(dev, ABS_MT_POSITION_Y);

	if (!mtdev_has_mt_event(dev, ABS_MT_POSITION_X))
		getabs(get_info(dev, ABS_MT_POSITION_X), ABS_X, fd);
	if (!mtdev_has_mt_event(dev, ABS_MT_POSITION_Y))
		getabs(get_info(dev, ABS_MT_POSITION_Y), ABS_Y, fd);
	if (!mtdev_has_mt_event(dev, ABS_MT_PRESSURE))
		getabs(get_info(dev, ABS_MT_PRESSURE), ABS_PRESSURE, fd);

	if (!mtdev_has_mt_event(dev, ABS_MT_TRACKING_ID)) {
		mtdev_set_abs_minimum(dev, ABS_MT_TRACKING_ID, MT_ID_MIN);
		mtdev_set_abs_maximum(dev, ABS_MT_TRACKING_ID, MT_ID_MAX);
	}

	default_fuzz(dev, ABS_MT_POSITION_X, SN_COORD);
	default_fuzz(dev, ABS_MT_POSITION_Y, SN_COORD);
	default_fuzz(dev, ABS_MT_TOUCH_MAJOR, SN_WIDTH);
	default_fuzz(dev, ABS_MT_TOUCH_MINOR, SN_WIDTH);
	default_fuzz(dev, ABS_MT_WIDTH_MAJOR, SN_WIDTH);
	default_fuzz(dev, ABS_MT_WIDTH_MINOR, SN_WIDTH);
	default_fuzz(dev, ABS_MT_ORIENTATION, SN_ORIENT);

	if (dev->has_slot)
		mtdev_set_slots(dev, fd);

	return 0;
}

int mtdev_has_mt_event(const struct mtdev *dev, int code)
{
	int ix;

	if (code == ABS_MT_SLOT)
		return dev->has_slot;
	if (!mtdev_is_absmt(code))
		return 0;

	ix = mtdev_abs2mt(code);
	if (ix < LEGACY_API_NUM_MT_AXES)
		return dev->has_abs[ix];
	else
		return dev->state->has_ext_abs[ix - LEGACY_API_NUM_MT_AXES];
}

int mtdev_get_abs_minimum(const struct mtdev *dev, int code)
{
	const struct input_absinfo *abs = get_info((struct mtdev *)dev, code);
	return abs ? abs->minimum : 0;
}

int mtdev_get_abs_maximum(const struct mtdev *dev, int code)
{
	const struct input_absinfo *abs = get_info((struct mtdev *)dev, code);
	return abs ? abs->maximum : 0;
}

int mtdev_get_abs_fuzz(const struct mtdev *dev, int code)
{
	const struct input_absinfo *abs = get_info((struct mtdev *)dev, code);
	return abs ? abs->fuzz : 0;
}

int mtdev_get_abs_resolution(const struct mtdev *dev, int code)
{
	const struct input_absinfo *abs = get_info((struct mtdev *)dev, code);
	return abs ? abs->resolution : 0;
}

void mtdev_set_abs_minimum(struct mtdev *dev, int code, int value)
{
	struct input_absinfo *abs = get_info(dev, code);
	if (abs)
		abs->minimum = value;
}

void mtdev_set_mt_event(struct mtdev *dev, int code, int value)
{
	int ix;

	if (code == ABS_MT_SLOT)
		dev->has_slot = value;
	if (!mtdev_is_absmt(code))
		return;

	ix = mtdev_abs2mt(code);
	if (ix < LEGACY_API_NUM_MT_AXES)
		dev->has_abs[ix] = value;
	else
		dev->state->has_ext_abs[ix - LEGACY_API_NUM_MT_AXES] = value;
}

void mtdev_set_abs_maximum(struct mtdev *dev, int code, int value)
{
	struct input_absinfo *abs = get_info(dev, code);
	if (abs)
		abs->maximum = value;
}

void mtdev_set_abs_fuzz(struct mtdev *dev, int code, int value)
{
	struct input_absinfo *abs = get_info(dev, code);
	if (abs)
		abs->fuzz = value;
}

void mtdev_set_abs_resolution(struct mtdev *dev, int code, int value)
{
	struct input_absinfo *abs = get_info(dev, code);
	if (abs)
		abs->resolution = value;
}

