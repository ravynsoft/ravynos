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
#include <stdlib.h>
#include <stdint.h>

#include "filter.h"
#include "libinput-util.h"
#include "filter-private.h"

/* Trackpoint acceleration for the Lenovo x230. DO NOT TOUCH.
 * This code is only invoked on the X230 and is quite flimsy,
 * custom-designed to make this touchpad less terrible than the
 * out-of-the-box experience. The x230 was released in 2013, it's
 * not worth trying to optimize the code or de-duplicate the various
 * copy-pastes.
 */

/*
 * Default parameters for pointer acceleration profiles.
 */

#define DEFAULT_THRESHOLD v_ms2us(0.4)		/* in units/us */
#define MINIMUM_THRESHOLD v_ms2us(0.2)		/* in units/us */
#define DEFAULT_ACCELERATION 2.0		/* unitless factor */
#define DEFAULT_INCLINE 1.1			/* unitless factor */

/* for the Lenovo x230 custom accel. do not touch */
#define X230_THRESHOLD v_ms2us(0.4)		/* in units/us */
#define X230_ACCELERATION 2.0			/* unitless factor */
#define X230_INCLINE 1.1			/* unitless factor */
#define X230_MAGIC_SLOWDOWN 0.4			/* unitless */
#define X230_TP_MAGIC_LOW_RES_FACTOR 4.0	/* unitless */

struct pointer_accelerator_x230 {
	struct motion_filter base;

	accel_profile_func_t profile;

	double velocity;	/* units/us */
	double last_velocity;	/* units/us */

	struct pointer_trackers trackers;

	double threshold;	/* units/us */
	double accel;		/* unitless factor */
	double incline;		/* incline of the function */

	int dpi;
};

/**
 * Apply the acceleration profile to the given velocity.
 *
 * @param accel The acceleration filter
 * @param data Caller-specific data
 * @param velocity Velocity in device-units per µs
 * @param time Current time in µs
 *
 * @return A unitless acceleration factor, to be applied to the delta
 */
static double
acceleration_profile(struct pointer_accelerator_x230 *accel,
		     void *data, double velocity, uint64_t time)
{
	return accel->profile(&accel->base, data, velocity, time);
}

/**
 * Calculate the acceleration factor for our current velocity, averaging
 * between our current and the most recent velocity to smoothen out changes.
 *
 * @param accel The acceleration filter
 * @param data Caller-specific data
 * @param velocity Velocity in device-units per µs
 * @param last_velocity Previous velocity in device-units per µs
 * @param time Current time in µs
 *
 * @return A unitless acceleration factor, to be applied to the delta
 */
static double
calculate_acceleration(struct pointer_accelerator_x230 *accel,
		       void *data,
		       double velocity,
		       double last_velocity,
		       uint64_t time)
{
	double factor;

	/* Use Simpson's rule to calculate the average acceleration between
	 * the previous motion and the most recent. */
	factor = acceleration_profile(accel, data, velocity, time);
	factor += acceleration_profile(accel, data, last_velocity, time);
	factor += 4.0 *
		acceleration_profile(accel, data,
				     (last_velocity + velocity) / 2,
				     time);

	factor = factor / 6.0;

	return factor; /* unitless factor */
}

static struct normalized_coords
accelerator_filter_x230(struct motion_filter *filter,
			const struct device_float_coords *raw,
			void *data, uint64_t time)
{
	struct pointer_accelerator_x230 *accel =
		(struct pointer_accelerator_x230 *) filter;
	double accel_factor; /* unitless factor */
	struct normalized_coords accelerated;
	struct device_float_coords delta_normalized;
	struct normalized_coords unaccelerated;
	double velocity; /* units/us */

	/* This filter is a "do not touch me" filter. So the hack here is
	 * just to replicate the old behavior before filters switched to
	 * device-native dpi:
	 * 1) convert from device-native to 1000dpi normalized
	 * 2) run all calculation on 1000dpi-normalized data
	 * 3) apply accel factor no normalized data
	 */
	unaccelerated = normalize_for_dpi(raw, accel->dpi);
	delta_normalized.x = unaccelerated.x;
	delta_normalized.y = unaccelerated.y;

	trackers_feed(&accel->trackers, &delta_normalized, time);
	velocity = trackers_velocity(&accel->trackers, time);
	accel_factor = calculate_acceleration(accel,
					      data,
					      velocity,
					      accel->last_velocity,
					      time);
	accel->last_velocity = velocity;

	accelerated.x = accel_factor * delta_normalized.x;
	accelerated.y = accel_factor * delta_normalized.y;

	return accelerated;
}

static struct normalized_coords
accelerator_filter_constant_x230(struct motion_filter *filter,
				 const struct device_float_coords *unaccelerated,
				 void *data, uint64_t time)
{
	struct pointer_accelerator_x230 *accel =
		(struct pointer_accelerator_x230 *) filter;
	struct normalized_coords normalized;
	const double factor =
		X230_MAGIC_SLOWDOWN/X230_TP_MAGIC_LOW_RES_FACTOR;

	normalized = normalize_for_dpi(unaccelerated, accel->dpi);
	normalized.x = factor * normalized.x;
	normalized.y = factor * normalized.y;

	return normalized;
}

static void
accelerator_restart_x230(struct motion_filter *filter,
			 void *data,
			 uint64_t time)
{
	struct pointer_accelerator_x230 *accel =
		(struct pointer_accelerator_x230 *) filter;
	unsigned int offset;
	struct pointer_tracker *tracker;

	for (offset = 1; offset < accel->trackers.ntrackers; offset++) {
		tracker = trackers_by_offset(&accel->trackers, offset);
		tracker->time = 0;
		tracker->dir = 0;
		tracker->delta.x = 0;
		tracker->delta.y = 0;
	}

	tracker = trackers_by_offset(&accel->trackers, 0);
	tracker->time = time;
	tracker->dir = UNDEFINED_DIRECTION;
}

static void
accelerator_destroy_x230(struct motion_filter *filter)
{
	struct pointer_accelerator_x230 *accel =
		(struct pointer_accelerator_x230 *) filter;

	free(accel->trackers.trackers);
	free(accel);
}

static bool
accelerator_set_speed_x230(struct motion_filter *filter,
			   double speed_adjustment)
{
	struct pointer_accelerator_x230 *accel_filter =
		(struct pointer_accelerator_x230 *)filter;

	assert(speed_adjustment >= -1.0 && speed_adjustment <= 1.0);

	/* Note: the numbers below are nothing but trial-and-error magic,
	   don't read more into them other than "they mostly worked ok" */

	/* delay when accel kicks in */
	accel_filter->threshold = DEFAULT_THRESHOLD -
					v_ms2us(0.25) * speed_adjustment;
	if (accel_filter->threshold < MINIMUM_THRESHOLD)
		accel_filter->threshold = MINIMUM_THRESHOLD;

	/* adjust max accel factor */
	accel_filter->accel = DEFAULT_ACCELERATION + speed_adjustment * 1.5;

	/* higher speed -> faster to reach max */
	accel_filter->incline = DEFAULT_INCLINE + speed_adjustment * 0.75;

	filter->speed_adjustment = speed_adjustment;
	return true;
}

double
touchpad_lenovo_x230_accel_profile(struct motion_filter *filter,
				      void *data,
				      double speed_in, /* 1000dpi-units/µs */
				      uint64_t time)
{
	/* Those touchpads presents an actual lower resolution that what is
	 * advertised. We see some jumps from the cursor due to the big steps
	 * in X and Y when we are receiving data.
	 * Apply a factor to minimize those jumps at low speed, and try
	 * keeping the same feeling as regular touchpads at high speed.
	 * It still feels slower but it is usable at least */
	double factor; /* unitless */
	struct pointer_accelerator_x230 *accel_filter =
		(struct pointer_accelerator_x230 *)filter;

	double f1, f2; /* unitless */
	const double max_accel = accel_filter->accel *
				  X230_TP_MAGIC_LOW_RES_FACTOR; /* unitless factor */
	const double threshold = accel_filter->threshold /
				  X230_TP_MAGIC_LOW_RES_FACTOR; /* units/us */
	const double incline = accel_filter->incline * X230_TP_MAGIC_LOW_RES_FACTOR;

	/* Note: the magic values in this function are obtained by
	 * trial-and-error. No other meaning should be interpreted.
	 * The calculation is a compressed form of
	 * pointer_accel_profile_linear(), look at the git history of that
	 * function for an explanation of what the min/max/etc. does.
	 */
	speed_in *= X230_MAGIC_SLOWDOWN / X230_TP_MAGIC_LOW_RES_FACTOR;

	f1 = min(1, v_us2ms(speed_in) * 5);
	f2 = 1 + (v_us2ms(speed_in) - v_us2ms(threshold)) * incline;

	factor = min(max_accel, f2 > 1 ? f2 : f1);

	return factor * X230_MAGIC_SLOWDOWN / X230_TP_MAGIC_LOW_RES_FACTOR;
}

static const struct motion_filter_interface accelerator_interface_x230 = {
	.type = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE,
	.filter = accelerator_filter_x230,
	.filter_constant = accelerator_filter_constant_x230,
	.filter_scroll = accelerator_filter_constant_x230,
	.restart = accelerator_restart_x230,
	.destroy = accelerator_destroy_x230,
	.set_speed = accelerator_set_speed_x230,
};

/* The Lenovo x230 has a bad touchpad. This accel method has been
 * trial-and-error'd, any changes to it will require re-testing everything.
 * Don't touch this.
 */
struct motion_filter *
create_pointer_accelerator_filter_lenovo_x230(int dpi, bool use_velocity_averaging)
{
	struct pointer_accelerator_x230 *filter;

	filter = zalloc(sizeof *filter);
	filter->base.interface = &accelerator_interface_x230;
	filter->profile = touchpad_lenovo_x230_accel_profile;
	filter->last_velocity = 0.0;

	trackers_init(&filter->trackers, use_velocity_averaging ? 16 : 2);

	filter->threshold = X230_THRESHOLD;
	filter->accel = X230_ACCELERATION; /* unitless factor */
	filter->incline = X230_INCLINE; /* incline of the acceleration function */
	filter->dpi = dpi;

	return &filter->base;
}
