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


/* This is a variant on many strokes where we precompute
 * a simplified stroke-to-path.
 * When we have a real stroke-to-path, it would useful to compare the cost
 * of stroking vs filling the "identical" paths.
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
do_many_fills_ha (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double y = floor (uniform_random (0, height));
	double x = floor (uniform_random (0, width));
	cairo_rectangle (cr, x, y, ceil (uniform_random (0, width)) - x, 1);
    }

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_fills_h (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double y = uniform_random (0, height);
	double x = uniform_random (0, width);
	cairo_rectangle (cr, x, y, uniform_random (0, width) - x, 1);
    }

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_fills_va (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double x = floor (uniform_random (0, width));
	double y = floor (uniform_random (0, height));
	cairo_rectangle (cr, x, y, 1, ceil (uniform_random (0, height) - y));
    }

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_fills_v (cairo_t *cr, int width, int height, int loops)
{
    int count;

    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	double x = uniform_random (0, width);
	double y = uniform_random (0, height);
	cairo_rectangle (cr, x, y, 1, uniform_random (0, height) - y);
    }

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_many_fills (cairo_t *cr, int width, int height, int loops)
{
    int count;

    /* lots and lots of overlapping stroke-like fills */
    state = 0xc0ffee;
    for (count = 0; count < 1000; count++) {
	cairo_save (cr);
	cairo_translate (cr,
			 uniform_random (0, width),
			 uniform_random (0, height));
	cairo_rotate (cr, uniform_random (-M_PI,M_PI));
	cairo_rectangle (cr, 0, 0, uniform_random (0, width), 1);
	cairo_restore (cr);
    }

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
many_fills_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "many-fills", NULL);
}

void
many_fills (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "many-fills-halign", do_many_fills_ha, NULL);
    cairo_perf_run (perf, "many-fills-valign", do_many_fills_va, NULL);
    cairo_perf_run (perf, "many-fills-horizontal", do_many_fills_h, NULL);
    cairo_perf_run (perf, "many-fills-vertical", do_many_fills_v, NULL);
    cairo_perf_run (perf, "many-fills-random", do_many_fills, NULL);
}
