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
#include <math.h>

#include "filter.h"
#include "libinput-util.h"
#include "filter-private.h"

/* Once normalized, touchpads see the same acceleration as mice. that is
 * technically correct but subjectively wrong, we expect a touchpad to be a
 * lot slower than a mouse. Apply a magic factor to slow down all movements
 */
#define TP_MAGIC_SLOWDOWN 0.2968 /* unitless factor */

struct touchpad_accelerator {
	struct motion_filter base;

	accel_profile_func_t profile;

	double velocity;	/* units/us */
	double last_velocity;	/* units/us */

	struct pointer_trackers trackers;

	double threshold;	/* mm/s */
	double accel;		/* unitless factor */

	int dpi;

	double speed_factor;    /* factor based on speed setting */
};

/**
 * Calculate the acceleration factor for the given delta with the timestamp.
 *
 * @param accel The acceleration filter
 * @param unaccelerated The raw delta in the device's dpi
 * @param data Caller-specific data
 * @param time Current time in µs
 *
 * @return A unitless acceleration factor, to be applied to the delta
 */
static inline double
calculate_acceleration_factor(struct touchpad_accelerator *accel,
			      const struct device_float_coords *unaccelerated,
			      void *data,
			      uint64_t time)
{
	double velocity; /* units/us in device-native dpi*/
	double accel_factor;

	trackers_feed(&accel->trackers, unaccelerated, time);
	velocity = trackers_velocity(&accel->trackers, time);
	accel_factor = calculate_acceleration_simpsons(&accel->base,
						       accel->profile,
						       data,
						       velocity,
						       accel->last_velocity,
						       time);
	accel->last_velocity = velocity;

	return accel_factor;
}

static struct normalized_coords
accelerator_filter_touchpad(struct motion_filter *filter,
			    const struct device_float_coords *unaccelerated,
			    void *data, uint64_t time)
{
	struct touchpad_accelerator *accel =
		(struct touchpad_accelerator *) filter;

	/* Accelerate for device units, normalize afterwards */
	double accel_factor = calculate_acceleration_factor(accel,
							    unaccelerated,
							    data,
							    time);
	const struct device_float_coords accelerated =  {
		.x = unaccelerated->x * accel_factor,
		.y = unaccelerated->y * accel_factor,
	};

	return normalize_for_dpi(&accelerated, accel->dpi);
}

/* Maps the [-1, 1] speed setting into a constant acceleration
 * range. This isn't a linear scale, we keep 0 as the 'optimized'
 * mid-point and scale down to 0.05 for setting -1 and up to 5 for
 * setting 1. On the premise that if you want a faster cursor, it
 * doesn't matter as much whether you have 0.56789 or 0.56790,
 * but for lower settings it does because you may lose movements.
 * *shrug*.
 */
static inline double
speed_factor(double s)
{
	return pow(s + 1, 2.38) * 0.95 + 0.05;
}

static bool
touchpad_accelerator_set_speed(struct motion_filter *filter,
		      double speed_adjustment)
{
	struct touchpad_accelerator *accel_filter =
		(struct touchpad_accelerator *)filter;

	assert(speed_adjustment >= -1.0 && speed_adjustment <= 1.0);

	filter->speed_adjustment = speed_adjustment;
	accel_filter->speed_factor = speed_factor(speed_adjustment);

	return true;
}

static struct normalized_coords
touchpad_constant_filter(struct motion_filter *filter,
			 const struct device_float_coords *unaccelerated,
			 void *data, uint64_t time)
{
	struct touchpad_accelerator *accel =
		(struct touchpad_accelerator *)filter;
	struct normalized_coords normalized;
	/* We need to use the same baseline here as the accelerated code,
	 * otherwise our unaccelerated speed is different to the accelerated
	 * speed on the plateau.
	 *
	 * This is a hack, the baseline should be incorporated into the
	 * TP_MAGIC_SLOWDOWN so we only have one number here but meanwhile
	 * this will do.
	 */
	const double baseline = 0.9;

	normalized = normalize_for_dpi(unaccelerated, accel->dpi);
	normalized.x = baseline * TP_MAGIC_SLOWDOWN * normalized.x;
	normalized.y = baseline * TP_MAGIC_SLOWDOWN * normalized.y;

	return normalized;
}

static void
touchpad_accelerator_restart(struct motion_filter *filter,
			     void *data,
			     uint64_t time)
{
	struct touchpad_accelerator *accel =
		(struct touchpad_accelerator *) filter;

	trackers_reset(&accel->trackers, time);
}

static void
touchpad_accelerator_destroy(struct motion_filter *filter)
{
	struct touchpad_accelerator *accel =
		(struct touchpad_accelerator *) filter;

	trackers_free(&accel->trackers);
	free(accel);
}

double
touchpad_accel_profile_linear(struct motion_filter *filter,
			      void *data,
			      double speed_in, /* in device units/µs */
			      uint64_t time)
{
	struct touchpad_accelerator *accel_filter =
		(struct touchpad_accelerator *)filter;
	const double threshold = accel_filter->threshold; /* mm/s */
	const double baseline = 0.9;
	double factor; /* unitless */

	/* Convert to mm/s because that's something one can understand */
	speed_in = v_us2s(speed_in) * 25.4/accel_filter->dpi;

	/*
	   Our acceleration function calculates a factor to accelerate input
	   deltas with. The function is a double incline with a plateau,
	   with a rough shape like this:

	  accel
	 factor
	   ^         ______
	   |        )
	   |  _____)
	   | /
	   |/
	   +-------------> speed in

	   Except the second incline is a curve, but well, asciiart.

	   The first incline is a linear function in the form
		   y = ax + b
		   where y is speed_out
		         x is speed_in
			 a is the incline of acceleration
			 b is minimum acceleration factor
	   for speeds up to the lower threshold, we decelerate, down to 30%
	   of input speed.
		   hence 1 = a * 7 + 0.3
		       0.7 = a * 7  => a := 0.1
		   deceleration function is thus:
			y = 0.1x + 0.3

	   The first plateau is the baseline.

	   The second incline is a curve up, based on magic numbers
	   obtained by trial-and-error.

	   Above the second incline we have another plateau because
	   by then you're moving so fast that extra acceleration doesn't
	   help.

	  Note:
	  * The minimum threshold is a result of trial-and-error and
	    has no other special meaning.
	  * 0.3 is chosen simply because it is above the Nyquist frequency
	    for subpixel motion within a pixel.
	*/

	if (speed_in < 7.0) {
		factor = min(baseline, 0.1 * speed_in + 0.3);
	/* up to the threshold, we keep factor 1, i.e. 1:1 movement */
	} else if (speed_in < threshold) {
		factor = baseline;
	} else {

	/* Acceleration function above the threshold is a curve up
	   to four times the threshold, because why not.

	   Don't assume anything about the specific numbers though, this was
	   all just trial and error by tweaking numbers here and there, then
	   the formula was optimized doing basic maths.

	   You could replace this with some other random formula that gives
	   the same numbers and it would be just as correct.

	 */
		const double upper_threshold = threshold * 4.0;
		speed_in = min(speed_in, upper_threshold);

		factor = 0.0025 * (speed_in/threshold) * (speed_in - threshold) + baseline;
	}

	factor *= accel_filter->speed_factor;
	return factor * TP_MAGIC_SLOWDOWN;
}

static const struct motion_filter_interface accelerator_interface_touchpad = {
	.type = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE,
	.filter = accelerator_filter_touchpad,
	.filter_constant = touchpad_constant_filter,
	.filter_scroll = touchpad_constant_filter,
	.restart = touchpad_accelerator_restart,
	.destroy = touchpad_accelerator_destroy,
	.set_speed = touchpad_accelerator_set_speed,
};

struct motion_filter *
create_pointer_accelerator_filter_touchpad(int dpi,
	uint64_t event_delta_smooth_threshold,
	uint64_t event_delta_smooth_value,
	bool use_velocity_averaging)
{
	struct touchpad_accelerator *filter;

	filter = zalloc(sizeof *filter);
	filter->last_velocity = 0.0;

	trackers_init(&filter->trackers, use_velocity_averaging ? 16 : 2);

	filter->threshold = 130;
	filter->dpi = dpi;

	filter->base.interface = &accelerator_interface_touchpad;
	filter->profile = touchpad_accel_profile_linear;
	filter->trackers.smoothener = pointer_delta_smoothener_create(event_delta_smooth_threshold, event_delta_smooth_value);

	return &filter->base;
}
