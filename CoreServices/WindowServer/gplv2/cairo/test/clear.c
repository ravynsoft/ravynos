/*
 * Copyright © 2009 Chris Wilson
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
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_text_extents_t extents;

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);

    cairo_translate (cr, 2, 2);
    cairo_save (cr);
    cairo_rectangle (cr, 0, 0, 20, 20);
    cairo_clip (cr);
    cairo_rectangle (cr, 5, 5, 10, 10);
    cairo_fill (cr);
    cairo_restore (cr);

    cairo_translate (cr, 20, 0);
    cairo_save (cr);
    cairo_rectangle (cr, 0, 0, 20, 20);
    cairo_clip (cr);
    cairo_arc (cr, 10, 10, 8, 0, 2*M_PI);
    cairo_fill (cr);
    cairo_restore (cr);

    cairo_translate (cr, 0, 20);
    cairo_save (cr);
    cairo_rectangle (cr, 0, 0, 20, 20);
    cairo_clip (cr);
    cairo_text_extents (cr, "Cairo", &extents);
    cairo_move_to (cr,
	           10 - (extents.width/2. + extents.x_bearing),
	           10 - (extents.height/2. + extents.y_bearing));
    cairo_text_path (cr, "Cairo");
    cairo_fill (cr);
    cairo_restore (cr);

    cairo_translate (cr, -20, 0);
    cairo_save (cr);
    cairo_rectangle (cr, 0, 0, 20, 20);
    cairo_clip (cr);
    cairo_move_to (cr, 10, 2);
    cairo_line_to (cr, 18, 18);
    cairo_line_to (cr, 2, 18);
    cairo_close_path (cr);
    cairo_fill (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clear,
	    "Test masked clears",
	    "paint, clear", /* keywords */
	    NULL, /* requirements */
	    44, 44,
	    NULL, draw)

