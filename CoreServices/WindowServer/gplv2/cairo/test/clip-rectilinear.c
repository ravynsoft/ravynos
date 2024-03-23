/*
 * Copyright (c) 2011 Intel Corporation
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

#define SIZE 120

static void L(cairo_t *cr, int w, int h)
{
	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, 0, h);
	cairo_line_to (cr, w, h);
	cairo_line_to (cr, w, h/2);
	cairo_line_to (cr, w/2, h/2);
	cairo_line_to (cr, w/2, 0);
	cairo_close_path (cr);
}

static void LL(cairo_t *cr, int w, int h)
{
    cairo_save (cr);

    /* aligned */
    cairo_rectangle (cr, 0, 0, w, h);
    cairo_clip (cr);
    L (cr, w, h);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    /* unaligned */
    cairo_translate (cr, w+.25, .25);
    cairo_rectangle (cr, 0, 0, w, h);
    cairo_clip (cr);
    L (cr, w, h);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int w = SIZE/2, h = SIZE/2;

    cairo_paint (cr); /* opaque background */

    cairo_set_source_rgb (cr, 1, 0, 0);
    LL (cr, w, h);

    cairo_translate (cr, 0, h);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    cairo_set_source_rgb (cr, 0, 0, 1);
    LL (cr, w, h);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_rectilinear,
	    "Test handling of rectilinear clipping",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
