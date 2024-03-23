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

#define LINE_WIDTH	12.
#define SIZE		(5 * LINE_WIDTH)
#define PAD		(2 * LINE_WIDTH)

static void
make_path (cairo_t *cr)
{
    const struct {
	double x, y;
    } scales[] = {
	{  1,  1 },
	{ -1,  1 },
	{  1, -1 },
	{ -1, -1 },
    };
    unsigned int i, j;

    for (j = 0; j < ARRAY_LENGTH (scales); j++) {
	cairo_save (cr);
	/* include reflections to flip the orientation of the join */
	cairo_scale (cr, scales[j].x, scales[j].y);
	for (i = 0; i < 3; i++) {
	    cairo_new_sub_path (cr);
	    cairo_move_to (cr, 0, -9*LINE_WIDTH/4 - 2);
	    cairo_line_to (cr,  0, -2*LINE_WIDTH - 2);
	    cairo_line_to (cr,  LINE_WIDTH/4, -2*LINE_WIDTH - 2);
	    cairo_rotate (cr, M_PI / 4.);
	}
	cairo_restore (cr);
    }
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_line_join_t join;

    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_set_line_width (cr, LINE_WIDTH);
    cairo_translate (cr, PAD + SIZE / 2., PAD + SIZE / 2.);

    for (join = CAIRO_LINE_JOIN_MITER; join <= CAIRO_LINE_JOIN_BEVEL; join++) {
	cairo_save (cr);

	cairo_set_line_join (cr, join);

	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	make_path (cr);
	cairo_stroke (cr);

	cairo_translate (cr, 0, SIZE + PAD);

	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	make_path (cr);
	cairo_stroke (cr);

	cairo_translate (cr, 0, SIZE + PAD);

	cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
	make_path (cr);
	cairo_stroke (cr);

	cairo_restore (cr);

	cairo_translate (cr, SIZE + PAD, 0);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (joins,
	    "Test joins",
	    "stroke, joins", /* keywords */
	    NULL, /* requirements */
	    3 * (PAD + SIZE) + PAD,
	    3 * (PAD + SIZE) + PAD,
	    NULL, draw)

