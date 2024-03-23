/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2007 Brian Ewins
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
 * Author: Brian Ewins <Brian.Ewins@gmail.com>
 * Contributor(s):
 *      Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

/* This test case is designed to exercise the opaque test for
 * gradients with no stop.
 */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pat;

    cairo_set_source_rgb (cr, 1., 0., 0.);

    pat = cairo_pattern_create_linear (0., 0., 1., 1.);
    cairo_mask (cr, pat);
    cairo_pattern_destroy (pat);

    pat = cairo_pattern_create_radial (0., 0., 0., 1., 1., 1.);
    cairo_mask (cr, pat);
    cairo_pattern_destroy (pat);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (gradient_zero_stops_mask,
	    "Verifies that gradients with no stops are considered clear.",
	    "gradient", /* keywords */
	    NULL, /* requirements */
	    2, 2,
	    NULL, draw)
