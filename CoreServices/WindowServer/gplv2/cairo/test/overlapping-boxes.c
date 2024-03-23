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

/* Not strictly overlapping, but it does highlight the error in
 * an optimisation of fill-box handling that I frequently am
 * tempted to write.
 */

#include "cairo-test.h"

#define WIDTH		(20)
#define HEIGHT		(20)

static void
border (cairo_t *cr)
{
    cairo_rectangle (cr, 1, 1, 8, 8);
    cairo_rectangle (cr, 1.25, 1.25, 7.5, 7.5);
    cairo_rectangle (cr, 1.75, 1.75, 6.5, 6.5);
    cairo_rectangle (cr, 2, 2, 6, 6);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    border (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_fill (cr);

    cairo_translate (cr, 10, 0);

    border (cr);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_fill (cr);

    cairo_translate (cr, 0, 10);

    cairo_rectangle (cr, 0, 0, 10, 10);
    cairo_clip (cr);

    border (cr);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_IN);
    cairo_fill (cr);

    cairo_reset_clip (cr);

    cairo_translate (cr, -10, 0);

    cairo_rectangle (cr, 0, 0, 10, 10);
    cairo_clip (cr);

    border (cr);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (overlapping_boxes,
	    "A sub-pixel double border to highlight the danger in an easy optimisation",
	    "fill", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
