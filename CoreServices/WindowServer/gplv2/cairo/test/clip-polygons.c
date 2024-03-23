/*
 * Copyright © 2011 Intel Corporation
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
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define STEP	5
#define WIDTH	100
#define HEIGHT	100

static void diamond (cairo_t *cr)
{
    cairo_move_to (cr, WIDTH/2, 0);
    cairo_line_to (cr, WIDTH, HEIGHT/2);
    cairo_line_to (cr, WIDTH/2, HEIGHT);
    cairo_line_to (cr, 0, HEIGHT/2);
    cairo_close_path (cr);
}

static void background (cairo_t *cr)
{
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb (cr, 0,0,0);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    background (cr);

    /* completely overlapping diamonds */
    cairo_save (cr);
    diamond (cr);
    cairo_clip (cr);
    diamond (cr);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, WIDTH, 0);

    /* partial overlap */
    cairo_save (cr);
    cairo_translate (cr, -WIDTH/4, 0);
    diamond (cr);
    cairo_clip (cr);
    cairo_translate (cr, WIDTH/2, 0);
    diamond (cr);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, WIDTH, 0);

    /* no overlap, but the bounding boxes must */
    cairo_save (cr);
    cairo_translate (cr, -WIDTH/2 + 2, -2);
    diamond (cr);
    cairo_clip (cr);
    cairo_translate (cr, WIDTH - 4, 4);
    diamond (cr);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, WIDTH, 0);

    /* completely disjoint */
    cairo_save (cr);
    cairo_translate (cr, -WIDTH/2 - 1, 0);
    diamond (cr);
    cairo_clip (cr);
    cairo_translate (cr, WIDTH + 2, 0);
    diamond (cr);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_polygons,
	    "Test drawing through through an intersection of polygons",
	    "clip", /* keywords */
	    "target=raster", /* requirements */
	    4*WIDTH, HEIGHT,
	    NULL, draw)
