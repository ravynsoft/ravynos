/*
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
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define SIZE 80

/* A very simple test to exercise the scan rasterisers with constant regions. */

static void
rounded_rectangle (cairo_t *cr, int x, int y, int w, int h, int r)
{
    cairo_new_sub_path (cr);
    cairo_arc (cr, x + r, y + r, r, M_PI, 3 * M_PI / 2);
    cairo_arc (cr, x + w - r, y + r, r, 3 *M_PI / 2, 2 * M_PI);
    cairo_arc (cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_arc (cr, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_close_path (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* Paint background white, then draw in black. */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    rounded_rectangle (cr, 5, 5, width-10, height-10, 15);
    rounded_rectangle (cr, 15, 15, width-30, height-30, 5);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (rounded_rectangle_fill,
	    "Tests handling of rounded rectangles, the UI designers favourite",
	    "fill, rounded-rectangle", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
