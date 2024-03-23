/*
 * Copyright © 2006 Benjamin Otte
 * Copyright © 2009 Chris Wilson
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
 * Author: Benjamin Otte <otte@gnome.org>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

/* set this to 0.1 to make this test work */
#define FACTOR 1.e6

/* XXX poppler-cairo doesn't handle gradients very well... */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;
    cairo_matrix_t mat = {
	0, -4.5254285714285709 * FACTOR,
	-2.6398333333333333 * FACTOR, 0,
	0, 0
    };

    pattern = cairo_pattern_create_radial (0, 0, 0,
					   0, 0, 16384 * FACTOR);
    cairo_pattern_add_color_stop_rgba (pattern,
				       0, 0.376471, 0.533333, 0.27451, 1);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 1, 1, 1, 1);
    cairo_pattern_set_matrix (pattern, &mat);

    cairo_scale (cr, 0.05, 0.05);
    cairo_translate (cr, 6000, 3500);

    cairo_set_source (cr, pattern);
    cairo_rectangle (cr, -6000, -3500, 12000, 7000);
    cairo_pattern_destroy (pattern);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (huge_radial,
	    "Test huge radial patterns",
	    "gradient, radial", /* keywords */
	    NULL, /* requirements */
	    600, 350,
	    NULL, draw)
