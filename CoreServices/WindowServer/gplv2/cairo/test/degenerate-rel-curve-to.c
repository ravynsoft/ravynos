/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2009 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define SIZE 30

/* Another attempt at avoiding unnecessary splines was made, where
 * a curve-to that ended on the same point as it began were discarded.
 * The same bug was made to rel-curve-to as well, hence this test.
 */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_set_line_width (cr, 1.0);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */

    /* entirely degenerate */
    cairo_move_to (cr, 2.5, 2.5);
    cairo_rel_curve_to (cr,
			0., 0.,
			0., 0.,
			0., 0.);
    cairo_stroke (cr);

    /* horizontal */
    cairo_move_to (cr, 5.5, 2.5);
    cairo_rel_curve_to (cr,
		         10., 0.,
			-10., 0.,
			 0., 0.);
    cairo_stroke (cr);

    /* vertical */
    cairo_move_to (cr, 5.5, 2.5);
    cairo_rel_curve_to (cr,
		        0.,  10.,
			0., -10.,
			0.,   0.);
    cairo_stroke (cr);

    cairo_translate (cr, 15, 0);

    /* horizontal/vertical */
    cairo_move_to (cr, 5.5, 0.5);
    cairo_rel_curve_to (cr,
		        -6.,  0.,
			 0., 10.,
			 0.,  0.);

    cairo_translate (cr, 10, 0);

    /* vertical/horizontal */
    cairo_move_to (cr, 5.5, 0.0);
    cairo_rel_curve_to (cr,
		        0., 11.,
			5.,  0.,
			0.,  0.);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (degenerate_rel_curve_to,
	    "Test optimization treating degenerate rel-curve-to as line-to",
	    "path", /* keywords */
	    NULL, /* requirements */
	    40,
	    5,
	    NULL, draw)
