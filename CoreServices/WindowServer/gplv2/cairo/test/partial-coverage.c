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

/* Test the sampling stratagems of the rasterisers by creating pixels
 * containing minute holes and seeing how close to the expected
 * coverage each rasteriser approaches.
 */

#define SIZE 64

#include "../src/cairo-fixed-type-private.h"
#define SAMPLE (1 << CAIRO_FIXED_FRAC_BITS)

static uint32_t state;

static uint32_t
hars_petruska_f54_1_random (void)
{
#define rol(x,k) ((x << k) | (x >> (32-k)))
    return state = (state ^ rol (state, 5) ^ rol (state, 24)) + 0x37798849;
#undef rol
}

static double
uniform_random (void)
{
    return hars_petruska_f54_1_random() / (double) UINT32_MAX;
}

/* coverage is given in [0,sample] */
static void
compute_occupancy (uint8_t *occupancy, int coverage, int sample)
{
    int i, c;

    if (coverage < sample/2) {
	memset (occupancy, 0, sample);
	if (coverage == 0)
	    return;

	for (i = c = 0; i < sample; i++) {
	    if ((sample - i) * uniform_random() < coverage - c) {
		occupancy[i] = 0xff;
		if (++c == coverage)
		    return;
	    }
	}
    } else {
	coverage = sample - coverage;
	memset (occupancy, 0xff, sample);
	if (coverage == 0)
	    return;

	for (i = c = 0; i < sample; i++) {
	    if ((sample - i) * uniform_random() < coverage - c) {
		occupancy[i] = 0;
		if (++c == coverage)
		    return;
	    }
	}
    }
}

static cairo_test_status_t
reference (cairo_t *cr, int width, int height)
{
    int i;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    for (i = 0; i < SIZE*SIZE; i++) {
	cairo_set_source_rgba (cr, 1., 1., 1.,
			       i / (double) (SIZE * SIZE));
	cairo_rectangle (cr, i % SIZE, i / SIZE, 1, 1);
	cairo_fill (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
three_quarter_reference (cairo_t *cr, int width, int height)
{
    int i;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    for (i = 0; i < SIZE*SIZE; i++) {
	cairo_set_source_rgba (cr, 1., 1., 1.,
			       .75 * i / (double) (SIZE * SIZE));
	cairo_rectangle (cr, i % SIZE, i / SIZE, 1, 1);
	cairo_fill (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
half_reference (cairo_t *cr, int width, int height)
{
    int i;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    for (i = 0; i < SIZE*SIZE; i++) {
	cairo_set_source_rgba (cr, 1., 1., 1.,
			       .5 * i / (double) (SIZE * SIZE));
	cairo_rectangle (cr, i % SIZE, i / SIZE, 1, 1);
	cairo_fill (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
rectangles (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE*SAMPLE * i / (SIZE * SIZE), SAMPLE*SAMPLE);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE*SAMPLE; j++) {
		if (occupancy[j]) {
		    cairo_rectangle (cr,
				     (j % SAMPLE + xs) / (double) SAMPLE,
				     (j / SAMPLE + ys) / (double) SAMPLE,
				     1 / (double) SAMPLE,
				     1 / (double) SAMPLE);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
intersecting_quads (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE*SAMPLE * i / (SIZE * SIZE), SAMPLE*SAMPLE);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE*SAMPLE; j++) {
		if (occupancy[j]) {
		    cairo_move_to (cr,
				     (j % SAMPLE + xs) / (double) SAMPLE,
				     (j / SAMPLE + ys) / (double) SAMPLE);
		    cairo_rel_line_to (cr, 1 / (double) SAMPLE, 1 / (double) SAMPLE);
		    cairo_rel_line_to (cr, 0, -1 / (double) SAMPLE);
		    cairo_rel_line_to (cr, -1 / (double) SAMPLE, 1 / (double) SAMPLE);
		    cairo_close_path (cr);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
half_triangles (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE*SAMPLE * i / (SIZE * SIZE), SAMPLE*SAMPLE);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE*SAMPLE; j++) {
		if (occupancy[j]) {
		    int x = j % SAMPLE + xs;
		    int y = j / SAMPLE + ys;
		    cairo_move_to (cr, x / (double) SAMPLE, y / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y+1) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, y / (double) SAMPLE);
		    cairo_close_path (cr);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
overlap_half_triangles (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE/2*SAMPLE/2 * i / (SIZE * SIZE), SAMPLE/2*SAMPLE/2);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE/2*SAMPLE/2; j++) {
		if (occupancy[j]) {
		    int x = 2 * (j % (SAMPLE/2)) + xs;
		    int y = 2 * (j / (SAMPLE/2)) + ys;

		    /* Add a 4-tile composed of two overlapping triangles.
		     *   .__.__.
		     *   |\   /|
		     *   | \ / |
		     *   .  x  |
		     *   | / \ |
		     *   |/   \|
		     *   .     .
		     *
		     * Coverage should be computable as 50% (due to counter-winding).
		     */

		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x) / (double) SAMPLE,   (y+2) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_close_path (cr);

		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y+2) / (double) SAMPLE);
		    cairo_close_path (cr);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
overlap_half_triangles_eo (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE/2*SAMPLE/2 * i / (SIZE * SIZE), SAMPLE/2*SAMPLE/2);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE/2*SAMPLE/2; j++) {
		if (occupancy[j]) {
		    int x = 2 * (j % (SAMPLE/2)) + xs;
		    int y = 2 * (j / (SAMPLE/2)) + ys;

		    /* Add a 4-tile composed of two overlapping triangles.
		     *   .__.__.
		     *   |\   /|
		     *   | \ / |
		     *   .  x  |
		     *   | / \ |
		     *   |/   \|
		     *   .     .
		     *
		     * Coverage should be computable as 50%, due to even-odd fill rule.
		     */

		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x) / (double) SAMPLE,   (y+2) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_close_path (cr);

		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y+2) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_close_path (cr);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
overlap_three_quarter_triangles (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE/2*SAMPLE/2 * i / (SIZE * SIZE), SAMPLE/2*SAMPLE/2);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE/2*SAMPLE/2; j++) {
		if (occupancy[j]) {
		    int x = 2 * (j % (SAMPLE/2)) + xs;
		    int y = 2 * (j / (SAMPLE/2)) + ys;

		    /* Add a 4-tile composed of two overlapping triangles.
		     *   .__.__.
		     *   |\   /|
		     *   | \ / |
		     *   .  x  |
		     *   | / \ |
		     *   |/   \|
		     *   .     .
		     *
		     * Coverage should be computable as 75%.
		     */

		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x) / (double) SAMPLE,   (y+2) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_close_path (cr);

		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y+2) / (double) SAMPLE);
		    cairo_line_to (cr, (x+2) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_close_path (cr);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
triangles (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE*SAMPLE * i / (SIZE * SIZE), SAMPLE*SAMPLE);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE*SAMPLE; j++) {
		if (occupancy[j]) {
		    /* Add a tile composed of two non-overlapping triangles.
		     *   .__.
		     *   | /|
		     *   |/ |
		     *   .--.
		     */
		    int x = j % SAMPLE + xs;
		    int y = j / SAMPLE + ys;

		    /* top-left triangle */
		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x) / (double) SAMPLE,   (y+1) / (double) SAMPLE);
		    cairo_close_path (cr);

		    /* bottom-right triangle */
		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y+1) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y+1) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_close_path (cr);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
intersecting_triangles (cairo_t *cr, int width, int height)
{
    uint8_t *occupancy;
    int i, j, channel;

    state = 0x12345678;
    occupancy = xmalloc (SAMPLE*SAMPLE);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (channel = 0; channel < 3; channel++) {
	switch (channel) {
	default:
	case 0: cairo_set_source_rgb (cr, 1.0, 0.0, 0.0); break;
	case 1: cairo_set_source_rgb (cr, 0.0, 1.0, 0.0); break;
	case 2: cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); break;
	}

	for (i = 0; i < SIZE*SIZE; i++) {
	    int xs, ys;

	    compute_occupancy (occupancy, SAMPLE*SAMPLE * i / (SIZE * SIZE), SAMPLE*SAMPLE);

	    xs = i % SIZE * SAMPLE;
	    ys = i / SIZE * SAMPLE;
	    for (j = 0; j < SAMPLE*SAMPLE; j++) {
		if (occupancy[j]) {
		    /* Add 2 overlapping tiles in a single cell, each composed
		     * of two non-overlapping triangles.
		     *   .--.   .--.
		     *   | /|   |\ |
		     *   |/ | + | \|
		     *   .--.   .--.
		     */
		    int x = j % SAMPLE + xs;
		    int y = j / SAMPLE + ys;

		    /* first pair of triangles, diagonal bottom-left to top-right */
		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x) / (double) SAMPLE,   (y+1) / (double) SAMPLE);
		    cairo_close_path (cr);
		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y+1) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y+1) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y) / (double) SAMPLE);
		    cairo_close_path (cr);

		    /* second pair of triangles, diagonal top-left to bottom-right */
		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y+1) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_close_path (cr);
		    cairo_move_to (cr, (x) / (double) SAMPLE,   (y) / (double) SAMPLE);
		    cairo_line_to (cr, (x+1) / (double) SAMPLE, (y+1) / (double) SAMPLE);
		    cairo_line_to (cr, (x) / (double) SAMPLE, (y+1) / (double) SAMPLE);
		    cairo_close_path (cr);
		}
	    }
	    cairo_fill (cr);
	}
    }

    free (occupancy);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (partial_coverage_rectangles,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, rectangles)

CAIRO_TEST (partial_coverage_intersecting_quads,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, intersecting_quads)

CAIRO_TEST (partial_coverage_intersecting_triangles,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, intersecting_triangles)
CAIRO_TEST (partial_coverage_triangles,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, triangles)
CAIRO_TEST (partial_coverage_overlap_three_quarter_triangles,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, overlap_three_quarter_triangles)
CAIRO_TEST (partial_coverage_overlap_half_triangles_eo,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, overlap_half_triangles_eo)
CAIRO_TEST (partial_coverage_overlap_half_triangles,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, overlap_half_triangles)
CAIRO_TEST (partial_coverage_half_triangles,
	    "Check the fidelity of the rasterisation.",
	    "coverage, raster", /* keywords */
	    "target=raster slow", /* requirements */
	    SIZE, SIZE,
	    NULL, half_triangles)

CAIRO_TEST (partial_coverage_reference,
	    "Check the fidelity of this test.",
	    "coverage, raster", /* keywords */
	    "target=raster", /* requirements */
	    SIZE, SIZE,
	    NULL, reference)
CAIRO_TEST (partial_coverage_three_quarter_reference,
	    "Check the fidelity of this test.",
	    "coverage, raster", /* keywords */
	    "target=raster", /* requirements */
	    SIZE, SIZE,
	    NULL, three_quarter_reference)
CAIRO_TEST (partial_coverage_half_reference,
	    "Check the fidelity of this test.",
	    "coverage, raster", /* keywords */
	    "target=raster", /* requirements */
	    SIZE, SIZE,
	    NULL, half_reference)
