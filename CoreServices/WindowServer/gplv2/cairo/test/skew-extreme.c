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

/* This test case is designed to exercise the following bug:
 *
 *	Skew transforms were broken by the cairo update in December
 *	https://bugzilla.mozilla.org/show_bug.cgi?id=373632
 *
 * What's happening is that the rectangle is being skewed into the
 * following shape:
 *
 * a__b
 *  \ \
 *   \ \
 *    \ \
 *     \ \
 *      \ \
 *      d\_\c
 *
 * and the bug is that _cairo_traps_tessellate_convex_quad is
 * comparing b.x as less then d.x and therefore determining that the bc
 * edge is left of the ad edge. The fix is simply to compare c.x to
 * d.x instead of b.x to d.x .
 */

#define PAD		2
#define LINE_WIDTH	10
#define LINE_LENGTH	(2 * LINE_WIDTH)
#define SKEW_FACTOR	5.0
#define WIDTH		(PAD + (LINE_LENGTH * SKEW_FACTOR) + LINE_WIDTH + PAD)
#define HEIGHT		(PAD + LINE_WIDTH + (LINE_LENGTH * SKEW_FACTOR) + LINE_WIDTH + PAD)

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, PAD, PAD);

    cairo_set_line_width (cr, LINE_WIDTH);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);

    cairo_save (cr);
    {
	cairo_matrix_t skew_x = {
	    1.0, 0.0,
	    SKEW_FACTOR, 1.0,
	    0.0, 0.0
	};

	cairo_translate (cr, LINE_WIDTH / 2.0, 0.0);

	cairo_transform (cr, &skew_x);

	cairo_move_to (cr, 0.0, 0.0);
	cairo_line_to (cr, 0.0, LINE_LENGTH);
	cairo_stroke (cr);
    }
    cairo_restore (cr);

    cairo_translate (cr, 0.0, LINE_WIDTH);

    cairo_save (cr);
    {
	cairo_matrix_t skew_y = {
	    1.0, SKEW_FACTOR,
	    0.0, 1.0,
	    0.0, 0.0
	};

	cairo_translate (cr, 0.0, LINE_WIDTH / 2.0);

	cairo_transform (cr, &skew_y);

	cairo_move_to (cr, 0.0, 0.0);
	cairo_line_to (cr, LINE_LENGTH, 0.0);
	cairo_stroke (cr);
    }
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (skew_extreme,
	    "Test cases of extreme skew.",
	    "transform, stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
