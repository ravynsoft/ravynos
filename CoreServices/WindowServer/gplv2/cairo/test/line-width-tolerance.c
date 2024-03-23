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
 * by Zan Dobersek <zandobersek@gmail.com> at
 * https://bugs.webkit.org/show_bug.cgi?id=54471
 */

#include "cairo-test.h"

#include <math.h>

#define RADIUS 50

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* fill with green so RGB and RGBA tests can share the ref image */
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_paint (cr);

    /* red to see eventual bugs immediately */
    cairo_set_source_rgb (cr, 1, 0, 0);

    /* stroke 3/4 of a circle where the last quarter would be this
     * reference image. Keep just a 1 pixel border. Use a huge line
     * width (twice the circle's radius to get it filled completely).
     */
    cairo_set_line_width (cr, 2 * RADIUS);
    cairo_arc (cr, 1, RADIUS - 1, RADIUS, 0, - M_PI / 2.0);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (line_width_tolerance,
	    "Test interaction of line width and tolerance when stroking arcs",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    RADIUS, RADIUS,
	    NULL, draw)
