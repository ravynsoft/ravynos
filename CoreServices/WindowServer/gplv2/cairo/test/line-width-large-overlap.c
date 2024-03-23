/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2011 Red Hat Inc.
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
 * Author: Benjamin Otte <otte@redhat.com>
 */

/*
 * Test case taken from the WebKit test suite, failure originally reported
 * by Zan Dobersek <zandobersek@gmail.com>. WebKit test is
 * LayoutTests/canvas/philip/tests/2d.path.rect.selfintersect.html
 */

#include "cairo-test.h"

#include <math.h>

#define LINE_WIDTH 120
#define SIZE 100
#define RECT_SIZE 10

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* fill with green so RGB and RGBA tests can share the ref image */
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_paint (cr);

    /* red to see eventual bugs immediately */
    cairo_set_source_rgb (cr, 1, 0, 0);

    /* big line width */
    cairo_set_line_width (cr, LINE_WIDTH);

    /* rectangle that is smaller than the line width in center of image */
    cairo_rectangle (cr,
                     (SIZE - RECT_SIZE) / 2,
                     (SIZE - RECT_SIZE) / 2,
                     RECT_SIZE,
                     RECT_SIZE);

    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

/* and again slightly offset to trigger another path */
static cairo_test_status_t
draw_offset (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, .5, .5);
    return draw (cr, width, height);
}

static cairo_test_status_t
draw_rotated (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, SIZE/2, SIZE/2);
    cairo_rotate (cr, M_PI/4);
    cairo_translate (cr, -SIZE/2, -SIZE/2);

    return draw (cr, width, height);
}

static cairo_test_status_t
draw_flipped (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, SIZE/2, SIZE/2);
    cairo_scale (cr, -1, 1);
    cairo_translate (cr, -SIZE/2, -SIZE/2);

    return draw (cr, width, height);
}

static cairo_test_status_t
draw_flopped (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, SIZE/2, SIZE/2);
    cairo_scale (cr, 1, -1);
    cairo_translate (cr, -SIZE/2, -SIZE/2);

    return draw (cr, width, height);
}

static cairo_test_status_t
draw_dashed (cairo_t *cr, int width, int height)
{
    const double dashes[] = { 4 };
    cairo_set_dash (cr, dashes, 1, 0);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    return draw (cr, width, height);
}

CAIRO_TEST (line_width_large_overlap,
	    "Test overlapping lines due to large line width",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
CAIRO_TEST (line_width_large_overlap_offset,
	    "Test overlapping lines due to large line width",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw_offset)
CAIRO_TEST (line_width_large_overlap_rotated,
	    "Test overlapping lines due to large line width",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw_rotated)
CAIRO_TEST (line_width_large_overlap_flipped,
	    "Test overlapping lines due to large line width",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw_flipped)
CAIRO_TEST (line_width_large_overlap_flopped,
	    "Test overlapping lines due to large line width",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw_flopped)
CAIRO_TEST (line_width_large_overlap_dashed,
	    "Test overlapping lines due to large line width",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw_dashed)
