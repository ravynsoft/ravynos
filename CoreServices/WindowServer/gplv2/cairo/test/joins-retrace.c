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

#define LINE_WIDTH	10.
#define SIZE		(8 * LINE_WIDTH)
#define PAD		(1 * LINE_WIDTH)


static void
make_path (cairo_t *cr)
{
    cairo_move_to (cr, 0, SIZE/2 + LINE_WIDTH);
    cairo_rel_line_to (cr, SIZE, 0);
    cairo_rel_line_to (cr, -SIZE, 0);
    cairo_close_path (cr);

    cairo_move_to (cr, 3*SIZE/4, 0);
    cairo_rel_line_to (cr, -SIZE/2, SIZE);
    cairo_rel_line_to (cr, SIZE/2, -SIZE);
    cairo_close_path (cr);

    cairo_move_to (cr, 0, SIZE/2-LINE_WIDTH);
    cairo_rel_curve_to (cr,
			SIZE/2, -2*LINE_WIDTH,
			SIZE/2, 2*LINE_WIDTH,
			SIZE, 0);
    cairo_rel_curve_to (cr,
			-SIZE/2, 2*LINE_WIDTH,
			-SIZE/2, -2*LINE_WIDTH,
			-SIZE, 0);
    cairo_close_path (cr);
}

static void
draw_joins (cairo_t *cr)
{
    cairo_save (cr);
    cairo_translate (cr, PAD, PAD);

    make_path (cr);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke (cr);
    cairo_translate (cr, SIZE + PAD, 0.);

    make_path (cr);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_stroke (cr);
    cairo_translate (cr, SIZE + PAD, 0.);

    make_path (cr);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    cairo_stroke (cr);
    cairo_translate (cr, SIZE + PAD, 0.);

    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_set_line_width (cr, LINE_WIDTH);

    draw_joins (cr);

    /* and reflect to generate the opposite vertex ordering */
    cairo_translate (cr, 0, height);
    cairo_scale (cr, 1, -1);

    draw_joins (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (joins_retrace,
	    "A shape that repeats upon itself",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    3*(SIZE+PAD)+PAD, 2*(SIZE+PAD)+2*PAD,
	    NULL, draw)

