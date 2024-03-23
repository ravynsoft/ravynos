/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * the author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE AUTHOR. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;
    double dashes[] = {6, 3};

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* partially visible rectangle... */
    cairo_rectangle (cr, -0.5, -0.5, 61, 61);

    /* rectangles with intersecting segments... */
    cairo_save (cr);
    cairo_translate (cr, 30, 30);
    for (i = 0; i < 4; i++) {
	cairo_rotate (cr, M_PI / 4);
	cairo_rectangle (cr, -37, -15, 74, 30);
    }
    cairo_restore (cr);

    /* completely invisible rectangle */
    cairo_rectangle (cr, -5, -5, 70, 70);

    cairo_set_dash (cr, dashes, ARRAY_LENGTH (dashes), 0.);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (long_dashed_lines,
	    "Exercises _cairo_box_intersects_line_segment()",
	    "dash, stroke, stress", /* keywords */
	    NULL, /* requirements */
	    60, 60,
	    NULL, draw)
