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
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const double dashes_1[] = { 10, 0 };
    const double dashes_2[] = { 10, 0, 10, 10};
    const double dashes_3[] = { 10, 0, 10, 0};

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_set_line_width (cr, 6);

    cairo_set_dash (cr, NULL, 0, 0);
    cairo_rectangle (cr, 10, 10, 30, 30);
    cairo_stroke (cr);

    cairo_translate (cr, 50, 0);
    cairo_set_dash (cr, dashes_1, 2, 0);
    cairo_rectangle (cr, 10, 10, 30, 30);
    cairo_stroke (cr);

    cairo_translate (cr, 0, 50);
    cairo_set_dash (cr, dashes_2, 4, 0);
    cairo_rectangle (cr, 10, 10, 30, 30);
    cairo_stroke (cr);

    cairo_translate (cr, -50, 0);
    cairo_set_dash (cr, dashes_3, 4, 0);
    cairo_rectangle (cr, 10, 10, 30, 30);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (degenerate_solid_dash,
	    "Exercises degenerate dash ellison",
	    "stroke, dash", /* keywords */
	    NULL, /* requirements */
	    100, 100,
	    NULL, draw)
