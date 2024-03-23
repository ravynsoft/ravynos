/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2010 Andrea Canciani
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
 * Author: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

#define SIZE 32

/*
  When cairo_arc is used to draw an arc of more than 2pi radians
  (i.e. a circle "looping over itself"), various different behaviors
  are possible:

  - draw exactly a circle (an arc of 2pi radians)

  - draw an arc such that the current point is the expected one and
    that does at least a complete circle (an arc of [2pi, 4pi)
    radians)

  - draw an arc with the original number of loops

  This test produces different results for each of these three cases.
*/

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double dashes[] = { 0.3, 7 };

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_save (cr);

    cairo_translate (cr, SIZE * .5, SIZE * .5);
    cairo_scale (cr, SIZE * 3 / 8., SIZE * 3 / 8.);

    cairo_arc (cr, 0, 0, 1, 0, 11 * M_PI);

    cairo_set_line_width (cr, 8. / SIZE);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_dash (cr, dashes, 2, 0);
    cairo_stroke (cr);

    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (arc_looping_dash,
	    "Test cairo_arc for angles describing more than a complete circle",
	    "arc", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
