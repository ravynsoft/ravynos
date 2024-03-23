/*
 * Copyright © 2007 Adrian Johnson
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

#define SIZE 40
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
    cairo_pattern_t *pattern;

    cairo_translate (cr, PAD, PAD);

    /* mask */
    cairo_push_group (cr);
    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 0.4);
    cairo_paint (cr);
    cairo_arc (cr, SIZE / 2, SIZE / 2, SIZE / 6, 0., 2. * M_PI);
    cairo_set_source_rgba (cr, 0.4, 0.4, 0.4, 0.8);
    cairo_fill (cr);
    pattern = cairo_pop_group (cr);

    /* source */
    cairo_push_group (cr);
    cairo_rectangle (cr, 0.3 * SIZE, 0.2 * SIZE, 0.5 * SIZE, 0.5 * SIZE);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_fill (cr);
    cairo_move_to     (cr,   0.0,          0.8 * SIZE);
    cairo_rel_line_to (cr,   0.7 * SIZE,   0.0);
    cairo_rel_line_to (cr, -0.375 * SIZE, -0.6 * SIZE);
    cairo_close_path (cr);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_fill (cr);
    cairo_pop_group_to_source (cr);

    cairo_mask (cr, pattern);
    cairo_pattern_destroy (pattern);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (mask_alpha,
	    "A simple test painting a group through a circle mask",
	    "mask, alpha", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
