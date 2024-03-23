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
 *         Franz Schmid <Franz.Schmid@altmuehlnet.de>
 */

/* Test case for bug reported by Franz Schmid <Franz.Schmid@altmuehlnet.de>
 * https://lists.cairographics.org/archives/cairo/2008-April/013912.html
 *
 * See also: https://bugs.freedesktop.org/show_bug.cgi?id=17177
 */

#include "cairo-test.h"

#define WIDTH 60
#define HEIGHT 60

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const double dash[2] = {4, 2};

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0., 0., 0);

    cairo_translate (cr, 0.5, .5);
    cairo_set_line_width (cr, 1); /* This is vital to reproduce the bug. */

    /* First check simple rectangles */
    cairo_set_source_rgb (cr, 0., 0., 0);
    cairo_rectangle (cr, -WIDTH/4, -HEIGHT/4, WIDTH, HEIGHT);
    cairo_stroke (cr);
    cairo_rectangle (cr, WIDTH+WIDTH/4, -HEIGHT/4, -WIDTH, HEIGHT);
    cairo_stroke (cr);
    cairo_rectangle (cr, -WIDTH/4, HEIGHT+HEIGHT/4, WIDTH, -HEIGHT);
    cairo_stroke (cr);
    cairo_rectangle (cr, WIDTH+WIDTH/4, HEIGHT+HEIGHT/4, -WIDTH, -HEIGHT);
    cairo_stroke (cr);

    cairo_set_dash (cr, dash, 2, 0);

    /* And now dashed. */
    cairo_set_source_rgb (cr, 1., 0., 0);
    cairo_rectangle (cr, -WIDTH/4, -HEIGHT/4, WIDTH, HEIGHT);
    cairo_stroke (cr);
    cairo_set_source_rgb (cr, 0., 1., 0);
    cairo_rectangle (cr, WIDTH+WIDTH/4, -HEIGHT/4, -WIDTH, HEIGHT);
    cairo_stroke (cr);
    cairo_set_source_rgb (cr, 0., 0., 1);
    cairo_rectangle (cr, -WIDTH/4, HEIGHT+HEIGHT/4, WIDTH, -HEIGHT);
    cairo_stroke (cr);
    cairo_set_source_rgb (cr, 1., 1., 0);
    cairo_rectangle (cr, WIDTH+WIDTH/4, HEIGHT+HEIGHT/4, -WIDTH, -HEIGHT);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (leaky_dashed_rectangle,
	    "Exercises bug in which a dashed stroke leaks in from outside the surface",
	    "dash, stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
