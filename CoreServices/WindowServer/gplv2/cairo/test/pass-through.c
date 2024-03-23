/*
 * Copyright 2008 Chris Wilson
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

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int n;

    for (n = 0; n < 256; n++) {
	cairo_set_source_rgba (cr, 1, 1, 1, n / 255.);
	cairo_rectangle (cr, 0, n, 2, 1);
	cairo_fill (cr);
    }
    for (n = 0; n < 256; n++) {
	cairo_set_source_rgb (cr, n / 255., n / 255., n / 255.);
	cairo_rectangle (cr, 2, n, 2, 1);
	cairo_fill (cr);
    }

    cairo_translate (cr, 4, 0);

    for (n = 0; n < 256; n++) {
	cairo_set_source_rgba (cr, 1, 0, 0, n / 255.);
	cairo_rectangle (cr, 0, n, 2, 1);
	cairo_fill (cr);
    }
    for (n = 0; n < 256; n++) {
	cairo_set_source_rgb (cr, n / 255., 0, 0);
	cairo_rectangle (cr, 2, n, 2, 1);
	cairo_fill (cr);
    }

    cairo_translate (cr, 4, 0);

    for (n = 0; n < 256; n++) {
	cairo_set_source_rgba (cr, 0, 1, 0, n / 255.);
	cairo_rectangle (cr, 0, n, 2, 1);
	cairo_fill (cr);
    }
    for (n = 0; n < 256; n++) {
	cairo_set_source_rgb (cr, 0, n / 255., 0);
	cairo_rectangle (cr, 2, n, 2, 1);
	cairo_fill (cr);
    }

    cairo_translate (cr, 4, 0);

    for (n = 0; n < 256; n++) {
	cairo_set_source_rgba (cr, 0, 0, 1, n / 255.);
	cairo_rectangle (cr, 0, n, 2, 1);
	cairo_fill (cr);
    }
    for (n = 0; n < 256; n++) {
	cairo_set_source_rgb (cr, 0, 0, n / 255.);
	cairo_rectangle (cr, 2, n, 2, 1);
	cairo_fill (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (pass_through,
	    "tests pixel values",
	    "color", /* keywords */
	    NULL, /* requirements */
	    16, 256,
	    NULL, draw)
