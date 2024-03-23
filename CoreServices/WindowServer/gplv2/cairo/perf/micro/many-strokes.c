/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-perf.h"

static uint32_t state;

static double
uniform_random (double minval, double maxval)
{
    static uint32_t const poly = 0x9a795537U;
    uint32_t n = 32;
    while (n-->0)
	state = 2*state < state ? (2*state ^ poly) : 2*state;
    return minval + state * (maxval - minval) / 4294967296.0;
}

static cairo_time_t
do_many_strokes_ha (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double h = floor (uniform_random (0, height)) + .5;
	cairo_move_to (cr, floor (uniform_random (0, width)), h);
	cairo_line_to (cr, ceil (uniform_random (0, width)), h);
    }

    cairo_set_line_width (cr, 1.);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_strokes_h (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double h = uniform_random (0, height);
	cairo_move_to (cr, uniform_random (0, width), h);
	cairo_line_to (cr, uniform_random (0, width), h);
    }

    cairo_set_line_width (cr, 1.);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_strokes_va (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double v = floor (uniform_random (0, width)) + .5;
	cairo_move_to (cr, v, floor (uniform_random (0, height)));
	cairo_line_to (cr, v, ceil (uniform_random (0, height)));
    }

    cairo_set_line_width (cr, 1.);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_strokes_v (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double v = uniform_random (0, width);
	cairo_move_to (cr, v, uniform_random (0, height));
	cairo_line_to (cr, v, uniform_random (0, height));
    }

    cairo_set_line_width (cr, 1.);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_strokes (cairo_t *cr, int width, int height, int loops)
{
    int count;

    /* lots and lots of overlapping strokes */
    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	cairo_line_to (cr,
		       uniform_random (0, width),
		       uniform_random (0, height));
    }

    cairo_set_line_width (cr, 1.);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
many_strokes_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "many-strokes", NULL);
}

void
many_strokes (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "many-strokes-halign", do_many_strokes_ha, NULL);
    cairo_perf_run (perf, "many-strokes-valign", do_many_strokes_va, NULL);
    cairo_perf_run (perf, "many-strokes-horizontal", do_many_strokes_h, NULL);
    cairo_perf_run (perf, "many-strokes-vertical", do_many_strokes_v, NULL);
    cairo_perf_run (perf, "many-strokes-random", do_many_strokes, NULL);
}
