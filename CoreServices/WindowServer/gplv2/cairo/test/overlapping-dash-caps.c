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

#include "cairo-test.h"

#define SIZE 100

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double dashes1[] = {20, 10};
    double dashes2[] = {10, 1};

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_line_width (cr, 15);

    cairo_set_dash (cr, dashes1, 2, 0);
    cairo_new_sub_path (cr);
    cairo_arc (cr, SIZE/2, SIZE/2, SIZE/2-10, 0, 2*M_PI);

    cairo_set_source_rgba (cr, 1, 0, 0, 0.5);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_stroke (cr);

    cairo_set_dash (cr, dashes2, 2, 0);
    cairo_new_sub_path (cr);
    cairo_arc (cr, SIZE/2, SIZE/2, SIZE/4-5, 0, 2*M_PI);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_source_rgba (cr, 0, 1, 0, 0.5);
    cairo_stroke (cr);


    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (overlapping_dash_caps,
	    "Test intersections between neighbouring dash segments",
	    "overlap, dash", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
