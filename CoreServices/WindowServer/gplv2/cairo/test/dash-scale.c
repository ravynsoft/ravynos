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

#define LINE_WIDTH	8.
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

static void
draw_three_shapes (cairo_t *cr)
{
    cairo_save (cr);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke (cr);

    cairo_translate (cr, SIZE + PAD, 0.);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_stroke (cr);

    cairo_translate (cr, SIZE + PAD, 0.);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    cairo_stroke (cr);

    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double dash[] = {1.5 * LINE_WIDTH};

    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, PAD, PAD);

    cairo_set_dash (cr, dash, ARRAY_LENGTH (dash), - 2 * LINE_WIDTH);
    cairo_set_line_width (cr, LINE_WIDTH);
    draw_three_shapes (cr);

    cairo_translate (cr, 0, SIZE + 2 * PAD);

    cairo_save (cr);
    {
	cairo_set_dash (cr, dash, ARRAY_LENGTH (dash), - 2 * LINE_WIDTH);
	cairo_set_line_width (cr, LINE_WIDTH);
	cairo_scale (cr, 1, 2);
	draw_three_shapes (cr);
    }
    cairo_restore (cr);

    cairo_translate (cr, 0, 2 * (SIZE + PAD));

    cairo_save (cr);
    {
	cairo_scale (cr, 1, 2);
	cairo_set_dash (cr, dash, ARRAY_LENGTH (dash), - 2 * LINE_WIDTH);
	cairo_set_line_width (cr, LINE_WIDTH);
	draw_three_shapes (cr);
    }
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (dash_scale,
	    "Test interactions of cairo_set_dash and cairo_scale, (in particular with a non-uniformly scaled pen)",
	    "dash, stroke, transform", /* keywords */
	    NULL, /* requirements */
	    3 * (PAD + SIZE) + PAD,
	    PAD + 5 * SIZE + 2 * (2 * PAD) + PAD,
	    NULL, draw)
