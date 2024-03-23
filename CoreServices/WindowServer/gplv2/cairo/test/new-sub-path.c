/*
 * Copyright © 2005 Red Hat, Inc.
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
 */

#include "cairo-test.h"

#define SIZE 10

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); /* blue */

    /* Test cairo_new_sub_path followed by several different
     * path-modification functions in turn...
     */

    /* ... cairo_move_to */
    cairo_new_sub_path (cr);
    cairo_move_to (cr, SIZE, SIZE);
    cairo_line_to (cr, SIZE, 2 * SIZE);

    /* ... cairo_line_to */
    cairo_new_sub_path (cr);
    cairo_line_to (cr, 2 * SIZE, 1.5 * SIZE);
    cairo_line_to (cr, 3 * SIZE, 1.5 * SIZE);

    /* ... cairo_curve_to */
    cairo_new_sub_path (cr);
    cairo_curve_to (cr,
		    4.0 * SIZE, 1.5 * SIZE,
		    4.5 * SIZE, 1.0 * SIZE,
		    5.0 * SIZE, 1.5 * SIZE);

    /* ... cairo_arc */
    cairo_new_sub_path (cr);
    cairo_arc (cr,
	       6.5 * SIZE, 1.5 * SIZE,
	       0.5 * SIZE,
	       0.0, 2.0 * M_PI);

    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (new_sub_path,
	    "Test the cairo_new_sub_path call",
	    "path", /* keywords */
	    NULL, /* requirements */
	    8 * SIZE,
	    3 * SIZE,
	    NULL, draw)
