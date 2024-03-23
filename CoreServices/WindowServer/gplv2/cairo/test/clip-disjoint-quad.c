/*
 * Copyright © 2012 Red Hat, Inc.
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Soren Sandmann <sandmann@redhat.com>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define WIDTH 100
#define HEIGHT 200

static void
draw_quad (cairo_t *cr,
	   double x1, double y1,
	   double x2, double y2,
	   double x3, double y3,
	   double x4, double y4)
{
    cairo_move_to (cr, x1, y1);
    cairo_line_to (cr, x2, y2);
    cairo_line_to (cr, x3, y3);
    cairo_line_to (cr, x4, y4);
    cairo_close_path (cr);

    cairo_set_source_rgb (cr, 0, 0.6, 0);
    cairo_fill (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int position[5] = {0, HEIGHT/2-10, HEIGHT/2-5, HEIGHT/2, HEIGHT-10 };
    int i;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    for (i = 0; i < 5; i++) {
	cairo_reset_clip (cr);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_rectangle (cr, 0, 0, WIDTH/2, HEIGHT/2);
	cairo_rectangle (cr, WIDTH/2, position[i], WIDTH/2, 10);
	cairo_fill_preserve (cr);
	cairo_clip (cr);

	cairo_set_source_rgb (cr, 1, 0, 1);
	draw_quad (cr, 50, 50, 75, 75, 50, 150, 25, 75);
	cairo_fill (cr);

	cairo_translate(cr, WIDTH, 0);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_disjoint_quad,
	    "Tests a simple fill through two disjoint clips.",
	    "clip, fill", /* keywords */
	    NULL, /* requirements */
	    5*WIDTH, HEIGHT,
	    NULL, draw)
