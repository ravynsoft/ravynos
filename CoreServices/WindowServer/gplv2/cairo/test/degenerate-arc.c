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

/* This test case exercises a "Potential division by zero in cairo_arc"
 * reported by Luiz Americo Pereira Camara <luizmed@oi.com.br>,
 * https://lists.cairographics.org/archives/cairo/2008-May/014054.html.
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int n;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

    cairo_set_line_width (cr, 5);
    cairo_set_source_rgb (cr, 0, 1, 0);
    for (n = 0; n < 8; n++) {
	double theta = n * 2 * M_PI / 8;
	cairo_new_sub_path (cr);
	cairo_arc (cr, 20, 20, 15, theta, theta);
	cairo_close_path (cr);
    }
    cairo_stroke (cr);

    cairo_set_line_width (cr, 2);
    cairo_set_source_rgb (cr, 0, 0, 1);
    for (n = 0; n < 8; n++) {
	double theta = n * 2 * M_PI / 8;
	cairo_move_to (cr, 20, 20);
	cairo_arc (cr, 20, 20, 15, theta, theta);
    }
    cairo_stroke (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_arc (cr, 20, 20, 2, 0, 2*M_PI);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (degenerate_arc,
	    "Tests the behaviour of degenerate arcs",
	    "degenerate", /* keywords */
	    NULL, /* requirements */
	    40, 40,
	    NULL, draw)
