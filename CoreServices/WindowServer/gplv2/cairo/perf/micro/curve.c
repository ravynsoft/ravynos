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
do_curve_stroke (cairo_t *cr, int width, int height, int loops)
{
    state = 0xc0ffee;
    cairo_set_line_width (cr, 2.);
    cairo_perf_timer_start ();

    while (loops--) {
	double x1 = uniform_random (0, width);
	double x2 = uniform_random (0, width);
	double x3 = uniform_random (0, width);
	double y1 = uniform_random (0, height);
	double y2 = uniform_random (0, height);
	double y3 = uniform_random (0, height);
	cairo_move_to (cr, uniform_random (0, width), uniform_random (0, height));
	cairo_curve_to (cr, x1, y1, x2, y2, x3, y3);
	cairo_stroke(cr);
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_curve_fill (cairo_t *cr, int width, int height, int loops)
{
    state = 0xc0ffee;
    cairo_perf_timer_start ();

    while (loops--) {
	double x0 = uniform_random (0, width);
	double x1 = uniform_random (0, width);
	double x2 = uniform_random (0, width);
	double x3 = uniform_random (0, width);
	double xm = uniform_random (0, width);
	double xn = uniform_random (0, width);
	double y0 = uniform_random (0, height);
	double y1 = uniform_random (0, height);
	double y2 = uniform_random (0, height);
	double y3 = uniform_random (0, height);
	double ym = uniform_random (0, height);
	double yn = uniform_random (0, height);

	cairo_move_to (cr, xm, ym);
	cairo_curve_to (cr, x1, y1, x2, y2, xn, yn);
	cairo_curve_to (cr, x3, y3, x0, y0, xm, ym);
	cairo_close_path (cr);

	cairo_fill(cr);
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
curve_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "curve", NULL);
}

void
curve (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1., 1., 1.);

    cairo_perf_run (perf, "curve-stroked", do_curve_stroke, NULL);
    cairo_perf_run (perf, "curve-filled", do_curve_fill, NULL);
}
