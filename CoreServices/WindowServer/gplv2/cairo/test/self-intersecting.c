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

/* Bug history
 *
 * 2005-06-01  Carl Worth  <cworth@cworth.org>
 *
 *   There's a long-standing bug in that self-intersecting paths give
 *   an incorrect result when stroked. The problem is that the
 *   trapezoids are generated incrementally along the stroke and as
 *   such, are not disjoint. The errant intersections of these
 *   trapezoids then leads to overfilled pixels.
 *
 *   The test belows first creates and fills a path. Then it creates a
 *   second path which has a stroked boundary identical to the first
 *   filled path. But the results of the two operations are
 *   different. The most obvious difference is in the central region
 *   where the entire path intersects itself. But notice that every
 *   time the path turns there are also errors on the inside of the
 *   turn, (since the subsequent trapezoids along the path intersect).
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_translate (cr, 1.0, 1.0);

    cairo_set_source_rgb (cr, 1, 0, 0); /* red */

    /* First draw the desired shape with a fill */
    cairo_rectangle (cr, 0.5, 0.5,  4.0, 4.0);
    cairo_rectangle (cr, 3.5, 3.5,  4.0, 4.0);
    cairo_rectangle (cr, 3.5, 1.5, -2.0, 2.0);
    cairo_rectangle (cr, 6.5, 4.5, -2.0, 2.0);

    cairo_fill (cr);

    /* Then try the same thing with a stroke */
    cairo_translate (cr, 0, 10);
    cairo_move_to (cr, 1.0, 1.0);
    cairo_rel_line_to (cr,  3.0,  0.0);
    cairo_rel_line_to (cr,  0.0,  6.0);
    cairo_rel_line_to (cr,  3.0,  0.0);
    cairo_rel_line_to (cr,  0.0, -3.0);
    cairo_rel_line_to (cr, -6.0,  0.0);
    cairo_close_path (cr);

    cairo_set_line_width (cr, 1.0);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (self_intersecting,
	    "Test strokes of self-intersecting paths"
	    "\nSelf-intersecting strokes are wrong due to incremental trapezoidization.",
	    "stroke, trap", /* keywords */
	    NULL, /* requirements */
	    10, 20,
	    NULL, draw)
