/*
 * Copyright © 2008 Red Hat, Inc.
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
 * Author: Soren Sandmann <sandmann@redhat.com>
 */

#include "cairo-test.h"

#define WIDTH 300
#define HEIGHT 300

typedef struct {
    double x, y;
} point_t;

static void
paint_curve (cairo_t *cr)
{
    const point_t points[] = {
	{ 100, 320 }, { 110, -80 },
	{ 180, 60 }, { 300, 170 },
	{ 300, -40 }
    };
    unsigned i;

    cairo_set_line_width (cr, 2);
    cairo_move_to (cr, points[0].x, points[0].y);

    for (i = 1; i < ARRAY_LENGTH (points) - 2; i += 3) {
	cairo_curve_to (cr,
			points[i].x, points[i].y,
			points[i + 1].x, points[i + 1].y,
			points[i + 2].x, points[i + 2].y);
    }
    cairo_set_line_width (cr, 5);
    cairo_stroke (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* Fill window with light blue */
    cairo_set_source_rgba (cr, 0.8, 0.8, 1.9, 1.0);
    cairo_paint (cr);

    /* Paint curve in green */
    cairo_set_source_rgba (cr, 0.6, 0.8, 0.6, 1.0);
    paint_curve (cr);

    /* Make clip region */
    cairo_rectangle (cr, 228, 131, 50, 13);
    cairo_rectangle (cr, 20, 99, 200, 75);
    cairo_clip_preserve (cr);

    /* Fill clip region with red */
    cairo_set_source_rgba (cr, 1.0, 0.5, 0.5, 0.8);
    cairo_fill (cr);

    /* Paint curve again, this time in blue */
    cairo_set_source_rgba (cr, 0, 0, 1.0, 1.0);
    paint_curve (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_disjoint,
	    "Tests stroking through two disjoint clips.",
	    "clip, stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
