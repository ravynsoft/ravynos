/*
 * Copyright 2012 Intel Corporation
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

/* Test the fidelity of the rasterisation, because Cairo is my favourite
 * driver test suite.
 */

#define GENERATE_REFERENCE 0

#define WIDTH 256
#define HEIGHT 40

#include "../src/cairo-fixed-type-private.h"
#define PRECISION (1 << CAIRO_FIXED_FRAC_BITS)

static cairo_test_status_t
vertical (cairo_t *cr, int width, int height)
{
    int x;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    for (x = -HEIGHT*PRECISION-2; x <= (WIDTH+HEIGHT)*PRECISION+2; x += 4) {
	cairo_move_to (cr, x / (double)PRECISION - 2, -2);
	cairo_rel_line_to (cr, 0, HEIGHT + 4);
    }
    cairo_set_line_width (cr, 2 / (double)PRECISION);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
horizontal (cairo_t *cr, int width, int height)
{
    int x;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    for (x = -HEIGHT*PRECISION-2; x <= (WIDTH+HEIGHT)*PRECISION+2; x += 4) {
	cairo_move_to (cr, -2, x / (double)PRECISION - 2);
	cairo_rel_line_to (cr, HEIGHT + 4, 0);
    }
    cairo_set_line_width (cr, 2 / (double)PRECISION);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
diagonal (cairo_t *cr, int width, int height)
{
    int x;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    for (x = -HEIGHT*PRECISION-2; x <= (WIDTH+HEIGHT)*PRECISION+2; x += 6) {
	cairo_move_to (cr, x / (double)PRECISION - 2, -2);
	cairo_rel_line_to (cr, HEIGHT + 4, HEIGHT + 4);
    }
    cairo_set_line_width (cr, 2 / (double)PRECISION);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (sample_vertical,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster slow", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, vertical)

CAIRO_TEST (sample_horizontal,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster slow", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, horizontal)

CAIRO_TEST (sample_diagonal,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster slow", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, diagonal)
