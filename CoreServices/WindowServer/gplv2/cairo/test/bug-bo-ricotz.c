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

/* An assertion failure found by Rico Tzschichholz */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_rectangle (cr, 10, 55,165, 1);
    cairo_rectangle (cr, 174, 55,1, 413);
    cairo_rectangle (cr, 10, 56, 1, 413);
    cairo_rectangle (cr, 10, 469, 165, 1);
    cairo_clip (cr);

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

    cairo_move_to (cr, 10, 57);
    cairo_curve_to (cr, 10, 55.894531, 10.894531, 55, 12, 55);
    cairo_line_to (cr, 173, 55);
    cairo_curve_to (cr, 174.105469, 55, 175, 55.894531, 175, 57);
    cairo_line_to (cr, 175, 468);
    cairo_curve_to (cr, 175, 469.105469, 174.105469, 470, 173, 470);
    cairo_line_to (cr, 12, 470);
    cairo_curve_to (cr, 10.894531, 470, 10, 469.105469, 10, 468);

    cairo_move_to (cr, 11, 57);
    cairo_curve_to (cr, 11, 56.449219, 11.449219, 56, 12, 56);
    cairo_line_to (cr, 173, 56);
    cairo_curve_to (cr, 173.550781, 56, 174, 56.449219, 174, 57);
    cairo_line_to (cr, 174, 468);
    cairo_curve_to (cr, 174, 468.550781, 173.550781, 469, 173, 469);
    cairo_line_to (cr, 12, 469);
    cairo_curve_to (cr, 11.449219, 469, 11, 468.550781, 11, 468);

    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_bo_ricotz,
	    "Exercises a bug discovered by Rico Tzschichholz",
	    "clip, fill", /* keywords */
	    NULL, /* requirements */
	    649, 480,
	    NULL, draw)
