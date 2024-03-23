/*
 * Copyright © 2006 M Joonas Pihlaja
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
 * Author: M Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
 */

/* Bug history
 *
 * 2006-12-05  M Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
 *
 *   There's currently a regression bug in the tessellation code from
 *   switching to the "new tessellator".  The bug is caused by
 *   confusion in the comparator used to order events when there are
 *   degenerate edges.
 */

#include "cairo-test.h"

/* Derived from zrusin's "another" polygon in the performance suite. */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 0, 0);

    /* The polygon uses (43,103) as its "base point".  Closed
     * subpaths are simulated by going from the base point to the
     * subpath's first point, doing the subpath, and returning to the
     * base point.  The moving to and from the base point causes
     * degenerate edges which shouldn't result in anything visible. */
    cairo_move_to (cr, 43, 103);

    /* First subpath. */
    cairo_line_to (cr, 91, 101);
    cairo_line_to (cr, 0, 112);
    cairo_line_to (cr, 60, 0);
    cairo_line_to (cr, 91, 101);

    cairo_line_to (cr, 43, 103);

    /* Second subpath. */
    cairo_line_to (cr, 176, 110);
    cairo_line_to (cr, 116, 100);
    cairo_line_to (cr, 176, 0);
    cairo_line_to (cr, 176, 110);

    cairo_close_path (cr);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (fill_degenerate_sort_order,
	    "Tests the tessellator's event comparator with degenerate input",
	    "degenerate, fill", /* keywords */
	    NULL, /* requirements */
	    190, 120,
	    NULL, draw)
