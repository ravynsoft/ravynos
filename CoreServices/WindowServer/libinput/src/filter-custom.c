/*
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
#include "filter-private.h"

#define MOTION_TIMEOUT ms2us(1000)
#define FIRST_MOTION_TIME_INTERVAL ms2us(7) /* random but good enough interval for very first event */

struct custom_accel_function {
	uint64_t last_time;
	double step;
	size_t npoints;
	double points[];
};

static struct custom_accel_function *
create_custom_accel_function(double step, const double *points, size_t npoints)
{
	if (npoints < LIBINPUT_ACCEL_NPOINTS_MIN ||
	    npoints > LIBINPUT_ACCEL_NPOINTS_MAX)
		return NULL;

	if (step <= 0 || step > LIBINPUT_ACCEL_STEP_MAX)
		return NULL;

	for (size_t idx = 0; idx < npoints; idx++) {
		if (points[idx] < LIBINPUT_ACCEL_POINT_MIN_VALUE ||
		    points[idx] > LIBINPUT_ACCEL_POINT_MAX_VALUE)
			return NULL;
	}

	struct custom_accel_function *cf = zalloc(sizeof(*cf) + npoints * sizeof(*points));
	cf->last_time = 0;
	cf->step = step;
	cf->npoints = npoints;
	memcpy(cf->points, points, sizeof(*points) * npoints);

	return cf;
}

static void
custom_accel_function_destroy(struct custom_accel_function *cf)
{
	if (cf == NULL)
		return;

	free(cf);
}

static double
custom_accel_function_calculate_speed(struct custom_accel_function *cf,
				      const struct device_float_coords *unaccelerated,
				      uint64_t time)
{
	/* Although most devices have a constant polling rate, and for fast
	 * movements these distances do represent the actual speed,
	 * for slow movements it is not the case.
	 *
	 * Since all devices have a finite resolution, real world events
	 * for a slow smooth movement could look like:
	 *   Event 1 - (0,  1) - time 0
	 *   Event 2 - (0,  0) - time 7  - filtered (zero event)
	 *   Event 3 - (1,  0) - time 14
	 *   Event 4 - (0,  0) - time 21 - filtered (zero event)
	 *   Event 5 - (0,  0) - time 28 - filtered (zero event)
	 *   Event 6 - (0,  1) - time 35
	 *
	 * Not taking the time into account would mean interpreting those events as:
	 *   Move 1 unit over 7 ms
	 *   Pause for 7 ms
	 *   Move 1 unit over 7 ms
	 *   Pause for 14 ms
	 *   Move 1 unit over 7ms
	 *
	 * Where in reality this was one smooth movement without pauses,
	 * so after normalizing for time we get:
	 *   Move 1 unit over 7 ms
	 *   Move 1 unit over 14 ms
	 *   Move 1 unit over 21ms
	 *
	 * which should give us better speed estimation.
	 */

	/* calculate speed based on time passed since last event */
	double distance = hypot(unaccelerated->x, unaccelerated->y);
	/* handle first event in a motion */
	if (time - cf->last_time > MOTION_TIMEOUT)
		cf->last_time = time - FIRST_MOTION_TIME_INTERVAL;

	double dt = us2ms_f(time - cf->last_time);
	double speed = distance / dt; /* speed is in device-units per ms */
	cf->last_time = time;

	return speed;
}

static double
custom_accel_function_profile(struct custom_accel_function *cf,
			      double speed_in)
{
	size_t npoints = cf->npoints;
	double step = cf->step;
	double *points = cf->points;

	/* calculate the index of the first point used for interpolation */
	size_t i = speed_in / step;

	/* if speed is greater than custom curve's max speed,
	   use last 2 points for linear extrapolation
	   (same calculation as linear interpolation) */
	i = min(i, npoints - 2);

	/* the 2 points used for linear interpolation */
	double x0 = step * i;
	double x1 = step * (i + 1);
	double y0 = points[i];
	double y1 = points[i + 1];

	/* linear interpolation */
	double speed_out = (y0 * (x1 - speed_in) + y1 * (speed_in - x0)) / step;

	/* We moved (dx, dy) device units within the last N ms. This gives us a
	 * given speed S in units/ms, that's our accel input. Our curve says map
	 * that speed S to some other speed S'.
	 *
	 * Our device delta is represented by the vector, that vector needs to
	 * be modified to represent our intended speed.
	 *
	 * Example: we moved a delta of 7 over the last 7ms. Our speed is
	 * thus 1 u/ms, our out speed is 2 u/ms because we want to double our
	 * speed (points: [0.0, 2.0]). Our delta must thus be 14 - factor of 2,
	 * or out-speed/in-speed.
	 *
	 * Example: we moved a delta of 1 over the last 7ms. Our input speed is
	 * 1/7 u/ms, our out speed is 1/7ms because we set up a flat accel
	 * curve (points: [0.0, 1.0]). Our delta must thus be 1 - factor of 1,
	 * or out-speed/in-speed.
	 *
	 * Example: we moved a delta of 1 over the last 21ms. Our input speed is
	 * 1/21 u/ms, our out speed is 1u/ms because we set up a fixed-speed
	 * curve (points: [1.0, 1.0]). Our delta must thus be 21 - factor of 21,
	 * or out-speed/in-speed.
	 *
	 * Example: we moved a delta of 21 over the last 7ms. Our input speed is
	 * 3 u/ms, our out speed is 1u/ms because we set up a fixed-speed
	 * curve (points: [1.0, 1.0]). Our delta must thus be 7 - factor of 1/3,
	 * or out-speed/in-speed.
	 */

	/* calculate the acceleration factor based on the user desired speed out */
	double accel_factor = speed_out / speed_in;

	return accel_factor;
}

static struct normalized_coords
custom_accel_function_filter(struct custom_accel_function *cf,
			     const struct device_float_coords *unaccelerated,
			     uint64_t time)
{
	double speed = custom_accel_function_calculate_speed(cf, unaccelerated, time);

	double accel_factor = custom_accel_function_profile(cf, speed);

	struct normalized_coords accelerated = {
		.x = unaccelerated->x * accel_factor,
		.y = unaccelerated->y * accel_factor,
	};

	return accelerated;
}

struct custom_accelerator {
	struct motion_filter base;
	struct {
		struct custom_accel_function *fallback;
		struct custom_accel_function *motion;
		struct custom_accel_function *scroll;
	} funcs;
};

static struct custom_accel_function *
custom_accelerator_get_custom_function(struct custom_accelerator *f,
				       enum libinput_config_accel_type accel_type)
{
	switch (accel_type) {
	case LIBINPUT_ACCEL_TYPE_FALLBACK:
		return f->funcs.fallback;
	case LIBINPUT_ACCEL_TYPE_MOTION:
		return f->funcs.motion ? f->funcs.motion : f->funcs.fallback;
	case LIBINPUT_ACCEL_TYPE_SCROLL:
		return f->funcs.scroll ? f->funcs.scroll : f->funcs.fallback;
	}

	return f->funcs.fallback;
}

static double
custom_accelerator_profile(enum libinput_config_accel_type accel_type,
			   struct motion_filter *filter,
			   double speed_in)
{
	struct custom_accelerator *f = (struct custom_accelerator *)filter;
	struct custom_accel_function *cf;

	cf = custom_accelerator_get_custom_function(f, accel_type);

	return custom_accel_function_profile(cf, speed_in);
}

static struct normalized_coords
custom_accelerator_filter(enum libinput_config_accel_type accel_type,
			  struct motion_filter *filter,
			  const struct device_float_coords *unaccelerated,
			  uint64_t time)
{
	struct custom_accelerator *f = (struct custom_accelerator *)filter;
	struct custom_accel_function *cf;

	cf = custom_accelerator_get_custom_function(f, accel_type);

	return custom_accel_function_filter(cf, unaccelerated, time);
}

static void
custom_accelerator_restart(struct motion_filter *filter,
			   void *data,
			   uint64_t time)
{
	/* noop, this function has no effect in the custom interface */
}

static void
custom_accelerator_destroy(struct motion_filter *filter)
{
	struct custom_accelerator *f =
		(struct custom_accelerator *)filter;

	/* destroy all custom movement functions */
	custom_accel_function_destroy(f->funcs.fallback);
	custom_accel_function_destroy(f->funcs.motion);
	custom_accel_function_destroy(f->funcs.scroll);
	free(f);
}

static bool
custom_accelerator_set_speed(struct motion_filter *filter,
			     double speed_adjustment)
{
	assert(speed_adjustment >= -1.0 && speed_adjustment <= 1.0);

	/* noop, this function has no effect in the custom interface */

	return true;
}

static bool
custom_accelerator_set_accel_config(struct motion_filter *filter,
				    struct libinput_config_accel *config)
{
	struct custom_accelerator *f =
		(struct custom_accelerator *)filter;

	struct custom_accel_function *fallback = NULL,
				     *motion = NULL,
				     *scroll = NULL;

	if (config->custom.fallback) {
		fallback = create_custom_accel_function(config->custom.fallback->step,
							config->custom.fallback->points,
							config->custom.fallback->npoints);
		if (!fallback)
			goto out;
	}

	if (config->custom.motion) {
		motion = create_custom_accel_function(config->custom.motion->step,
						      config->custom.motion->points,
						      config->custom.motion->npoints);
		if (!motion)
			goto out;
	}

	if (config->custom.scroll) {
		scroll = create_custom_accel_function(config->custom.scroll->step,
						      config->custom.scroll->points,
						      config->custom.scroll->npoints);
		if (!scroll)
			goto out;
	}

	custom_accel_function_destroy(f->funcs.fallback);
	custom_accel_function_destroy(f->funcs.motion);
	custom_accel_function_destroy(f->funcs.scroll);

	f->funcs.fallback = fallback;
	f->funcs.motion = motion;
	f->funcs.scroll = scroll;

	return true;

out:
	custom_accel_function_destroy(fallback);
	custom_accel_function_destroy(motion);
	custom_accel_function_destroy(scroll);

	return false;
}

/* custom profiles and filters for the different accel types: */

double
custom_accel_profile_fallback(struct motion_filter *filter,
			      void *data,
			      double speed_in,
			      uint64_t time)
{
	return custom_accelerator_profile(LIBINPUT_ACCEL_TYPE_FALLBACK,
					  filter,
					  speed_in);
}

static struct normalized_coords
custom_accelerator_filter_fallback(struct motion_filter *filter,
				   const struct device_float_coords *unaccelerated,
				   void *data,
				   uint64_t time)
{
	return custom_accelerator_filter(LIBINPUT_ACCEL_TYPE_FALLBACK,
					 filter,
					 unaccelerated,
					 time);
}

double
custom_accel_profile_motion(struct motion_filter *filter,
			    void *data,
			    double speed_in,
			    uint64_t time)
{
	return custom_accelerator_profile(LIBINPUT_ACCEL_TYPE_MOTION,
					  filter,
					  speed_in);
}

static struct normalized_coords
custom_accelerator_filter_motion(struct motion_filter *filter,
				 const struct device_float_coords *unaccelerated,
				 void *data,
				 uint64_t time)
{
	return custom_accelerator_filter(LIBINPUT_ACCEL_TYPE_MOTION,
					 filter,
					 unaccelerated,
					 time);
}

double
custom_accel_profile_scroll(struct motion_filter *filter,
			    void *data,
			    double speed_in,
			    uint64_t time)
{
	return custom_accelerator_profile(LIBINPUT_ACCEL_TYPE_SCROLL,
					  filter,
					  speed_in);
}

static struct normalized_coords
custom_accelerator_filter_scroll(struct motion_filter *filter,
				 const struct device_float_coords *unaccelerated,
				 void *data,
				 uint64_t time)
{
	return custom_accelerator_filter(LIBINPUT_ACCEL_TYPE_SCROLL,
					 filter,
					 unaccelerated,
					 time);
}

struct motion_filter_interface custom_accelerator_interface = {
	.type = LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM,
	.filter = custom_accelerator_filter_motion,
	.filter_constant = custom_accelerator_filter_fallback,
	.filter_scroll = custom_accelerator_filter_scroll,
	.restart = custom_accelerator_restart,
	.destroy = custom_accelerator_destroy,
	.set_speed = custom_accelerator_set_speed,
	.set_accel_config = custom_accelerator_set_accel_config,
};

struct motion_filter *
create_custom_accelerator_filter(void)
{
	struct custom_accelerator *f = zalloc(sizeof(*f));

	/* the unit function by default, speed in = speed out,
	   i.e. no acceleration */
	const double default_step = 1.0;
	const double default_points[2] = {0.0, 1.0};

	/* initialize default acceleration, used as fallback */
	f->funcs.fallback = create_custom_accel_function(default_step,
							 default_points,
							 ARRAY_LENGTH(default_points));
	/* Don't initialize other acceleration functions. Those will be
	   initialized if the user sets their points, otherwise the fallback
	   acceleration function is used */

	f->base.interface = &custom_accelerator_interface;

	return &f->base;
}
