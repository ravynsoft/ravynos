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
#include <math.h>

#include "filter.h"
#include "libinput-util.h"
#include "filter-private.h"

#define MOTION_TIMEOUT		ms2us(1000)

struct normalized_coords
filter_dispatch(struct motion_filter *filter,
		const struct device_float_coords *unaccelerated,
		void *data, uint64_t time)
{
	return filter->interface->filter(filter, unaccelerated, data, time);
}

struct normalized_coords
filter_dispatch_constant(struct motion_filter *filter,
			 const struct device_float_coords *unaccelerated,
			 void *data, uint64_t time)
{
	return filter->interface->filter_constant(filter, unaccelerated, data, time);
}

struct normalized_coords
filter_dispatch_scroll(struct motion_filter *filter,
		       const struct device_float_coords *unaccelerated,
		       void *data, uint64_t time)
{
	return filter->interface->filter_scroll(filter, unaccelerated, data, time);
}

void
filter_restart(struct motion_filter *filter,
	       void *data, uint64_t time)
{
	if (filter->interface->restart)
		filter->interface->restart(filter, data, time);
}

void
filter_destroy(struct motion_filter *filter)
{
	if (!filter || !filter->interface->destroy)
		return;

	filter->interface->destroy(filter);
}

bool
filter_set_speed(struct motion_filter *filter,
		 double speed_adjustment)
{
	return filter->interface->set_speed(filter, speed_adjustment);
}

double
filter_get_speed(struct motion_filter *filter)
{
	return filter->speed_adjustment;
}

enum libinput_config_accel_profile
filter_get_type(struct motion_filter *filter)
{
	return filter->interface->type;
}

bool
filter_set_accel_config(struct motion_filter *filter,
			struct libinput_config_accel *accel_config)
{
	assert(filter_get_type(filter) == accel_config->profile);

	if (!filter->interface->set_accel_config)
		return false;

	return filter->interface->set_accel_config(filter, accel_config);
}

void
trackers_init(struct pointer_trackers *trackers, int ntrackers)
{
	trackers->trackers = zalloc(ntrackers *
				    sizeof(*trackers->trackers));
	trackers->ntrackers = ntrackers;
	trackers->cur_tracker = 0;
	trackers->smoothener = NULL;
}

void
trackers_free(struct pointer_trackers *trackers)
{
	free(trackers->trackers);
	pointer_delta_smoothener_destroy(trackers->smoothener);
}

void
trackers_reset(struct pointer_trackers *trackers,
	       uint64_t time)
{
	unsigned int offset;
	struct pointer_tracker *tracker;

	for (offset = 1; offset < trackers->ntrackers; offset++) {
		tracker = trackers_by_offset(trackers, offset);
		tracker->time = 0;
		tracker->dir = 0;
		tracker->delta.x = 0;
		tracker->delta.y = 0;
	}

	tracker = trackers_by_offset(trackers, 0);
	tracker->time = time;
	tracker->dir = UNDEFINED_DIRECTION;
}

void
trackers_feed(struct pointer_trackers *trackers,
	      const struct device_float_coords *delta,
	      uint64_t time)
{
	unsigned int i, current;
	struct pointer_tracker *ts = trackers->trackers;

	assert(trackers->ntrackers);

	for (i = 0; i < trackers->ntrackers; i++) {
		ts[i].delta.x += delta->x;
		ts[i].delta.y += delta->y;
	}

	current = (trackers->cur_tracker + 1) % trackers->ntrackers;
	trackers->cur_tracker = current;

	ts[current].delta.x = 0.0;
	ts[current].delta.y = 0.0;
	ts[current].time = time;
	ts[current].dir = device_float_get_direction(*delta);
}

struct pointer_tracker *
trackers_by_offset(struct pointer_trackers *trackers, unsigned int offset)
{
	unsigned int index =
		(trackers->cur_tracker + trackers->ntrackers - offset)
		% trackers->ntrackers;
	return &trackers->trackers[index];
}

static double
calculate_trackers_velocity(const struct pointer_tracker *tracker,
			    uint64_t time,
			    struct pointer_delta_smoothener *smoothener)
{
	uint64_t tdelta = time - tracker->time + 1;

	if (smoothener && tdelta < smoothener->threshold)
		tdelta = smoothener->value;

	return hypot(tracker->delta.x, tracker->delta.y) /
	       (double)tdelta; /* units/us */
}

static double
trackers_velocity_after_timeout(const struct pointer_tracker *tracker,
				struct pointer_delta_smoothener *smoothener)
{
	/* First movement after timeout needs special handling.
	 *
	 * When we trigger the timeout, the last event is too far in the
	 * past to use it for velocity calculation across multiple tracker
	 * values.
	 *
	 * Use the motion timeout itself to calculate the speed rather than
	 * the last tracker time. This errs on the side of being too fast
	 * for really slow movements but provides much more useful initial
	 * movement in normal use-cases (pause, move, pause, move)
	 */
	return calculate_trackers_velocity(tracker,
					   tracker->time + MOTION_TIMEOUT,
					   smoothener);
}

/**
 * Calculate the velocity based on the tracker data. Velocity is averaged
 * across multiple historical values, provided those values aren't "too
 * different" to our current one. That includes either being too far in the
 * past, moving into a different direction or having too much of a velocity
 * change between events.
 */
double
trackers_velocity(struct pointer_trackers *trackers, uint64_t time)
{
	const double MAX_VELOCITY_DIFF = v_ms2us(1); /* units/us */
	double result = 0.0;
	double initial_velocity = 0.0;

	unsigned int dir = trackers_by_offset(trackers, 0)->dir;

	/* Find least recent vector within a timelimit, maximum velocity diff
	 * and direction threshold. */
	for (unsigned int offset = 1; offset < trackers->ntrackers; offset++) {
		const struct pointer_tracker *tracker = trackers_by_offset(trackers, offset);

		/* Bug: time running backwards */
		if (tracker->time > time)
			break;

		/* Stop if too far away in time */
		if (time - tracker->time > MOTION_TIMEOUT) {
			if (offset == 1)
				result = trackers_velocity_after_timeout(
							  tracker,
							  trackers->smoothener);
			break;
		}

		double velocity = calculate_trackers_velocity(tracker,
							      time,
							      trackers->smoothener);

		/* Stop if direction changed */
		dir &= tracker->dir;
		if (dir == 0) {
			/* First movement after dirchange - velocity is that
			 * of the last movement */
			if (offset == 1)
				result = velocity;
			break;
		}

		/* Always average the first two events. On some touchpads
		 * where the first event is jumpy, this somewhat reduces
		 * pointer jumps on slow motions. */
		if (initial_velocity == 0.0 || offset <= 2) {
			result = initial_velocity = velocity;
		} else {
			/* Stop if velocity differs too much from initial */
			double velocity_diff = fabs(initial_velocity - velocity);
			if (velocity_diff > MAX_VELOCITY_DIFF)
				break;

			result = velocity;
		}
	}

	return result; /* units/us */
}

/**
 * Calculate the acceleration factor for our current velocity, averaging
 * between our current and the most recent velocity to smoothen out changes.
 *
 * @param accel The acceleration filter
 * @param data Caller-specific data
 * @param velocity Velocity - depending on the caller this may be in
 *		   device-units per µs or normalized per µs
 * @param last_velocity Previous velocity in device-units per µs
 * @param time Current time in µs
 *
 * @return A unitless acceleration factor, to be applied to the delta
 */
double
calculate_acceleration_simpsons(struct motion_filter *filter,
				accel_profile_func_t profile,
				void *data,
				double velocity,
				double last_velocity,
				uint64_t time)
{
	double factor;

	/* Use Simpson's rule to calculate the average acceleration between
	 * the previous motion and the most recent. */
	factor = profile(filter, data, velocity, time);
	factor += profile(filter, data, last_velocity, time);
	factor += 4.0 * profile(filter, data,
				(last_velocity + velocity) / 2,
				time);

	factor = factor / 6.0;

	return factor; /* unitless factor */
}
