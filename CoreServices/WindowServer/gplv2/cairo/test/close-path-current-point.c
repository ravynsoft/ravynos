/*
 * Copyright © 2009 Nis Martensen
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of the copyright holder
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission. The
 * copyright holder makes no representations about the suitability of
 * this software for any purpose. It is provided "as is" without
 * express or implied warranty.
 *
 * THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Nis Martensen <nis.martensen@web.de>
 */

#include "cairo-test.h"

#define SIZE 20

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    /* subpath starts with cairo_move_to */
    cairo_new_sub_path (cr);
    cairo_move_to (cr, SIZE, SIZE);
    cairo_rel_line_to (cr, SIZE, 0);
    cairo_rel_line_to (cr, 0, SIZE);
    cairo_close_path (cr);
    cairo_rel_line_to (cr, 0.5 * SIZE, SIZE);

    /* subpath starts with cairo_line_to */
    cairo_new_sub_path (cr);
    cairo_line_to (cr, SIZE, 3 * SIZE);
    cairo_rel_line_to (cr, SIZE, 0);
    cairo_rel_line_to (cr, 0, SIZE);
    cairo_close_path (cr);
    cairo_rel_line_to (cr, 0, SIZE);

    /* subpath starts with cairo_curve_to */
    cairo_new_sub_path (cr);
    cairo_curve_to (cr,
		    SIZE, 5 * SIZE,
		    1.5 * SIZE, 6 * SIZE,
		    2 * SIZE, 5 * SIZE);
    cairo_rel_line_to (cr, 0, SIZE);
    cairo_close_path (cr);
    cairo_rel_line_to (cr, -0.5 * SIZE, SIZE);

    /* subpath starts with cairo_arc */
    cairo_new_sub_path (cr);
    cairo_arc (cr,
	       1.5 * SIZE, 7 * SIZE,
	       0.5 * SIZE,
	       M_PI, 2 * M_PI);
    cairo_rel_line_to (cr, 0, SIZE);
    cairo_close_path (cr);
    cairo_rel_line_to (cr, -0.7 * SIZE, 0.7 * SIZE);

    /* subpath starts with cairo_arc_negative */
    cairo_new_sub_path (cr);
    cairo_arc_negative (cr,
			1.5 * SIZE, 9 * SIZE,
			0.5 * SIZE,
			M_PI, 2 * M_PI);
    cairo_rel_line_to (cr, 0, SIZE);
    cairo_close_path (cr);
    cairo_rel_line_to (cr, -0.8 * SIZE, 0.3 * SIZE);

    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (close_path_current_point,
	    "Test some corner cases related to cairo path operations and the current point",
	    "path", /* keywords */
	    NULL, /* requirements */
	    3 * SIZE, 11 * SIZE,
	    NULL, draw)
