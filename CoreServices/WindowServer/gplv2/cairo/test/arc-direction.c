/*
 * Copyright © 2012 Intel Corporation
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

#define SIZE		(2 * 20)
#define PAD		(2)

static cairo_test_status_t
draw_arcs (cairo_t *cr)
{
    double start = M_PI/12, stop = 2*start;

    cairo_move_to (cr, SIZE/2, SIZE/2);
    cairo_arc (cr, SIZE/2, SIZE/2, SIZE/2, start, stop);
    cairo_fill (cr);

    cairo_translate (cr, SIZE+PAD, 0);
    cairo_move_to (cr, SIZE/2, SIZE/2);
    cairo_arc (cr, SIZE/2, SIZE/2, SIZE/2, 2*M_PI-stop, 2*M_PI-start);
    cairo_fill (cr);

    cairo_translate (cr, 0, SIZE+PAD);
    cairo_move_to (cr, SIZE/2, SIZE/2);
    cairo_arc_negative (cr, SIZE/2, SIZE/2, SIZE/2, 2*M_PI-stop, 2*M_PI-start);
    cairo_fill (cr);

    cairo_translate (cr, -SIZE-PAD, 0);
    cairo_move_to (cr, SIZE/2, SIZE/2);
    cairo_arc_negative (cr, SIZE/2, SIZE/2, SIZE/2, start, stop);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_translate (cr, PAD, PAD);
    draw_arcs(cr);
    cairo_restore (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_translate (cr, 2*SIZE+3*PAD, 0);
    cairo_save (cr);
    cairo_translate (cr, 2*SIZE+2*PAD, PAD);
    cairo_scale (cr, -1, 1);
    draw_arcs(cr);
    cairo_restore (cr);

    cairo_set_source_rgb (cr, 1, 0, 1);
    cairo_translate (cr, 0, 2*SIZE+3*PAD);
    cairo_save (cr);
    cairo_translate (cr, 2*SIZE+2*PAD, 2*SIZE+2*PAD);
    cairo_scale (cr, -1, -1);
    draw_arcs(cr);
    cairo_restore (cr);

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_translate (cr, -(2*SIZE+3*PAD), 0);
    cairo_save (cr);
    cairo_translate (cr, PAD, 2*SIZE+2*PAD);
    cairo_scale (cr, 1, -1);
    draw_arcs(cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (arc_direction,
	    "Test drawing positive/negative arcs",
	    "arc, fill", /* keywords */
	    NULL, /* requirements */
	    2*(3*PAD + 2*SIZE), 2*(3*PAD + 2*SIZE),
	    NULL, draw)

