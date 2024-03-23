/*
 * Copyright © 2007 Jeff Smith
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Jeff Smith not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Jeff Smith makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * JEFF SMITH DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL JEFF SMITH BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Jeff Smith <whydoubt@yahoo.com>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double dashes[2] = {20, 20};
    int a=0, b=0, c=0;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    for (a=0; a<4; a++)
    for (b=0; b<5; b++)
    for (c=0; c<5; c++) {
	cairo_move_to (cr, ((b*5)+c)*60+10, a*60+10);
	cairo_rel_line_to (cr, 0, b*10);
	cairo_rel_line_to (cr, c*10, 0);

	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_set_line_width (cr, 8);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_dash (cr, dashes, 2, a*10);
	cairo_stroke_preserve (cr);

	cairo_set_source_rgb (cr, 0, 0.5, 1);
	cairo_set_line_width (cr, 2);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_dash (cr, 0, 0, 0);
	cairo_stroke (cr);
    }
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (dash_state,
	    "Tries to explore the state space of the dashing code",
	    "dash, stroke", /* keywords */
	    NULL, /* requirements */
	    25*60, 4*60,
	    NULL, draw)
