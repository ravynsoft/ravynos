/*
 * Copyright © 2007 Adrian Johnson
 * Copyright © 2009 Chris Wilson
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
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define PAT_WIDTH  120
#define PAT_HEIGHT 120
#define SIZE (PAT_WIDTH*2)
#define PAD 2
#define WIDTH (PAD + SIZE + PAD)
#define HEIGHT WIDTH

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_matrix_t m;

    /* make the output opaque */
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_translate (cr, PAD, PAD);

    cairo_matrix_init_scale (&m, 2, 1.5);
    cairo_matrix_rotate (&m, 1);
    cairo_matrix_translate (&m, -PAT_WIDTH/4.0, -PAT_WIDTH/2.0);
    cairo_matrix_invert (&m);
    cairo_set_matrix (cr, &m);

    cairo_rectangle (cr, 0, 0, PAT_WIDTH, PAT_HEIGHT);
    cairo_clip (cr);

    cairo_set_source_rgba (cr, 1, 0, 1, 0.5);
    cairo_rectangle (cr, PAT_WIDTH/6.0, PAT_HEIGHT/6.0, PAT_WIDTH/4.0, PAT_HEIGHT/4.0);
    cairo_fill (cr);

    cairo_set_source_rgba (cr, 0, 1, 1, 0.5);
    cairo_rectangle (cr, PAT_WIDTH/2.0, PAT_HEIGHT/2.0, PAT_WIDTH/4.0, PAT_HEIGHT/4.0);
    cairo_fill (cr);

    cairo_set_line_width (cr, 1);
    cairo_move_to (cr, PAT_WIDTH/6.0, 0);
    cairo_line_to (cr, 0, 0);
    cairo_line_to (cr, 0, PAT_HEIGHT/6.0);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_stroke (cr);

    cairo_move_to (cr, PAT_WIDTH/6.0, PAT_HEIGHT);
    cairo_line_to (cr, 0, PAT_HEIGHT);
    cairo_line_to (cr, 0, 5*PAT_HEIGHT/6.0);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_stroke (cr);

    cairo_move_to (cr, 5*PAT_WIDTH/6.0, 0);
    cairo_line_to (cr, PAT_WIDTH, 0);
    cairo_line_to (cr, PAT_WIDTH, PAT_HEIGHT/6.0);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_stroke (cr);

    cairo_move_to (cr, 5*PAT_WIDTH/6.0, PAT_HEIGHT);
    cairo_line_to (cr, PAT_WIDTH, PAT_HEIGHT);
    cairo_line_to (cr, PAT_WIDTH, 5*PAT_HEIGHT/6.0);
    cairo_set_source_rgb (cr, 1, 1, 0);
    cairo_stroke (cr);

    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_set_line_width (cr, PAT_WIDTH/10.0);

    cairo_move_to (cr, 0,         PAT_HEIGHT/4.0);
    cairo_line_to (cr, PAT_WIDTH, PAT_HEIGHT/4.0);
    cairo_stroke (cr);

    cairo_move_to (cr, PAT_WIDTH/4.0,         0);
    cairo_line_to (cr, PAT_WIDTH/4.0, PAT_WIDTH);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (rotated_clip,
	    "Test clipping with non identity pattern matrix",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
