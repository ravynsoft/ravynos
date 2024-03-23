/*
 * Copyright © 2007 Red Hat, Inc.
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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define SIZE   20
#define PAD    5
#define WIDTH  (PAD + 3 * (PAD + SIZE) + PAD)
#define HEIGHT (PAD + SIZE + PAD)

/* We're demonstrating here a bug originally reported by Benjamin Otte
 * on the cairo mailing list here, (after he ran into this problem
 * with various flash animations):
 *
 *	[cairo] Assertion `i < pen->num_vertices' failed in 1.4.10
 *	https://lists.cairographics.org/archives/cairo/2007-August/011282.html
 *
 * The problem shows up with an extreme transformation matrix that
 * collapses the pen to a single line, (which means that
 * _cairo_slope_compare cannot handle adjacent vertices in the pen
 * since they have parallel slope).
 *
 * This test case tests degenerate pens in several directions and uses
 * round caps to force the stroking code to attempt to walk around the
 * pen doing slope comparisons.
 */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    cairo_translate (cr, PAD, PAD);

    /* First compress the pen to a vertical line. */
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_curve_to (cr, SIZE / 2, 0, SIZE, SIZE / 2, SIZE, SIZE);
    cairo_save (cr);
    {
	cairo_scale (cr, 0.000001, 1.0);
	cairo_stroke (cr);
    }
    cairo_restore (cr);

    cairo_translate (cr, PAD + SIZE, 0);

    /* Then compress the pen to a horizontal line. */
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_curve_to (cr, SIZE / 2, 0, SIZE, SIZE / 2, SIZE, SIZE);
    cairo_save (cr);
    {
	cairo_scale (cr, 1.0, 0.000001);
	cairo_stroke (cr);
    }
    cairo_restore (cr);

    cairo_translate (cr, PAD + SIZE, 0);

    /* Finally a line at an angle. */
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_curve_to (cr, SIZE / 2, 0, SIZE, SIZE / 2, SIZE, SIZE);
    cairo_save (cr);
    {
	cairo_rotate (cr, M_PI / 4.0);
	cairo_scale (cr, 0.000001, 1.0);
	cairo_stroke (cr);
    }
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (degenerate_pen,
	    "Test round joins with a pen that's transformed to a line",
	    "degenerate", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
