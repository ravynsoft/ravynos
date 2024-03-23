/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2011 Intel Corporation
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

#include "cairo-test.h"

#define WIDTH 48
#define HEIGHT 52

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int sx, sy;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_translate(cr, 2, 2);

    for (sx = 1; sx <= 4; sx++) {
	cairo_save (cr);
	for (sy = 1; sy <= 4; sy++) {
	    cairo_rectangle (cr, 0, 0, sx, sy);
	    cairo_fill (cr);

	    cairo_rectangle (cr, sx + 1 + .5, 0, sx, sy);
	    cairo_fill (cr);

	    cairo_rectangle (cr, 0, sy + 1 + .5, sx, sy);
	    cairo_fill (cr);

	    cairo_rectangle (cr, sx + 1 + .5, sy + 1 + .5, sx-.5, sy-.5);
	    cairo_fill (cr);

	    cairo_translate (cr, 2*sx + 3, 0);
	}
	cairo_restore (cr);
	cairo_translate (cr, 0, 2*sy + 3);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (unaligned_box,
	    "Tests handling of various boundary conditions for unaligned rectangles.",
	    "fill", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
