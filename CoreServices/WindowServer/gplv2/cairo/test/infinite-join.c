/*
 * Copyright © 2006 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission. The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

/* Test case for bug #8379:
 *
 *	infinite loop when stroking
 *	https://bugs.freedesktop.org/show_bug.cgi?id=8379
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* Paint white, then draw in black. */
    cairo_set_source_rgb (cr, 1, 1, 1); /* white */
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0); /* black */

    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    /* scaling 2 times causes a slight rounding error in the ctm.
     * Without that, the bug doesn't happen. */
    cairo_scale (cr, 20 / 100., 20 / 100.);
    cairo_scale (cr, 1. / 20, 1. / 20);

    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width (cr, 20);

    cairo_translate (cr, -18300, -13200);

    cairo_new_path (cr);
    cairo_move_to (cr, 18928, 13843);
    cairo_line_to (cr, 18500, 13843);
    cairo_line_to (cr, 18500, 13400);
    cairo_line_to (cr, 18928, 13400);
    cairo_line_to (cr, 18928, 13843);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (infinite_join,
	    "Test case for infinite loop when stroking with round joins",
	    "stroke", /* keywords */
	    NULL,
	    8, 8,
	    NULL, draw)
