/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2011 Intel Corporation
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

/*
 * This test case aims to reproduce the misbehaviour exhibited in
 * https://bugs.launchpad.net/ubuntu/+source/cairo/+bug/710072
 * i.e. out of bounds rendering with the rectangular span compositor.
 */

#include "cairo-test.h"

static cairo_test_status_t
draw_aligned (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_rectangle (cr, -10, -10, 20, 20);
    cairo_rectangle (cr, 5, 5, 20, 20);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, 1, 0, 0, .5);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_unaligned (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_rectangle (cr, -10.5, -10.5, 20, 20);
    cairo_rectangle (cr, 5.5, 5.5, 20, 20);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, 1, 0, 0, .5);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (image_bug_710072_aligned,
	    "Tests a bug where we may compute spans greater than bounded extents",
	    "extents, fill, stroke", /* keywords */
	    NULL, /* requirements */
	    15, 15,
	    NULL, draw_aligned)

CAIRO_TEST (image_bug_710072_unaligned,
	    "Tests a bug where we may compute spans greater than bounded extents",
	    "extents, fill, stroke", /* keywords */
	    NULL, /* requirements */
	    15, 15,
	    NULL, draw_unaligned)
