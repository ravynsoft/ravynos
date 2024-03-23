/*
 * Copyright © 2008 Red Hat, Inc.
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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define LINE_WIDTH  10
#define PAD	    2
#define WIDTH	    (PAD + LINE_WIDTH + PAD)
#define HEIGHT	    (WIDTH)

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, PAD, PAD);

    /* Paint background white, then draw in black. */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_width (cr, LINE_WIDTH);

    /* The default miter limit value of 10.0 guarantees that
     * right-angle turns, (in fact, any angle greater than 11
     * degrees), gets a miter rather than a bevel join. The
     * rectilinear stroke optimization was originally written in a
     * buggy way that did not respect the miter limit, (that is,
     * inappropriately drawing miter joins when the miter limit would
     * turn them into bevels). So we draw here with a miter limit of
     * 1.0 to force all miter joins into bevels. */
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    cairo_set_miter_limit (cr, 1.0);

    cairo_move_to (cr, LINE_WIDTH / 2.0, LINE_WIDTH);
    cairo_rel_line_to (cr, 0, - LINE_WIDTH / 2.0);
    cairo_rel_line_to (cr, LINE_WIDTH / 2.0, 0);

    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (rectilinear_miter_limit,
	    "Test that the rectilinear stroke optimization doesn't break cairo_set_miter_limit",
	    "miter, stroke, stress", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
