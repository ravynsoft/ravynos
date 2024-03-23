/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright (c) 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "cairo-perf.h"
#include <assert.h>

#define STEP	5

static void path (cairo_t *cr, int width, int height)
{
    int i;

    cairo_rectangle (cr, 0, 0, width, height);
    cairo_clip (cr);

    cairo_translate (cr, width/2, height/2);
    cairo_rotate (cr, M_PI/4);
    cairo_translate (cr, -width/2, -height/2);

    for (i = 0; i < width; i += STEP) {
	cairo_rectangle (cr, i, -2, 1, height+4);
	cairo_rectangle (cr, -2, i, width+4, 1);
    }
}

static void clip (cairo_t *cr, int width, int height)
{
    int i, j;

    for (j = 0; j < height; j += 2*STEP) {
	for (i = 0; i < width; i += 2*STEP)
	    cairo_rectangle (cr, i, j, STEP, STEP);

	j += 2*STEP;
	for (i = 0; i < width; i += 2*STEP)
	    cairo_rectangle (cr, i+STEP/2, j, STEP, STEP);
    }

    cairo_clip (cr);
}

static cairo_time_t
draw (cairo_t *cr, int width, int height, int loops)
{
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);

    cairo_perf_timer_start ();
    while (loops--) {
	cairo_save (cr);
	clip (cr, width, height);
	path (cr, width, height);
	cairo_fill (cr);
	cairo_restore (cr);
    }
    cairo_perf_timer_stop ();

    cairo_restore (cr);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
disjoint_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "disjoint", NULL);
}

void
disjoint (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    if (! cairo_perf_can_run (perf, "disjoint", NULL))
	return;

    cairo_perf_run (perf, "disjoint", draw, NULL);
}
