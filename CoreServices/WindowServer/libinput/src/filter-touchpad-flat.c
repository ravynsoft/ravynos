/*
 * Copyright © 2006-2009 Simon Thum
 * Copyright © 2012 Jonas Ådahl
 * Copyright © 2014-2015 Red Hat, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "filter.h"
#include "libinput-util.h"
#include "filter-private.h"

#define TP_MAGIC_SLOWDOWN_FLAT 0.2968

struct touchpad_accelerator_flat {
	struct motion_filter base;

	double factor;
	int dpi;
};

static struct normalized_coords
accelerator_filter_touchpad_flat(struct motion_filter *filter,
				 const struct device_float_coords *unaccelerated,
				 void *data, uint64_t time)
{
	struct touchpad_accelerator_flat *accel =
		(struct touchpad_accelerator_flat *)filter;
	double factor; /* unitless factor */
	struct normalized_coords normalized;

	/* You want flat acceleration, you get flat acceleration for the
	 * device */
	factor = accel->factor;
	normalized = normalize_for_dpi(unaccelerated, accel->dpi);
	normalized.x = TP_MAGIC_SLOWDOWN_FLAT * factor * normalized.x;
	normalized.y = TP_MAGIC_SLOWDOWN_FLAT * factor * normalized.y;

	return normalized;
}

static struct normalized_coords
accelerator_filter_noop_touchpad_flat(struct motion_filter *filter,
				      const struct device_float_coords *unaccelerated,
				      void *data, uint64_t time)
{
	/* We map the unaccelerated flat filter to have the same behavior as
	 * the "accelerated" flat filter.
	 * The filter by definition is flat, i.e. it does not actually
	 * apply any acceleration (merely a constant factor) and we can assume
	 * that a user wants all mouse movement to have the same speed, mapped
	 * 1:1 to the input speed.
	 *
	 * Thus we apply the same factor to our non-accelerated motion - this way
	 * things like gestures end up having the same movement as
	 * pointer motion.
	 */
	return accelerator_filter_touchpad_flat(filter, unaccelerated, data, time);
}

static bool
accelerator_set_speed_touchpad_flat(struct motion_filter *filter,
				    double speed_adjustment)
{
	struct touchpad_accelerator_flat *accel_filter =
		(struct touchpad_accelerator_flat *)filter;

	assert(speed_adjustment >= -1.0 && speed_adjustment <= 1.0);

	accel_filter->factor = max(0.005, 1 + speed_adjustment);
	filter->speed_adjustment = speed_adjustment;

	return true;
}

static void
accelerator_destroy_touchpad_flat(struct motion_filter *filter)
{
	struct touchpad_accelerator_flat *accel =
		(struct touchpad_accelerator_flat *) filter;

	free(accel);
}

static const struct motion_filter_interface accelerator_interface_touchpad_flat = {
	.type = LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT,
	.filter = accelerator_filter_touchpad_flat,
	.filter_constant = accelerator_filter_noop_touchpad_flat,
	.filter_scroll = accelerator_filter_noop_touchpad_flat,
	.restart = NULL,
	.destroy = accelerator_destroy_touchpad_flat,
	.set_speed = accelerator_set_speed_touchpad_flat,
};

struct motion_filter *
create_pointer_accelerator_filter_touchpad_flat(int dpi)
{
	struct touchpad_accelerator_flat *filter;

	filter = zalloc(sizeof *filter);
	filter->base.interface = &accelerator_interface_touchpad_flat;
	filter->dpi = dpi;

	return &filter->base;
}
