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

/* Test the fidelity of the rasterisation, because Cairo is my favourite
 * driver test suite.
 */

#define SIZE 256
#define WIDTH 2
#define HEIGHT 10

static cairo_test_status_t
rectangles (cairo_t *cr, int width, int height)
{
    int i;

    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

    for (i = 1; i <= SIZE; i++) {
	int x, y;

	cairo_save (cr);
	cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
	cairo_clip (cr);

	cairo_scale (cr, 1./SIZE, 1./SIZE);
	for (x = -i; x < SIZE*WIDTH; x += 2*i) {
	    for (y = -i; y < SIZE*HEIGHT; y += 2*i) {
		/* Add a little tile composed of two non-overlapping squares
		 *   +--+
		 *   |  |
		 *   |__|__
		 *      |  |
		 *      |  |
		 *      +--+
		 */
		cairo_rectangle (cr, x, y, i, i);
		cairo_rectangle (cr, x+i, y+i, i, i);
	    }
	}
	cairo_fill (cr);
	cairo_restore (cr);

	cairo_translate (cr, WIDTH, 0);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
triangles (cairo_t *cr, int width, int height)
{
    int i;

    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

    for (i = 1; i <= SIZE; i++) {
	int x, y;

	cairo_save (cr);
	cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
	cairo_clip (cr);

	cairo_scale (cr, 1./SIZE, 1./SIZE);
	for (x = -i; x < SIZE*WIDTH; x += 2*i) {
	    for (y = -i; y < SIZE*HEIGHT; y += 2*i) {
		/* Add a tile composed of four non-overlapping
		 * triangles.  The plus and minus signs inside the
		 * triangles denote the orientation of the triangle's
		 * edges: + for clockwise and - for anticlockwise.
		 *
		 *   +-----+
		 *    \-|+/
		 *     \|/
		 *     /|\
		 *    /-|-\
		 *   +-----+
		 */

		/* top left triangle */
		cairo_move_to (cr, x, y);
		cairo_line_to (cr, x+i, y+i);
		cairo_line_to (cr, x+i, y);
		cairo_close_path (cr);

		/* top right triangle */
		cairo_move_to (cr, x+i, y);
		cairo_line_to (cr, x+2*i, y);
		cairo_line_to (cr, x+i, y+i);
		cairo_close_path (cr);

		/* bottom left triangle */
		cairo_move_to (cr, x+i, y+i);
		cairo_line_to (cr, x, y+2*i);
		cairo_line_to (cr, x+i, y+2*i);
		cairo_close_path (cr);

		/* bottom right triangle */
		cairo_move_to (cr, x+i, y+i);
		cairo_line_to (cr, x+i, y+2*i);
		cairo_line_to (cr, x+2*i, y+2*i);
		cairo_close_path (cr);
	    }
	}
	cairo_fill (cr);
	cairo_restore (cr);

	cairo_translate (cr, WIDTH, 0);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (half_coverage_rectangles,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster slow", /* requirements */
	    WIDTH * SIZE, HEIGHT,
	    NULL, rectangles)

CAIRO_TEST (half_coverage_triangles,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster slow", /* requirements */
	    WIDTH * SIZE, HEIGHT,
	    NULL, triangles)
