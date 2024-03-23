/* cc `pkg-config --cflags --libs cairo` cairo-spline-image.c -o cairo-spline-image */

/* Copyright © 2005 Carl Worth
 * Copyright © 2012 Intel Corporation
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
 */

#include "cairo-test.h"

#define WIDE_LINE_WIDTH 160
#define NARROW_LINE_WIDTH 2

/* A spline showing bugs in the "contour-based stroking" in cairo 1.12 */
static const struct spline {
    struct { double x, y; } pt[5];
    double line_width;
    double rgba[4];
} splines[] = {
    {
	{
	    { 172.25, 156.185 },
	    { 177.225, 164.06 },
	    { 176.5, 157.5 },
	    { 175.5, 159.5 },
	},
	WIDE_LINE_WIDTH,
	{ 1, 1, 1, 1 },
    },
    {
	{
	    { 571.25, 247.185 },
	    { 78.225, 224.06 },
	    { 129.5, 312.5 },
	    { 210.5, 224.5 },
	},
	NARROW_LINE_WIDTH,
	{ 1, 0, 0, 1 },
    }
};

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    unsigned n;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);

    for (n = 0; n < ARRAY_LENGTH(splines); n++) {
	cairo_set_line_width (cr, splines[n].line_width);
	cairo_set_source_rgba (cr,
			       splines[n].rgba[0],
			       splines[n].rgba[1],
			       splines[n].rgba[2],
			       splines[n].rgba[3]);

	cairo_move_to (cr, splines[n].pt[0].x, splines[n].pt[0].y);
	cairo_curve_to (cr,
			splines[n].pt[1].x, splines[n].pt[1].y,
			splines[n].pt[2].x, splines[n].pt[2].y,
			splines[n].pt[3].x, splines[n].pt[3].y);

	cairo_stroke (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_spline,
	    "Exercises a bug in the stroking of splines",
	    "spline, stroke", /* keywords */
	    NULL, /* requirements */
	    300, 300,
	    NULL, draw)
