/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 *
 * Based on an original test case by M Joonas Pihlaja.
 */

#include "cairo-test.h"

#define PAD 5

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const double dashes[] = { 25, 25 };
    cairo_line_join_t joins[] = {
	CAIRO_LINE_JOIN_ROUND,
	CAIRO_LINE_JOIN_MITER,
	CAIRO_LINE_JOIN_BEVEL
    };
    cairo_line_cap_t caps[] = {
	CAIRO_LINE_CAP_ROUND,
	CAIRO_LINE_CAP_SQUARE,
	CAIRO_LINE_CAP_BUTT,
    };
    int i, j;

    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);

    cairo_set_dash (cr, dashes, 2, 0.);
    cairo_set_line_width (cr, 10);

    cairo_translate (cr, 5 + PAD, 5 + PAD);

    for (i = 0; i < ARRAY_LENGTH (joins); i++) {
	cairo_set_line_join (cr, joins[i]);
	cairo_save (cr);

	for (j = 0; j < ARRAY_LENGTH (caps); j++) {
	    cairo_set_line_cap (cr, caps[j]);

	    cairo_move_to (cr,  0, 0);
	    cairo_line_to (cr, 50, 0);
	    cairo_line_to (cr, 50,50);
	    cairo_stroke (cr);

	    cairo_translate (cr, 75, 0);
	}
	cairo_restore (cr);

	cairo_translate (cr, 0, 75);
    }

    return CAIRO_TEST_SUCCESS;
}

/*
 * XFAIL: needs path editing in PS to convert degenerate
 * end-caps into the shapes as expected by cairo (Or maybe PS is the correct
 * behaviour?)
 */
CAIRO_TEST (degenerate_dash,
	    "Tests the behaviour of dashed segments that end on a off-on transition",
	    "dash, degenerate", /* keywords */
	    NULL, /* requirements */
	    210 + 2*PAD, 210 + 2*PAD,
	    NULL, draw)
