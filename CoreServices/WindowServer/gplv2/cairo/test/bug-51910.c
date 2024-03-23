/*
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

/* An error in xlib pattern transformation discovered by Albertas Vyšniauskas */

static cairo_pattern_t *
source(void)
{
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;
    cairo_t *cr;
    int i;

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 32, 32);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, .5, .5, .5);
    cairo_set_line_width (cr, 2);

    for (i = -1; i <= 8; i++) {
	cairo_move_to (cr, -34 + 8*i, 34);
	cairo_rel_line_to (cr, 36, -36);
	cairo_stroke (cr);
    }

    pattern = cairo_pattern_create_for_surface (cairo_get_target (cr));
    cairo_destroy (cr);

    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);

    cairo_matrix_init_translate(&matrix, 14.1, 0);
    cairo_pattern_set_matrix(pattern, &matrix);

    return pattern;
}


static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;
    int i;

    cairo_paint (cr);

    pattern = source ();
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);

    for (i = 0; i < 8; i++) {
	cairo_rectangle (cr, 3.5*i, 32*i, 256, 32);
	cairo_fill (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_51910,
	    "A bug in the xlib pattern transformation",
	    " paint", /* keywords */
	    NULL, /* requirements */
	    256, 256,
	    NULL, draw)
