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

struct tablet_accelerator_flat {
	struct motion_filter base;

	double factor;
	int xres, yres;
	double xres_scale, /* 1000dpi : tablet res */
	       yres_scale; /* 1000dpi : tablet res */
};

static inline struct normalized_coords
tablet_accelerator_filter_flat_mouse(struct tablet_accelerator_flat *filter,
				     const struct device_float_coords *units)
{
	struct normalized_coords accelerated;

	/*
	   Tablets are high res (Intuos 4 is 5080 dpi) and unmodified deltas
	   are way too high. Slow it down to the equivalent of a 1000dpi
	   mouse. The ratio of that is:
		ratio = 1000/(resolution_per_mm * 25.4)

	   i.e. on the Intuos4 it's a ratio of ~1/5.
	 */

	accelerated.x = units->x * filter->xres_scale;
	accelerated.y = units->y * filter->yres_scale;

	accelerated.x *= filter->factor;
	accelerated.y *= filter->factor;

	return accelerated;
}

static struct normalized_coords
tablet_accelerator_filter_flat_pen(struct tablet_accelerator_flat *filter,
				   const struct device_float_coords *units)
{
	struct normalized_coords accelerated;

	/* Tablet input is in device units, output is supposed to be in
	 * logical pixels roughly equivalent to a mouse/touchpad.
	 *
	 * This is a magical constant found by trial and error. On a 96dpi
	 * screen 0.4mm of movement correspond to 1px logical pixel which
	 * is almost identical to the tablet mapped to screen in absolute
	 * mode. Tested on a Intuos5, other tablets may vary.
	 */
       const double DPI_CONVERSION = 96.0/25.4 * 2.5; /* unitless factor */
       struct normalized_coords mm;

       mm.x = 1.0 * units->x/filter->xres;
       mm.y = 1.0 * units->y/filter->yres;
       accelerated.x = mm.x * filter->factor * DPI_CONVERSION;
       accelerated.y = mm.y * filter->factor * DPI_CONVERSION;

       return accelerated;
}

static struct normalized_coords
tablet_accelerator_filter_flat(struct motion_filter *filter,
			       const struct device_float_coords *units,
			       void *data, uint64_t time)
{
	struct tablet_accelerator_flat *accel_filter =
		(struct tablet_accelerator_flat *)filter;
	struct libinput_tablet_tool *tool = (struct libinput_tablet_tool*)data;
	enum libinput_tablet_tool_type type;
	struct normalized_coords accel;

	type = libinput_tablet_tool_get_type(tool);

	switch (type) {
	case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:
	case LIBINPUT_TABLET_TOOL_TYPE_LENS:
		accel = tablet_accelerator_filter_flat_mouse(accel_filter,
							     units);
		break;
	default:
		accel = tablet_accelerator_filter_flat_pen(accel_filter,
							   units);
		break;
	}

	return accel;
}

static bool
tablet_accelerator_set_speed(struct motion_filter *filter,
			     double speed_adjustment)
{
	struct tablet_accelerator_flat *accel_filter =
		(struct tablet_accelerator_flat *)filter;

	assert(speed_adjustment >= -1.0 && speed_adjustment <= 1.0);

	accel_filter->factor = speed_adjustment + 1.0;

	return true;
}

static void
tablet_accelerator_destroy(struct motion_filter *filter)
{
	struct tablet_accelerator_flat *accel_filter =
		(struct tablet_accelerator_flat *)filter;

	free(accel_filter);
}

static const struct motion_filter_interface accelerator_interface_tablet = {
	.type = LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT,
	.filter = tablet_accelerator_filter_flat,
	.filter_constant = NULL,
	.filter_scroll = NULL,
	.restart = NULL,
	.destroy = tablet_accelerator_destroy,
	.set_speed = tablet_accelerator_set_speed,
};

static struct tablet_accelerator_flat *
create_tablet_filter_flat(int xres, int yres)
{
	struct tablet_accelerator_flat *filter;

	filter = zalloc(sizeof *filter);
	filter->factor = 1.0;
	filter->xres = xres;
	filter->yres = yres;
	filter->xres_scale = DEFAULT_MOUSE_DPI/(25.4 * xres);
	filter->yres_scale = DEFAULT_MOUSE_DPI/(25.4 * yres);

	return filter;
}

struct motion_filter *
create_pointer_accelerator_filter_tablet(int xres, int yres)
{
	struct tablet_accelerator_flat *filter;

	filter = create_tablet_filter_flat(xres, yres);
	if (!filter)
		return NULL;

	filter->base.interface = &accelerator_interface_tablet;

	return &filter->base;
}
