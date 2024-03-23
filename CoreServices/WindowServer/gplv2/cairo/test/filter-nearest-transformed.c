/*
 * Copyright © 2008 Chris Wilson
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
 * We wish to check the optimization away of non-fractional translations
 * for NEAREST surface patterns under a few transformations.
 */

static const char png_filename[] = "romedalen.png";

/* A single, black pixel */
static const uint32_t black_pixel = 0xff000000;

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    unsigned int i, j, k;
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    const cairo_matrix_t transform[] = {
	{  1, 0, 0,  1,  0, 0 },
	{ -1, 0, 0,  1,  8, 0 },
	{  1, 0, 0, -1,  0, 8 },
	{ -1, 0, 0, -1,  8, 8 },
    };
    const cairo_matrix_t ctx_transform[] = {
	{  1, 0, 0,  1,   0,  0 },
	{ -1, 0, 0,  1,  14,  0 },
	{  1, 0, 0, -1,   0, 14 },
	{ -1, 0, 0, -1,  14, 14 },
    };
    const double colour[][3] = {
	{0, 0, 0},
	{1, 0, 0},
	{0, 1, 0},
	{0, 0, 1},
    };
    cairo_matrix_t m;

    surface = cairo_image_surface_create_for_data ((uint8_t *) &black_pixel,
						   CAIRO_FORMAT_ARGB32,
						   1, 1, 4);
    pattern = cairo_pattern_create_for_surface (surface);
    cairo_surface_destroy (surface);

    cairo_pattern_set_filter (pattern, CAIRO_FILTER_NEAREST);

    surface = cairo_test_create_surface_from_png (ctx, png_filename);

    /* Fill background white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    for (k = 0; k < ARRAY_LENGTH (transform); k++) {
	/* draw a "large" section from an image */
	cairo_save (cr); {
	    cairo_set_matrix(cr, &ctx_transform[k]);
	    cairo_rectangle (cr, 0, 0, 7, 7);
	    cairo_clip (cr);

	    cairo_set_source_surface (cr, surface,
				      -cairo_image_surface_get_width (surface)/2.,
				      -cairo_image_surface_get_height (surface)/2.);
	    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
	    cairo_paint (cr);
	} cairo_restore (cr);

	cairo_set_source_rgb (cr, colour[k][0], colour[k][1], colour[k][2]);
	for (j = 4; j <= 6; j++) {
	    for (i = 4; i <= 6; i++) {
		cairo_matrix_init_translate (&m,
					     -(2*(i-4) + .1*i),
					     -(2*(j-4) + .1*j));
		cairo_matrix_multiply (&m, &m, &transform[k]);
		cairo_pattern_set_matrix (pattern, &m);
		cairo_mask (cr, pattern);
	    }
	}
    }

    cairo_pattern_destroy (pattern);
    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (filter_nearest_transformed,
	    "Test sample position when drawing transformed images with FILTER_NEAREST",
	    "filter, nearest", /* keywords */
	    NULL,
	    14, 14,
	    NULL, draw)
