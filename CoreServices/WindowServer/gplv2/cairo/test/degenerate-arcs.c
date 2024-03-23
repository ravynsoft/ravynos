/*
 * Copyright 2010 Intel Corporation
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

#define WIDTH 20
#define HEIGHT 20

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0.3, 0.4, 0.5);

    /* This should be equivalent to a simple rectangle, such as may be
     * constructed for a rounded-rectangle with corner radii of 0...
     */
    cairo_arc (cr, 5, 5, 0, M_PI, 3*M_PI/2);
    cairo_arc (cr, 15, 5, 0, 3*M_PI/2, 2*M_PI);
    cairo_arc (cr, 15, 15, 0, 0, M_PI/2);
    cairo_arc (cr, 5, 15, 0, M_PI/2, M_PI);

    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (degenerate_arcs,
	    "Tests path construction using a series of degenerate (radius=0) arcs",
	    "arc, fill", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
