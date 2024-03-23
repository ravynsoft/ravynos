/*
 * Copyright © 2010 Mozilla Corporation
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
    cairo_surface_t *mask;
    cairo_t *cr2;

    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_paint (cr);

    /* clip twice, note that the intersection is smaller then the extents */
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_rectangle (cr, 10, 10, 80, 80);
    cairo_rectangle (cr, 20, 20, 60, 60);
    cairo_clip (cr);

    cairo_rectangle (cr, 0, 40, 40, 30);
    cairo_clip (cr);

    /* and exercise the bug found by Jeff Muizelaar */
    mask = cairo_surface_create_similar (cairo_get_target (cr),
					 CAIRO_CONTENT_ALPHA,
					 width-20, height-20);
    cr2 = cairo_create (mask);
    cairo_surface_destroy (mask);

    cairo_set_source_rgba (cr2, 1, 1, 1, 1);
    cairo_paint (cr2);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_mask_surface (cr, cairo_get_target (cr2), 0, 0);
    cairo_destroy (cr2);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_twice_rectangle,
	    "Tests clipping twice using rectangles",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    100, 100,
	    NULL, draw)
