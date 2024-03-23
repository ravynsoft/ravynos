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

/* Test case for bug #4863:
 *
 *	stroking problems with wide dashed lines
 *	https://bugs.freedesktop.org/show_bug.cgi?id=4863
 */

#include "cairo-test.h"

#define WIDTH 71
#define HEIGHT 28

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double dash[2];

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */

    cairo_set_line_width (cr, 2);
    dash[0] = 8.0;
    dash[1] = 2.0;

    cairo_rectangle (cr, 2.0, 2.0, 67.0, 24.0);

    cairo_set_dash (cr, dash, 2, 9.0);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (leaky_dash,
	    "Exercises bug #4863 in which a dashed stroke leaks into half the rectangle being filled"
	    "\nknown bug (#4863) which has existed since the 1.0 release",
	    "dash, stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
