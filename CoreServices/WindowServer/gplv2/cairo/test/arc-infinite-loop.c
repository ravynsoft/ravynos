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
#include <float.h>

#define SIZE 8

/*
  cairo_arc can hang in an infinite loop if given huge (so big that
  adding/subtracting 4*M_PI to them doesn't change the value because
  of floating point rounding).

  The purpose of this test is to check that cairo doesn't hang or crash.
*/

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* Check if the code that guarantees start <= end hangs */
    cairo_arc (cr, 0, 0, 1, 1024 / DBL_EPSILON * M_PI, 0);

    /* Check if the code that handles huge angles hangs */
    cairo_arc (cr, 0, 0, 1, 0, 1024 / DBL_EPSILON * M_PI);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (arc_infinite_loop,
	    "Test cairo_arc with huge angles",
	    "arc", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
