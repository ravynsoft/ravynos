/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2010 Andrea Canciani
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
 * Author: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

#define SIZE 52
#define OFFSET 5
#define DISTANCE 10.25

/*
  This test checks that boxes not aligned to pixels are drawn
  correctly.

  In particular the corners of the boxes are drawn incorrectly by
  cairo-image in cairo 1.10.0, because overlapping boxes are passed to
  a span converter which assumes disjoint boxes as input.

  This results in corners to be drawn with the wrong shade.
*/

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_line_width (cr, 4);
    cairo_translate (cr, 2*OFFSET, 2*OFFSET);

    for (i = 0; i < 4; i++) {
	double x = i * DISTANCE;

	cairo_move_to (cr, x, -OFFSET-0.75);
	cairo_line_to (cr, x, SIZE-3*OFFSET-0.25);

	cairo_move_to (cr, -OFFSET-0.75, x);
	cairo_line_to (cr, SIZE-3*OFFSET-0.25, x);
    }

    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
aligned (cairo_t *cr, int width, int height)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
    return draw (cr, width, height);
}

CAIRO_TEST (rectilinear_grid,
	    "Test rectilinear rasterizer (covering partial pixels)",
	    "rectilinear", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)

CAIRO_TEST (a1_rectilinear_grid,
	    "Test rectilinear rasterizer (covering whole pixels)",
	    "rectilinear", /* keywords */
	    "target=raster", /* requirements */
	    SIZE, SIZE,
	    NULL, aligned)
