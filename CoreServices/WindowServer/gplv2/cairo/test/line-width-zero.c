/*
 * Copyright © 2007 Red Hat, Inc.
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
 * Author: Carl Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

/* This is a test case for the following bug:
 *
 *	Crash in cairo_stroke_extents when line width is 0 and line cap is ROUND
 *	(_cairo_pen_find_active_cw_vertex_index)
 *	https://bugs.freedesktop.org/show_bug.cgi?id=10231
 */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double x1, y1, x2, y2;

    cairo_move_to (cr, 0.0, 0.0);
    cairo_line_to (cr, 100.0, 100.0);
    cairo_set_line_width (cr, 0.0);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
    cairo_in_stroke (cr, 50, 50);
    cairo_stroke_preserve (cr);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
    cairo_in_stroke (cr, 50, 50);
    cairo_stroke_preserve (cr);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
    cairo_in_stroke (cr, 50, 50);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (line_width_zero,
	    "Test all stroke operations and all cap,join styles with line width of zero",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    NULL, draw)
