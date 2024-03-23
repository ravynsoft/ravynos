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

/*
 * A variation on
 *
 * https://bugzilla.mozilla.org/show_bug.cgi?id=668921
 *
 * The issue is that we failed to tighten the initial approximated bounds
 * after tessellating the path.
 */

#include "cairo-test.h"

#define SIZE 60

static void
triangle (cairo_t *cr, double x, double y, double h)
{
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x+h/2, y+h);
    cairo_line_to (cr, x+h, y);
    cairo_close_path (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 0, 1);
    cairo_paint (cr);

    /* Set an unbounded operator so that we can see how accurate the bounded
     * extents were.
     */
    cairo_set_operator (cr, CAIRO_OPERATOR_IN);
    cairo_set_source_rgb (cr, 1, 1, 1);

    /* Wind several triangles together that reduce to nothing */
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    triangle (cr, 0, 0, SIZE);
    triangle (cr, 0, 0, SIZE);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (big_empty_triangle,
	    "Tests that we tighten the bounds after tessellation.",
	    "fill", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
