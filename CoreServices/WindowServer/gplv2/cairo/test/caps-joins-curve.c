/*
 * Copyright © 2008 Chris Wilson
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

#define LINE_WIDTH	10.
#define SIZE		(5 * LINE_WIDTH)
#define PAD		(3 * LINE_WIDTH)

static void
make_path (cairo_t *cr)
{
    cairo_move_to (cr, 0, 0);
    cairo_rel_curve_to (cr,
			-SIZE/4, SIZE/3,
			-SIZE/4, SIZE/3,
			0, SIZE);
    cairo_rel_curve_to (cr,
			SIZE/3, -SIZE/4,
			SIZE/3, -SIZE/4,
			SIZE, 0);
    cairo_close_path (cr);

    cairo_move_to (cr, 5 * LINE_WIDTH, 3 * LINE_WIDTH);
    cairo_rel_curve_to (cr,
			0, -3 * LINE_WIDTH,
			0, -3 * LINE_WIDTH,
			-3 * LINE_WIDTH, -3 * LINE_WIDTH);
}

static void
draw_caps_joins (cairo_t *cr)
{
    cairo_save (cr);

    cairo_translate (cr, PAD, PAD);

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
    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_set_line_width (cr, LINE_WIDTH);

    draw_caps_joins (cr);

    /* and reflect to generate the opposite vertex ordering */
    cairo_translate (cr, 0, height);
    cairo_scale (cr, 1, -1);

    draw_caps_joins (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (caps_joins_curve,
	    "Test caps and joins on curves",
	    "stroke, cap, join", /* keywords */
	    NULL, /* requirements */
	    3 * (PAD + SIZE) + PAD,
	    2 * (PAD + SIZE) + PAD,
	    NULL, draw)

