/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2006 Red Hat, Inc.
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

#define LINE_WIDTH	10.
#define SIZE		(5 * LINE_WIDTH)
#define PAD		(2 * LINE_WIDTH)

static void
make_path (cairo_t *cr)
{
    cairo_move_to (cr, 0., 0.);
    cairo_rel_line_to (cr, 0., SIZE);
    cairo_rel_line_to (cr, SIZE, 0.);
    cairo_close_path (cr);

    cairo_move_to (cr, 2 * LINE_WIDTH, 0.);
    cairo_rel_line_to (cr, 3 * LINE_WIDTH, 0.);
    cairo_rel_line_to (cr, 0., 3 * LINE_WIDTH);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* First draw a checkered background */
    cairo_test_paint_checkered (cr);

    /* Then draw the original caps-joins test but with a bit of alphs thrown in. */
    cairo_set_line_width (cr, LINE_WIDTH);

    cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 0.5); /* 50% red */
    cairo_translate (cr, PAD, PAD);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke (cr);

    cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, 0.5); /* 50% green */
    cairo_translate (cr, SIZE + PAD, 0.);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_stroke (cr);

    cairo_set_source_rgba (cr, 0.0, 0.0, 1.0, 0.5); /* 50% blue */

    cairo_translate (cr, SIZE + PAD, 0.);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (caps_joins_alpha,
	    "Test caps and joins with some source alpha",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    3 * (PAD + SIZE) + PAD,
	    PAD + SIZE + PAD,
	    NULL, draw)
