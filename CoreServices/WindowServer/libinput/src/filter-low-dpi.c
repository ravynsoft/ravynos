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

/*
 * Default parameters for pointer acceleration profiles.
 */

#define DEFAULT_THRESHOLD v_ms2us(0.4)		/* in units/us */
#define MINIMUM_THRESHOLD v_ms2us(0.2)		/* in units/us */
#define DEFAULT_ACCELERATION 2.0		/* unitless factor */
#define DEFAULT_INCLINE 1.1			/* unitless factor */

struct pointer_accelerator_low_dpi {
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
 * Custom acceleration function for mice < 1000dpi.
 * At slow motion, a single device unit causes a one-pixel movement.
 * The threshold/max accel depends on the DPI, the smaller the DPI the
 * earlier we accelerate and the higher the maximum acceleration is. Result:
 * at low speeds we get pixel-precision, at high speeds we get approx. the
 * same movement as a high-dpi mouse.
 *
 * Note: data fed to this function is in device units, not normalized.
 */
double
pointer_accel_profile_linear_low_dpi(struct motion_filter *filter,
				     void *data,
				     double speed_in, /* in device units (units/us) */
				     uint64_t time)
{
	struct pointer_accelerator_low_dpi *accel_filter =
		(struct pointer_accelerator_low_dpi *)filter;

	double max_accel = accel_filter->accel; /* unitless factor */
	double threshold = accel_filter->threshold; /* units/us */
	const double incline = accel_filter->incline;
	double dpi_factor = accel_filter->dpi/(double)DEFAULT_MOUSE_DPI;
	double factor; /* unitless */

	/* dpi_factor is always < 1.0, increase max_accel, reduce
	   the threshold so it kicks in earlier */
	max_accel /= dpi_factor;
	threshold *= dpi_factor;

	/* see pointer_accel_profile_linear for a long description */
	if (v_us2ms(speed_in) < 0.07)
		factor = 10 * v_us2ms(speed_in) + 0.3;
	else if (speed_in < threshold)
		factor = 1;
	else
		factor = incline * v_us2ms(speed_in - threshold) + 1;

	factor = min(max_accel, factor);

	return factor;
}

static inline double
calculate_acceleration_factor(struct pointer_accelerator_low_dpi *accel,
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
accelerator_filter_low_dpi(struct motion_filter *filter,
			   const struct device_float_coords *unaccelerated,
			   void *data, uint64_t time)
{
	struct pointer_accelerator_low_dpi *accel =
		(struct pointer_accelerator_low_dpi *) filter;

	/* Accelerate for device units and return device units */
	double accel_factor = calculate_acceleration_factor(accel,
							    unaccelerated,
							    data,
							    time);
	const struct normalized_coords normalized = {
		.x = accel_factor * unaccelerated->x,
		.y = accel_factor * unaccelerated->y,
	};
	return normalized;
}

static struct normalized_coords
accelerator_filter_noop(struct motion_filter *filter,
			const struct device_float_coords *unaccelerated,
			void *data, uint64_t time)
{
	const struct normalized_coords normalized = {
		.x = unaccelerated->x,
		.y = unaccelerated->y,
	};
	return normalized;
}

static void
accelerator_restart(struct motion_filter *filter,
		    void *data,
		    uint64_t time)
{
	struct pointer_accelerator_low_dpi *accel =
		(struct pointer_accelerator_low_dpi *) filter;

	trackers_reset(&accel->trackers, time);
}

static void
accelerator_destroy(struct motion_filter *filter)
{
	struct pointer_accelerator_low_dpi *accel =
		(struct pointer_accelerator_low_dpi *) filter;

	trackers_free(&accel->trackers);
	free(accel);
}

static bool
accelerator_set_speed(struct motion_filter *filter,
		      double speed_adjustment)
{
	struct pointer_accelerator_low_dpi *accel_filter =
		(struct pointer_accelerator_low_dpi *)filter;

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

static const struct motion_filter_interface accelerator_interface_low_dpi = {
	.type = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE,
	.filter = accelerator_filter_low_dpi,
	.filter_constant = accelerator_filter_noop,
	.filter_scroll = accelerator_filter_noop,
	.restart = accelerator_restart,
	.destroy = accelerator_destroy,
	.set_speed = accelerator_set_speed,
};

static struct pointer_accelerator_low_dpi *
create_default_filter(int dpi, bool use_velocity_averaging)
{
	struct pointer_accelerator_low_dpi *filter;

	filter = zalloc(sizeof *filter);
	filter->last_velocity = 0.0;

	trackers_init(&filter->trackers, use_velocity_averaging ? 16 : 2);

	filter->threshold = DEFAULT_THRESHOLD;
	filter->accel = DEFAULT_ACCELERATION;
	filter->incline = DEFAULT_INCLINE;
	filter->dpi = dpi;

	return filter;
}

struct motion_filter *
create_pointer_accelerator_filter_linear_low_dpi(int dpi, bool use_velocity_averaging)
{
	struct pointer_accelerator_low_dpi *filter;

	filter = create_default_filter(dpi, use_velocity_averaging);
	if (!filter)
		return NULL;

	filter->base.interface = &accelerator_interface_low_dpi;
	filter->profile = pointer_accel_profile_linear_low_dpi;

	return &filter->base;
}
