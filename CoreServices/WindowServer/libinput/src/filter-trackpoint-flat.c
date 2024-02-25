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

struct trackpoint_flat_accelerator {
	struct motion_filter base;

	double speed_factor;
	double multiplier;
};

static struct normalized_coords
trackpoint_flat_filter(struct motion_filter *filter,
		       const struct device_float_coords *unaccelerated,
		       void *data, uint64_t time)
{
	struct trackpoint_flat_accelerator *accel_filter =
		(struct trackpoint_flat_accelerator *) filter;
	struct normalized_coords accelerated;

	double factor = accel_filter->speed_factor;
	double multiplier = accel_filter->multiplier;
	accelerated.x = factor * multiplier * unaccelerated->x;
	accelerated.y = factor * multiplier * unaccelerated->y;

	return accelerated;
}

static struct normalized_coords
trackpoint_flat_filter_noop(struct motion_filter *filter,
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
	 * things like button scrolling end up having the same movement as
	 * pointer motion.
	 */
	return trackpoint_flat_filter(filter, unaccelerated, data, time);
}

/* Maps the [-1, 1] speed setting into a constant acceleration
 * range. This isn't a linear scale, we keep 0 as the 'optimized'
 * mid-point and scale down to 0 for setting -1 and up to 5 for
 * setting 1. On the premise that if you want a faster cursor, it
 * doesn't matter as much whether you have 0.56789 or 0.56790,
 * but for lower settings it does because you may lose movements.
 * *shrug*.
 *
 * Magic numbers calculated by MyCurveFit.com, data points were
 *  0.0 0.0
 *  0.1 0.1 (because we need 4 points)
 *  1   1
 *  2   5
 *
 *  This curve fits nicely into the range necessary.
 */
static inline double
speed_factor(double s)
{
	s += 1; /* map to [0, 2] */
	return 435837.2 + (0.04762636 - 435837.2)/(1 + pow(s/240.4549,
							   2.377168));
}

static bool
trackpoint_flat_set_speed(struct motion_filter *filter,
			  double speed_adjustment)
{
	struct trackpoint_flat_accelerator *accel_filter =
		(struct trackpoint_flat_accelerator *) filter;

	assert(speed_adjustment >= -1.0 && speed_adjustment <= 1.0);

	filter->speed_adjustment = speed_adjustment;
	accel_filter->speed_factor = speed_factor(speed_adjustment);

	return true;
}

static void
trackpoint_flat_destroy(struct motion_filter *filter)
{
	struct trackpoint_flat_accelerator *accel_filter =
		(struct trackpoint_flat_accelerator *) filter;

	free(accel_filter);
}

static struct motion_filter_interface accelerator_interface_flat = {
	.type = LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT,
	.filter = trackpoint_flat_filter,
	.filter_constant = trackpoint_flat_filter_noop,
	.filter_scroll = trackpoint_flat_filter_noop,
	.restart = NULL,
	.destroy = trackpoint_flat_destroy,
	.set_speed = trackpoint_flat_set_speed,
};

struct motion_filter *
create_pointer_accelerator_filter_trackpoint_flat(double multiplier)
{
	struct trackpoint_flat_accelerator *filter;

	filter = zalloc(sizeof *filter);
	filter->base.interface = &accelerator_interface_flat;
	filter->multiplier = multiplier;

	return &filter->base;
}
