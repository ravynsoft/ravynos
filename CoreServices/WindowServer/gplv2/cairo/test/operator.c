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

#define N_OPERATORS (CAIRO_OPERATOR_SATURATE + 1)
#define SIZE 10
#define PAD 3

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    unsigned int n;

    cairo_translate (cr, PAD, PAD);

    for (n = 0; n < N_OPERATORS; n++) {
	cairo_reset_clip (cr);
	cairo_rectangle (cr, 0, 0, SIZE, SIZE);
	cairo_clip (cr);

	cairo_set_source_rgb (cr, 1, 0, 0);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	cairo_rectangle (cr, 0, 0, SIZE-PAD, SIZE-PAD);
	cairo_fill (cr);

	cairo_set_source_rgb (cr, 0, 0, 1);
	cairo_set_operator (cr, n);
	cairo_rectangle (cr, PAD, PAD, SIZE-PAD, SIZE-PAD);
	cairo_fill (cr);

	cairo_translate (cr, SIZE+PAD, 0);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (operator,
	    "Tests using set_operator()",
	    "operator", /* keywords */
	    NULL, /* requirements */
	    (SIZE+PAD) * N_OPERATORS + PAD, SIZE + 2*PAD,
	    NULL, draw)
