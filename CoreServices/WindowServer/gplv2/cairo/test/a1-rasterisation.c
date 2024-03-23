/*
 * Copyright 2010 Intel Corporation
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

/*
 * Test the fidelity of the rasterisation, paying careful attention to rounding.
 */

#include "../src/cairo-fixed-type-private.h"
#define PRECISION (int)(1 << CAIRO_FIXED_FRAC_BITS)

#define WIDTH ((PRECISION/2+1)*3)
#define HEIGHT ((PRECISION/2+1)*3)

#define SUBPIXEL(v) ((v)/(double)(PRECISION/2))

static cairo_test_status_t
rectangles (cairo_t *cr, int width, int height)
{
    int x, y;

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    for (x = 0; x < WIDTH; x += 3) {
	for (y = 0; y < HEIGHT; y += 3) {
	    cairo_rectangle (cr, x + SUBPIXEL (y/3) - .5, y + SUBPIXEL (x/3) - .5, .5, .5);
	}
    }
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
triangles (cairo_t *cr, int width, int height)
{
    int x, y;

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    for (x = 0; x < WIDTH; x += 3) {
	for (y = 0; y < HEIGHT; y += 3) {
	    /* a rectangle with a diagonal to force tessellation */
	    cairo_move_to (cr, x + SUBPIXEL (y/3) - .5, y + SUBPIXEL (x/3) - .5);
	    cairo_rel_line_to (cr, .5, .5);
	    cairo_rel_line_to (cr, 0, -.5);
	    cairo_rel_line_to (cr, -.5, 0);
	    cairo_rel_line_to (cr, 0, .5);
	    cairo_rel_line_to (cr, .5, 0);
	}
    }
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (a1_rasterisation_rectangles,
	    "Check the fidelity of the rasterisation.",
	    "rasterisation", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, rectangles)

CAIRO_TEST (a1_rasterisation_triangles,
	    "Check the fidelity of the rasterisation.",
	    "rasterisation", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, triangles)
