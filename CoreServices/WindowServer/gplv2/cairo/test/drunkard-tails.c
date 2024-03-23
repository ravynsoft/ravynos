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
#define SIZE		(5 * LINE_WIDTH)
#define PAD		(3 * LINE_WIDTH)

static uint32_t state;

static double
uniform_random (double minval, double maxval)
{
    static uint32_t const poly = 0x9a795537U;
    uint32_t n = 32;
    while (n-->0)
	state = 2*state < state ? (2*state ^ poly) : 2*state;
    return minval + state * (maxval - minval) / 4294967296.0;
}

static void
make_path (cairo_t *cr)
{
    int i;

    state = 0xdeadbeef;

    cairo_move_to (cr, SIZE/2, SIZE/2);
    for (i = 0; i < 200; i++) {
	double theta = uniform_random (-M_PI, M_PI);
	cairo_rel_line_to (cr,
			   cos (theta) * LINE_WIDTH / 4,
			   sin (theta) * LINE_WIDTH / 4);
    }
}

static void
draw_caps_joins (cairo_t *cr)
{
    cairo_save (cr);

    cairo_translate (cr, PAD, PAD);

    cairo_reset_clip (cr);
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_clip (cr);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_stroke (cr);

    cairo_translate (cr, SIZE + PAD, 0.);

    cairo_reset_clip (cr);
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_clip (cr);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_stroke (cr);

    cairo_translate (cr, SIZE + PAD, 0.);

    cairo_reset_clip (cr);
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_clip (cr);

    make_path (cr);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    cairo_stroke (cr);

    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_set_line_width (cr, LINE_WIDTH);

    draw_caps_joins (cr);

    cairo_save (cr);
    /* and reflect to generate the opposite vertex ordering */
    cairo_translate (cr, 0, height);
    cairo_scale (cr, 1, -1);

    draw_caps_joins (cr);
    cairo_restore (cr);

    cairo_translate (cr, 0, SIZE + PAD);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (drunkard_tails,
	    "Test caps and joins on short tail segments",
	    "stroke, cap, join", /* keywords */
	    NULL, /* requirements */
	    3 * (PAD + SIZE) + PAD,
	    2 * (PAD + SIZE) + PAD,
	    NULL, draw)

