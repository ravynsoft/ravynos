/*
 * Copyright © 2010 Intel Corporation
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
 *
 * Based on a bug snippet by Jeremy Moles <jeremy@emperorlinux.com>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *mask;

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    cairo_push_group_with_content (cr, CAIRO_CONTENT_ALPHA);
    {
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_paint (cr);

	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, width, height);
	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_set_line_width (cr, 10);
	cairo_stroke (cr);
    }
    mask = cairo_pop_group (cr);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_mask (cr, mask);
    cairo_pattern_destroy (mask);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (a8_clear,
	    "Test clear on an a8 surface",
	    "a8, clear", /* keywords */
	    NULL, /* requirements */
	    40, 40,
	    NULL, draw)

