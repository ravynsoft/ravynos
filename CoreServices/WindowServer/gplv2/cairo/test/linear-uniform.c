/*
 * Copyright © 2010 Intel Corporation
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

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    /* with alpha */
    pattern = cairo_pattern_create_linear (0, 0, 0, height);
    cairo_pattern_add_color_stop_rgba (pattern, 0, 1, 1, 1, .5);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 1, 1, 1, .5);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
    cairo_rectangle (cr, 0, 0, width/2, height);
    cairo_fill (cr);

    /* without alpha */
    pattern = cairo_pattern_create_linear (0, 0, 0, height);
    cairo_pattern_add_color_stop_rgb (pattern, 0, 1, 1, 1);
    cairo_pattern_add_color_stop_rgb (pattern, 1, 1, 1, 1);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
    cairo_rectangle (cr, width/2, 0, width/2, height);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (linear_uniform,
	    "Tests handling of \"solid\" linear gradients",
	    "gradient, linear", /* keywords */
	    NULL, /* requirements */
	    40, 40,
	    NULL, draw)
