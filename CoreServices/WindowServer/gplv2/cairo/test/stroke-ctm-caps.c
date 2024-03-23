/*
 * Copyright © 2008 Adrian Johnson
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
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"

#define SIZE 100
#define PAD 2
#define WIDTH (PAD + SIZE + PAD)
#define HEIGHT WIDTH

/* This test is designed to test that PDF viewers use the correct
 * alpha values in an Alpha SMasks. Some viewers use the color values
 * instead of the alpha. The test draws a triangle and rectangle in a
 * group then draws the group using cairo_mask(). The mask consists of
 * a circle with the rgba (0.4, 0.4, 0.4, 0.8) and the background rgba
 * (0.8, 0.8, 0.8, 0.4).
 */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* flip the CTM, which most clearly shows the problem */
    cairo_translate (cr, 0, HEIGHT);
    cairo_scale (cr, 1, -1);

    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_set_line_width (cr, 10);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    cairo_move_to (cr, 20, 20);
    cairo_line_to (cr, 20, 70);
    cairo_stroke (cr);

    cairo_move_to (cr, 40, 20);
    cairo_line_to (cr, 70, 70);
    cairo_stroke (cr);

    cairo_move_to (cr, 60, 20);
    cairo_line_to (cr, 90, 20);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (stroke_ctm_caps,
	    "Test that the stroker correctly passes the device-space vector to the stroker for endcaps",
	    "stroke, transform", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
