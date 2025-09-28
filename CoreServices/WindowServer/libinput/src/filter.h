/*
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

#ifndef FILTER_H
#define FILTER_H

#include "config.h"

#include <stdbool.h>
#include <stdint.h>

#include "libinput-private.h"

struct motion_filter;

/**
 * Accelerate the given coordinates.
 * Takes a set of unaccelerated deltas and accelerates them based on the
 * current and previous motion.
 *
 * This is a superset of filter_dispatch_constant()
 *
 * @param filter The device's motion filter
 * @param unaccelerated The unaccelerated delta in the device's dpi
 * resolution as specified during filter creation. If a device has uneven
 * resolution for x and y, one axis needs to be scaled to match the
 * originally provided resolution.
 * @param data Custom data
 * @param time The time of the delta
 *
 * @return A set of normalized coordinates that can be used for pixel
 * movement. The normalized coordinates are scaled to the default dpi range,
 * i.e. regardless of the resolution of the underlying device, the returned
 * values always reflect a 1000dpi mouse.
 *
 * @see filter_dispatch_constant
 */
struct normalized_coords
filter_dispatch(struct motion_filter *filter,
		const struct device_float_coords *unaccelerated,
		void *data, uint64_t time);

/**
 * Apply constant motion filters, but no acceleration.
 *
 * Takes a set of unaccelerated deltas and applies any constant filters to
 * it but does not accelerate the delta in the conventional sense.
 *
 * @param filter The device's motion filter
 * @param unaccelerated The unaccelerated delta in the device's dpi
 * resolution as specified during filter creation. If a device has uneven
 * resolution for x and y, one axis needs to be scaled to match the
 * originally provided resolution.
 * @param data Custom data
 * @param time The time of the delta
 *
 * @see filter_dispatch
 */
struct normalized_coords
filter_dispatch_constant(struct motion_filter *filter,
			 const struct device_float_coords *unaccelerated,
			 void *data, uint64_t time);

/**
 * Apply a scroll filter.
 * Depending on the device, and the acceleration profile,
 * this filter allows the user to accelerate the scroll movement.
 *
 * Takes a set of unaccelerated deltas and applies the scroll filter to it.
 *
 * @param filter The device's motion filter
 * @param unaccelerated The unaccelerated delta in the device's dpi
 * resolution as specified during filter creation. If a device has uneven
 * resolution for x and y, one axis needs to be scaled to match the
 * originally provided resolution.
 * @param data Custom data
 * @param time The time of the delta
 *
 * @see filter_dispatch
 */
struct normalized_coords
filter_dispatch_scroll(struct motion_filter *filter,
		       const struct device_float_coords *unaccelerated,
		       void *data, uint64_t time);

void
filter_restart(struct motion_filter *filter,
	       void *data, uint64_t time);

void
filter_destroy(struct motion_filter *filter);

bool
filter_set_speed(struct motion_filter *filter,
		 double speed);
double
filter_get_speed(struct motion_filter *filter);

enum libinput_config_accel_profile
filter_get_type(struct motion_filter *filter);

typedef double (*accel_profile_func_t)(struct motion_filter *filter,
				       void *data,
				       double velocity,
				       uint64_t time);

bool
filter_set_accel_config(struct motion_filter *filter,
		        struct libinput_config_accel *accel_config);

/* Pointer acceleration types */
struct motion_filter *
create_pointer_accelerator_filter_flat(int dpi);

struct motion_filter *
create_pointer_accelerator_filter_linear(int dpi, bool use_velocity_averaging);

struct motion_filter *
create_pointer_accelerator_filter_linear_low_dpi(int dpi, bool use_velocity_averaging);

struct motion_filter *
create_pointer_accelerator_filter_touchpad(int dpi,
	uint64_t event_delta_smooth_threshold,
	uint64_t event_delta_smooth_value,
	bool use_velocity_averaging);

struct motion_filter *
create_pointer_accelerator_filter_touchpad_flat(int dpi);

struct motion_filter *
create_pointer_accelerator_filter_lenovo_x230(int dpi, bool use_velocity_averaging);

struct motion_filter *
create_pointer_accelerator_filter_trackpoint(double multiplier, bool use_velocity_averaging);

struct motion_filter *
create_pointer_accelerator_filter_trackpoint_flat(double multiplier);

struct motion_filter *
create_pointer_accelerator_filter_tablet(int xres, int yres);

struct motion_filter *
create_custom_accelerator_filter(void);

/*
 * Pointer acceleration profiles.
 */

double
pointer_accel_profile_linear_low_dpi(struct motion_filter *filter,
				     void *data,
				     double speed_in,
				     uint64_t time);
double
pointer_accel_profile_linear(struct motion_filter *filter,
			     void *data,
			     double speed_in,
			     uint64_t time);
double
touchpad_accel_profile_linear(struct motion_filter *filter,
			      void *data,
			      double speed_in,
			      uint64_t time);
double
touchpad_lenovo_x230_accel_profile(struct motion_filter *filter,
				      void *data,
				      double speed_in,
				      uint64_t time);
double
trackpoint_accel_profile(struct motion_filter *filter,
			 void *data,
			 double velocity,
			 uint64_t time);
double
custom_accel_profile_fallback(struct motion_filter *filter,
			      void *data,
			      double speed_in,
			      uint64_t time);
double
custom_accel_profile_motion(struct motion_filter *filter,
			    void *data,
			    double speed_in,
			    uint64_t time);
double
custom_accel_profile_scroll(struct motion_filter *filter,
			    void *data,
			    double speed_in,
			    uint64_t time);
#endif /* FILTER_H */
