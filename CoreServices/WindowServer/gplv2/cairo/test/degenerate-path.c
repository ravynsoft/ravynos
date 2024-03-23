/*
 * Copyright © 2006 Jeff Muizelaar
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Jeff Muizelaar not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Jeff Muizelaar makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * JEFF MUIZELAAR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL JEFF MUIZELAAR BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Jeff Muizelaar <jeff@infidigm.net>
 */

#include "cairo-test.h"

#define PAD 3.0
#define LINE_WIDTH 6.0

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_line_cap_t cap[] = { CAIRO_LINE_CAP_ROUND, CAIRO_LINE_CAP_SQUARE, CAIRO_LINE_CAP_BUTT };
    size_t i;
    double dash[] = {2, 2};
    double dash_long[] = {6, 6};

    cairo_set_source_rgb (cr, 1, 0, 0);

    for (i = 0; i < ARRAY_LENGTH (cap); i++) {
	cairo_save (cr);

	cairo_set_line_cap (cr, cap[i]);

	/* simple degenerate paths */
	cairo_set_line_width (cr, LINE_WIDTH);
	cairo_move_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_line_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_stroke (cr);

	cairo_translate (cr, 0, 3*PAD);
	cairo_move_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_close_path (cr);
	cairo_stroke (cr);

	/* degenerate paths starting with dash on */
	cairo_set_dash (cr, dash, 2, 0.);

	cairo_translate (cr, 0, 3*PAD);
	cairo_move_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_line_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_stroke (cr);

	cairo_translate (cr, 0, 3*PAD);
	cairo_move_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_close_path (cr);
	cairo_stroke (cr);

	/* degenerate paths starting with dash off */
	/* these should not draw anything */
	cairo_set_dash (cr, dash, 2, 2.);

	cairo_translate (cr, 0, 3*PAD);
	cairo_move_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_line_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_stroke (cr);

	cairo_translate (cr, 0, 3*PAD);
	cairo_move_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_close_path (cr);
	cairo_stroke (cr);

	/* this should draw a single degenerate sub-path
	 * at the end of the path */
	cairo_set_dash (cr, dash_long, 2, 6.);

	cairo_translate (cr, 0, 3*PAD);
	cairo_move_to (cr, LINE_WIDTH + 6.0, LINE_WIDTH);
	cairo_line_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_stroke (cr);

	/* this should draw a single degenerate sub-path
	 * at the end of the path. The difference between this
	 * and the above is that this ends with a degenerate sub-path*/
	cairo_set_dash (cr, dash_long, 2, 6.);

	cairo_translate (cr, 0, 3*PAD);
	cairo_move_to (cr, LINE_WIDTH + 6.0, LINE_WIDTH);
	cairo_line_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_line_to (cr, LINE_WIDTH, LINE_WIDTH);
	cairo_stroke (cr);

	cairo_restore (cr);

	cairo_translate (cr, PAD+LINE_WIDTH+PAD, 0);
    }
    return CAIRO_TEST_SUCCESS;
}

/*
 * XFAIL: undefined behaviour in PS, needs path editing to convert degenerate
 * segments into circles/rectangles as expected by cairo
 */
CAIRO_TEST (degenerate_path,
	    "Tests the behaviour of degenerate paths with different cap types",
	    "degenerate", /* keywords */
	    NULL, /* requirements */
	    3*(PAD+LINE_WIDTH+PAD), 8*(LINE_WIDTH+PAD) + PAD,
	    NULL, draw)
