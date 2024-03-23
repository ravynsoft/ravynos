/*
 * Copyright (c) 2010 Intel Corporation
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

/* Adapted from a bug report by <cairouser@yahoo.com> */

#include "cairo-test.h"

static const struct xy {
   double x;
   double y;
} gp[] = {
    { 100, 250 },
    { 100, 100 },
    { 150, 230 },
    { 239, 100 },
    { 239, 250 },
};

static const double vp[3] = { 100, 144, 238.5 };

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;

    cairo_paint (cr); /* opaque background */

    for (i = 0; i < 5; ++i)
	cairo_line_to (cr, gp[i].x, gp[i].y);
    cairo_close_path (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_set_line_width (cr, 1.5);
    cairo_stroke_preserve (cr);
    cairo_clip (cr);

    for (i = 1; i < 3; ++i) {
	double x1 = vp[i - 1];
	double x2 = vp[i];

	cairo_move_to (cr, x1, 0);
	cairo_line_to (cr, x1, height);
	cairo_line_to (cr, x2, height);
	cairo_line_to (cr, x2, 0);
	cairo_close_path (cr);

	if (i & 1)
	    cairo_set_source_rgb (cr, 0, 1, 0);
	else
	    cairo_set_source_rgb (cr, 1, 1, 0);

	cairo_fill (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_shape,
	    "Test handling of clipping with a non-aligned shape",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    400, 300,
	    NULL, draw)
