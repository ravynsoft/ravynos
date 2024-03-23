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

#define SIZE 60 /* needs to be big to check large area effects (dithering) */
#define PAD 2

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const double alpha = 1./3;
    int n;

    /* flatten to white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* square */
    cairo_rectangle (cr, PAD, PAD, SIZE, SIZE);
    cairo_set_source_rgba (cr, 1, 0, 0, alpha);
    cairo_fill (cr);

    /* circle */
    cairo_translate (cr, SIZE + 2 * PAD, 0);
    cairo_arc (cr, PAD + SIZE / 2., PAD + SIZE / 2., SIZE / 2., 0, 2 * M_PI);
    cairo_set_source_rgba (cr, 0, 1, 0, alpha);
    cairo_fill (cr);

    /* triangle */
    cairo_translate (cr, 0, SIZE + 2 * PAD);
    cairo_move_to (cr, PAD + SIZE / 2, PAD);
    cairo_line_to (cr, PAD + SIZE, PAD + SIZE);
    cairo_line_to (cr, PAD, PAD + SIZE);
    cairo_set_source_rgba (cr, 0, 0, 1, alpha);
    cairo_fill (cr);

    /* star */
    cairo_translate (cr, -(SIZE + 2 * PAD) + SIZE/2., SIZE/2.);
    for (n = 0; n < 5; n++) {
	cairo_line_to (cr,
		       SIZE/2 * cos (2*n * 2*M_PI / 10),
		       SIZE/2 * sin (2*n * 2*M_PI / 10));

	cairo_line_to (cr,
		       SIZE/4 * cos ((2*n+1)*2*M_PI / 10),
		       SIZE/4 * sin ((2*n+1)*2*M_PI / 10));
    }
    cairo_set_source_rgba (cr, 0, 0, 0, alpha);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (fill_alpha,
	    "Tests using set_rgba();fill()",
	    "fill, alpha", /* keywords */
	    NULL, /* requirements */
	    2*SIZE + 4*PAD, 2*SIZE + 4*PAD,
	    NULL, draw)
