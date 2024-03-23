/*
 * Copyright © 2006 M Joonas Pihlaja
 * Copyright © 2011 Chris Wilson
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
 * Authors:
 *   M Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
 *   Chris Wilson <chris@chris-wilson.co.uk>
 */
#include "cairo-test.h"

#define SIZE 512
#define STEP (512+2)
#define NUM_SEGMENTS 128

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

static void nz_path (cairo_t *cr)
{
    int i;

    state = 0xc0ffee;

    cairo_move_to (cr, 0, 0);
    for (i = 0; i < NUM_SEGMENTS; i++) {
	double x = uniform_random (0, SIZE);
	double y = uniform_random (0, SIZE);
	cairo_line_to (cr, x, y);
    }
    cairo_close_path (cr);
}

static void region_path (cairo_t *cr)
{
    int i;

    state = 0xc0ffee;

    for (i = 0; i < NUM_SEGMENTS; i++) {
	int x = uniform_random (0, SIZE);
	int y = uniform_random (0, SIZE);
	int w = uniform_random (0, 40);
	int h = uniform_random (0, 40);
	cairo_rectangle (cr, x, y, w, h);
    }
}

static void rectangle_path (cairo_t *cr)
{
    int i;

    state = 0xc0ffee;

    for (i = 0; i < NUM_SEGMENTS; i++) {
	double x = uniform_random (0, SIZE);
	double y = uniform_random (0, SIZE);
	double w = uniform_random (0, 40);
	double h = uniform_random (0, 40);
	cairo_rectangle (cr, x, y, w, h);
    }
}

static void arc_path (cairo_t *cr)
{
    int i;

    state = 0xc0ffee;

    for (i = 0; i < NUM_SEGMENTS; i++) {
	double x = uniform_random (0, SIZE);
	double y = uniform_random (0, SIZE);
	double r = uniform_random (0, 20);
	cairo_new_sub_path (cr);
	cairo_arc (cr, x, y, r, 0, 2*M_PI);
    }
}


static void nz_fill_stroke (cairo_t *cr)
{
    nz_path (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_set_line_width (cr, 1.0);
    cairo_stroke (cr);
}

static void clip_to_quadrant (cairo_t *cr)
{
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_clip (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);

    state = 0xc0ffee;
    cairo_translate (cr, 1, 1);

    /* no clipping */
    cairo_save (cr); {
	clip_to_quadrant (cr);

	nz_fill_stroke (cr);
    } cairo_restore (cr);

    cairo_translate (cr, STEP, 0);

    /* random clipping */
    cairo_save (cr); {
	clip_to_quadrant (cr);

	nz_path (cr);
	cairo_clip (cr);

	nz_fill_stroke (cr);

	cairo_set_source_rgba (cr, 1, 1, 1, 0.5);
	cairo_paint (cr);
    } cairo_restore (cr);

    cairo_translate (cr, STEP, 0);

    /* regional clipping */
    cairo_save (cr); {
	clip_to_quadrant (cr);

	region_path (cr);
	cairo_clip (cr);

	nz_fill_stroke (cr);

	cairo_set_source_rgba (cr, 1, 1, 1, 0.5);
	cairo_paint (cr);
    } cairo_restore (cr);

    cairo_translate (cr, -2*STEP, STEP);

    /* rectangular clipping */
    cairo_save (cr); {
	clip_to_quadrant (cr);

	rectangle_path (cr);
	cairo_clip (cr);

	nz_fill_stroke (cr);

	cairo_set_source_rgba (cr, 1, 1, 1, 0.5);
	cairo_paint (cr);
    } cairo_restore (cr);

    cairo_translate (cr, STEP, 0);

    /* circular clipping */
    cairo_save (cr); {
	clip_to_quadrant (cr);

	arc_path (cr);
	cairo_clip (cr);

	nz_fill_stroke (cr);

	cairo_set_source_rgba (cr, 1, 1, 1, 0.5);
	cairo_paint (cr);
    } cairo_restore (cr);

    cairo_translate (cr, STEP, 0);

    /* all-of-the-above clipping */
    cairo_save (cr); {
	clip_to_quadrant (cr);

	nz_path (cr);
	cairo_clip (cr);
	region_path (cr);
	cairo_clip (cr);
	rectangle_path (cr);
	cairo_clip (cr);
	arc_path (cr);
	cairo_clip (cr);

	nz_fill_stroke (cr);

	cairo_set_source_rgba (cr, 1, 1, 1, 0.5);
	cairo_paint (cr);
    } cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (random_clip,
	    "Tests the clip generation and intersection computation",
	    "trap, clip", /* keywords */
	    NULL, /* requirements */
	    3*STEP+2, 2*STEP+2,
	    NULL, draw)
