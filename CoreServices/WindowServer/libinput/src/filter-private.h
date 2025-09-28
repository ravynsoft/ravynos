/*
 * Copyright © 2012 Jonas Ådahl
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

#ifndef FILTER_PRIVATE_H
#define FILTER_PRIVATE_H

#include "config.h"

#include "filter.h"

struct motion_filter_interface {
	enum libinput_config_accel_profile type;
	struct normalized_coords (*filter)(
			   struct motion_filter *filter,
			   const struct device_float_coords *unaccelerated,
			   void *data, uint64_t time);
	struct normalized_coords (*filter_constant)(
			   struct motion_filter *filter,
			   const struct device_float_coords *unaccelerated,
			   void *data, uint64_t time);
	struct normalized_coords (*filter_scroll)(
			   struct motion_filter *filter,
			   const struct device_float_coords *unaccelerated,
			   void *data, uint64_t time);
	void (*restart)(struct motion_filter *filter,
			void *data,
			uint64_t time);
	void (*destroy)(struct motion_filter *filter);
	bool (*set_speed)(struct motion_filter *filter,
			  double speed_adjustment);
	bool (*set_accel_config)(struct motion_filter *filter,
				 struct libinput_config_accel *accel_config);
};

struct motion_filter {
	double speed_adjustment; /* normalized [-1, 1] */
	const struct motion_filter_interface *interface;
};

struct pointer_tracker {
	struct device_float_coords delta; /* delta to most recent event */
	uint64_t time;  /* us */
	uint32_t dir;
};

/* For smoothing timestamps from devices with unreliable timing */
struct pointer_delta_smoothener {
	uint64_t threshold;
	uint64_t value;
};

static inline struct pointer_delta_smoothener *
pointer_delta_smoothener_create(uint64_t event_delta_smooth_threshold,
				uint64_t event_delta_smooth_value)
{
	struct pointer_delta_smoothener *s = zalloc(sizeof(*s));
	s->threshold = event_delta_smooth_threshold;
	s->value = event_delta_smooth_value;
	return s;
}

static inline void
pointer_delta_smoothener_destroy(struct pointer_delta_smoothener *smoothener)
{
	free(smoothener);
}

struct pointer_trackers {
	struct pointer_tracker *trackers;
	size_t ntrackers;
	unsigned int cur_tracker;

	struct pointer_delta_smoothener *smoothener;
};

void trackers_init(struct pointer_trackers *trackers, int ntrackers);
void trackers_free(struct pointer_trackers *trackers);

void
trackers_reset(struct pointer_trackers *trackers,
	       uint64_t time);
void
trackers_feed(struct pointer_trackers *trackers,
	      const struct device_float_coords *delta,
	      uint64_t time);

struct pointer_tracker *
trackers_by_offset(struct pointer_trackers *trackers, unsigned int offset);

double
trackers_velocity(struct pointer_trackers *trackers, uint64_t time);

double
calculate_acceleration_simpsons(struct motion_filter *filter,
				accel_profile_func_t profile,
				void *data,
				double velocity,
				double last_velocity,
				uint64_t time);

/* Convert speed/velocity from units/us to units/ms */
static inline double
v_us2ms(double units_per_us)
{
	return units_per_us * 1000.0;
}

static inline double
v_us2s(double units_per_us)
{
	return units_per_us * 1000000.0;
}

/* Convert speed/velocity from units/ms to units/us */
static inline double
v_ms2us(double units_per_ms)
{
	return units_per_ms/1000.0;
}

static inline struct normalized_coords
normalize_for_dpi(const struct device_float_coords *coords, int dpi)
{
	struct normalized_coords norm;

	norm.x = coords->x * DEFAULT_MOUSE_DPI/dpi;
	norm.y = coords->y * DEFAULT_MOUSE_DPI/dpi;

	return norm;
}

#endif
