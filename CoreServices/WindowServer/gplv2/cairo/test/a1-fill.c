/*
 * Copyright © 2011 Intel Corporation
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

/* Exercise https://bugs.freedesktop.org/show_bug.cgi?id=31604 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *a1;
    cairo_t *cr2;

    a1 = cairo_image_surface_create (CAIRO_FORMAT_A1, 100, 100);
    cr2 = cairo_create (a1);
    cairo_surface_destroy (a1);

    cairo_set_operator (cr2, CAIRO_OPERATOR_SOURCE);
    cairo_rectangle (cr2, 10, 10, 80, 80);
    cairo_set_source_rgb (cr2, 1, 1, 1);
    cairo_fill (cr2);
    cairo_rectangle (cr2, 20, 20, 60, 60);
    cairo_set_source_rgb (cr2, 0, 0, 0);
    cairo_fill (cr2);

    a1 = cairo_surface_reference (cairo_get_target (cr2));
    cairo_destroy (cr2);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_mask_surface (cr, a1, 0, 0);
    cairo_surface_destroy (a1);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (a1_fill,
	    "Test filling of an a1-surface and use as mask",
	    "a1, alpha, fill, mask", /* keywords */
	    "target=raster", /* requirements */
	    100, 100,
	    NULL, draw)
