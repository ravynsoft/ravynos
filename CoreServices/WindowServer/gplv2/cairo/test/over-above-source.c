/*
 * Copyright © 2007 Red Hat, Inc.
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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define SIZE 40
#define PAD 2
#define WIDTH (PAD + SIZE + PAD)
#define HEIGHT WIDTH

/* This test is designed to explore the interactions of "native" and
 * "fallback" objects. For the ps surface, OVER with non-1.0 opacity
 * will be a fallback while SOURCE will be native. For the pdf
 * surface, it's the reverse where OVER is native while SOURCE is a
 * fallback. */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, PAD, PAD);

    /* A red triangle with SOURCE */
    cairo_move_to     (cr,  SIZE / 2, SIZE / 2);
    cairo_rel_line_to (cr,  SIZE / 2, 0);
    cairo_rel_line_to (cr, -SIZE / 2, SIZE / 2);
    cairo_close_path (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba (cr, 1., 0., 0., 0.5); /* 50% red */

    cairo_fill (cr);

    /* A green circle with OVER */
    cairo_arc (cr, SIZE / 2, SIZE / 2, SIZE / 4, 0., 2. * M_PI);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba (cr, 0., 1., 0., 0.5); /* 50% green */

    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (over_above_source,
	    "A simple test drawing a circle with OVER after a triangle drawn with SOURCE",
	    "operator", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
