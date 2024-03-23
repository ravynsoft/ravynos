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

#define GENERATE_REFERENCE 0

#define WIDTH 256
#define HEIGHT 40

#include "../src/cairo-fixed-type-private.h"
#define PRECISION (1 << CAIRO_FIXED_FRAC_BITS)

/* XXX beware multithreading! */
static uint32_t state;

static uint32_t
hars_petruska_f54_1_random (void)
{
#define rol(x,k) ((x << k) | (x >> (32-k)))
    return state = (state ^ rol (state, 5) ^ rol (state, 24)) + 0x37798849;
#undef rol
}

static double
random_offset (int range, int precise, int width)
{
    double x = hars_petruska_f54_1_random() / (double) UINT32_MAX * range / width;
    if (precise)
	x = floor (x * PRECISION) / PRECISION;
    return x;
}

static cairo_test_status_t
rectangles (cairo_t *cr, int width, int height)
{
    int x, y, channel;

    state = 0x12345678;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

#if GENERATE_REFERENCE
    for (x = 0; x < width; x++) {
	cairo_set_source_rgba (cr, 1, 1, 1, x * x * 1.0 / (width * width));
	cairo_rectangle (cr, x, 0, 1, height);
	cairo_fill (cr);
    }
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (x = 0; x < width; x++) {
	    for (y = 0; y < height; y++) {
		double dx = random_offset (width - x, TRUE, width);
		double dy = random_offset (width - x, TRUE, width);
		cairo_rectangle (cr, x + dx, y + dy, x / (double) width, x / (double) width);
	    }
	}
	cairo_fill (cr);
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
rhombus (cairo_t *cr, int width, int height)
{
    int x, y;
    int internal_size = width / 2;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

#if GENERATE_REFERENCE
    for (y = 0; y < internal_size; y++) {
	for (x = 0; x < internal_size; x++) {
	    cairo_set_source_rgba (cr, 1, 1, 1,
				   x * y / (2. * internal_size * internal_size));
	    cairo_rectangle (cr, 2*x, 2*y, 2, 2);
	    cairo_fill (cr);
	}
    }
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb (cr, 1, 1, 1);

    for (y = 0; y < internal_size; y++) {
	double yf = y / (double) internal_size;
	for (x = 0; x < internal_size; x++) {
	    double xf = x / (double) internal_size;

	    cairo_move_to (cr,
			   2*x + 1 - xf,
			   2*y + 1);
	    cairo_line_to (cr,
			   2*x + 1,
			   2*y + 1 - yf);
	    cairo_line_to (cr,
			   2*x + 1 + xf,
			   2*y + 1);
	    cairo_line_to (cr,
			   2*x + 1,
			   2*y + 1 + yf);
	    cairo_close_path (cr);
	}
    }

    cairo_fill (cr);
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
intersecting_quads (cairo_t *cr, int width, int height)
{
    int x, y, channel;

    state = 0x12345678;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

#if GENERATE_REFERENCE
    for (x = 0; x < width; x++) {
	cairo_set_source_rgba (cr, 1, 1, 1, x * x * 0.5 / (width * width));
	cairo_rectangle (cr, x, 0, 1, height);
	cairo_fill (cr);
    }
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (x = 0; x < width; x++) {
	    double step = x / (double) width;
	    for (y = 0; y < height; y++) {
		double dx = random_offset (width - x, TRUE, width);
		double dy = random_offset (width - x, TRUE, width);
		cairo_move_to (cr, x + dx, y + dy);
		cairo_rel_line_to (cr, step, step);
		cairo_rel_line_to (cr, 0, -step);
		cairo_rel_line_to (cr, -step, step);
		cairo_close_path (cr);
	    }
	}
	cairo_fill (cr);
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
intersecting_triangles (cairo_t *cr, int width, int height)
{
    int x, y, channel;

    state = 0x12345678;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

#if GENERATE_REFERENCE
    for (x = 0; x < width; x++) {
	cairo_set_source_rgba (cr, 1, 1, 1, x * x * 0.75 / (width * width));
	cairo_rectangle (cr, x, 0, 1, height);
	cairo_fill (cr);
    }
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (x = 0; x < width; x++) {
	    double step = x / (double) width;
	    for (y = 0; y < height; y++) {
		double dx = random_offset (width - x, TRUE, width);
		double dy = random_offset (width - x, TRUE, width);

		/* left */
		cairo_move_to (cr, x + dx, y + dy);
		cairo_rel_line_to (cr, 0, step);
		cairo_rel_line_to (cr, step, 0);
		cairo_close_path (cr);

		/* right, mirrored */
		cairo_move_to (cr, x + dx + step, y + dy + step);
		cairo_rel_line_to (cr, 0, -step);
		cairo_rel_line_to (cr, -step, step);
		cairo_close_path (cr);
	    }
	}
	cairo_fill (cr);
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
triangles (cairo_t *cr, int width, int height)
{
    int x, y, channel;

    state = 0x12345678;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

#if GENERATE_REFERENCE
    for (x = 0; x < width; x++) {
	cairo_set_source_rgba (cr, 1, 1, 1, x * x * 0.5 / (width * width));
	cairo_rectangle (cr, x, 0, 1, height);
	cairo_fill (cr);
    }
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (x = 0; x < width; x++) {
	    for (y = 0; y < height; y++) {
		double dx = random_offset (width - x, TRUE, width);
		double dy = random_offset (width - x, TRUE, width);
		cairo_move_to (cr, x + dx, y + dy);
		cairo_rel_line_to (cr, x / (double) width, 0);
		cairo_rel_line_to (cr, 0, x / (double) width);
		cairo_close_path (cr);
	    }
	}
	cairo_fill (cr);
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
abutting (cairo_t *cr, int width, int height)
{
    int x, y;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.75);

#if GENERATE_REFERENCE
    cairo_paint (cr);
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);

    for (y = 0; y < 16; y++) {
	for (x = 0; x < 16; x++) {
	    double theta = (y * 16 + x) * M_PI / 512;
	    double cx = 16 * cos (theta) + x * 16;
	    double cy = 16 * sin (theta) + y * 16;

	    cairo_move_to (cr, x * 16, y * 16);
	    cairo_line_to (cr, cx, cy);
	    cairo_line_to (cr, (x + 1) * 16, y * 16);
	    cairo_fill (cr);

	    cairo_move_to (cr, (x + 1) * 16, y * 16);
	    cairo_line_to (cr, cx, cy);
	    cairo_line_to (cr, (x + 1) * 16, (y + 1) * 16);
	    cairo_fill (cr);

	    cairo_move_to (cr, (x + 1) * 16, (y + 1) * 16);
	    cairo_line_to (cr, cx, cy);
	    cairo_line_to (cr, x * 16, (y + 1) * 16);
	    cairo_fill (cr);

	    cairo_move_to (cr, x * 16, (y + 1) * 16);
	    cairo_line_to (cr, cx, cy);
	    cairo_line_to (cr, x * 16, y * 16);
	    cairo_fill (cr);
	}
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
column_triangles (cairo_t *cr, int width, int height)
{
    int x, y, i, channel;

    state = 0x12345678;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

#if GENERATE_REFERENCE
    for (x = 0; x < width; x++) {
	cairo_set_source_rgba (cr, 1, 1, 1, x * 0.5 / width);
	cairo_rectangle (cr, x, 0, 1, height);
	cairo_fill (cr);
    }
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (x = 0; x < width; x++) {
	    double step = x / (double) (2 * width);
	    for (y = 0; y < height; y++) {
		for (i = 0; i < PRECISION; i++) {
		    double dy = random_offset (width - x, FALSE, width);

		    /*
		     * We want to test some sharing of edges to further
		     * stress the rasterisers, so instead of using one
		     * tall triangle, it is split into two, with vertical
		     * edges on either side that may co-align with their
		     * neighbours:
		     *
		     *  s ---  .      ---
		     *  t  |   |\      |
		     *  e  |   | \     |
		     *  p ---  ....    |  2 * step = x / width
		     *          \ |    |
		     *           \|    |
		     *            .   ---
		     *        |---|
		     *     1 / PRECISION
		     *
		     * Each column contains two triangles of width one quantum and
		     * total height of (x / width), thus the total area covered by all
		     * columns in each pixel is .5 * (x / width).
		     */

		    cairo_move_to (cr, x + i / (double) PRECISION, y + dy);
		    cairo_rel_line_to (cr, 0, step);
		    cairo_rel_line_to (cr, 1 / (double) PRECISION, step);
		    cairo_rel_line_to (cr, 0, -step);
		    cairo_close_path (cr);
		}
		cairo_fill (cr); /* do these per-pixel due to the extra volume of edges */
	    }
	}
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
row_triangles (cairo_t *cr, int width, int height)
{
    int x, y, i, channel;

    state = 0x12345678;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

#if GENERATE_REFERENCE
    for (x = 0; x < width; x++) {
	cairo_set_source_rgba (cr, 1, 1, 1, x * 0.5 / width);
	cairo_rectangle (cr, x, 0, 1, height);
	cairo_fill (cr);
    }
#else
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (x = 0; x < width; x++) {
	    double step = x / (double) (2 * width);
	    for (y = 0; y < height; y++) {
		for (i = 0; i < PRECISION; i++) {
		    double dx = random_offset (width - x, FALSE, width);

		    /* See column_triangles() for a transposed description
		     * of this geometry.
		     */

		    cairo_move_to (cr, x + dx, y + i / (double) PRECISION);
		    cairo_rel_line_to (cr,  step, 0);
		    cairo_rel_line_to (cr,  step, 1 / (double) PRECISION);
		    cairo_rel_line_to (cr, -step, 0);
		    cairo_close_path (cr);
		}
		cairo_fill (cr); /* do these per-pixel due to the extra volume of edges */
	    }
	}
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (coverage_rectangles,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, rectangles)

CAIRO_TEST (coverage_rhombus,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    2*WIDTH, 2*WIDTH,
	    NULL, rhombus)

CAIRO_TEST (coverage_intersecting_quads,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, intersecting_quads)

CAIRO_TEST (coverage_intersecting_triangles,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, intersecting_triangles)
CAIRO_TEST (coverage_row_triangles,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, row_triangles)
CAIRO_TEST (coverage_column_triangles,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    /* Smaller height since this test does not vary by y-coordinate */
	    WIDTH, 4,
	    NULL, column_triangles)
CAIRO_TEST (coverage_triangles,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, triangles)
CAIRO_TEST (coverage_abutting,
	    "Check the fidelity of the rasterisation.",
	    NULL, /* keywords */
	    "target=raster", /* requirements */
	    16*16, 16*16,
	    NULL, abutting)
