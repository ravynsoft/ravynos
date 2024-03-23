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
#define PAD		(2 * LINE_WIDTH)

static void
make_path (cairo_t *cr)
{
    int i;

    cairo_save (cr);
    for (i = 0; i <= 3; i++) {
	cairo_new_sub_path (cr);
	cairo_move_to (cr, -SIZE / 2, 0.);
	cairo_line_to (cr,  SIZE / 2, 0.);
	cairo_rotate (cr, M_PI / 4.);
    }
    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr)
{
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, PAD + SIZE / 2., PAD + SIZE / 2.);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    make_path (cr);
    cairo_stroke (cr);

    cairo_translate (cr, 0, SIZE + PAD);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    make_path (cr);
    cairo_stroke (cr);

    cairo_translate (cr, 0, SIZE + PAD);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    make_path (cr);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_10 (cairo_t *cr, int width, int height)
{
    cairo_set_line_width (cr, LINE_WIDTH);
    return draw (cr);
}

static cairo_test_status_t
draw_2 (cairo_t *cr, int width, int height)
{
    cairo_set_line_width (cr, 2);
    return draw (cr);
}

static cairo_test_status_t
draw_1 (cairo_t *cr, int width, int height)
{
    cairo_set_line_width (cr, 1);
    return draw (cr);
}

static cairo_test_status_t
draw_05 (cairo_t *cr, int width, int height)
{
    cairo_set_line_width (cr, 0.5);
    return draw (cr);
}

CAIRO_TEST (caps,
	    "Test caps",
	    "stroke, caps", /* keywords */
	    NULL, /* requirements */
	    PAD + SIZE + PAD,
	    3 * (PAD + SIZE) + PAD,
	    NULL, draw_10)

CAIRO_TEST (caps_2,
	    "Test normal caps",
	    "stroke, caps", /* keywords */
	    NULL, /* requirements */
	    PAD + SIZE + PAD,
	    3 * (PAD + SIZE) + PAD,
	    NULL, draw_2)

CAIRO_TEST (caps_1,
	    "Test hairline caps",
	    "stroke, caps", /* keywords */
	    NULL, /* requirements */
	    PAD + SIZE + PAD,
	    3 * (PAD + SIZE) + PAD,
	    NULL, draw_1)

CAIRO_TEST (caps_05,
	    "Test fine caps",
	    "stroke, caps", /* keywords */
	    NULL, /* requirements */
	    PAD + SIZE + PAD,
	    3 * (PAD + SIZE) + PAD,
	    NULL, draw_05)

