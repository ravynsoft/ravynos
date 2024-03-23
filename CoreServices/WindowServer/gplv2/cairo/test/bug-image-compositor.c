/*
 * Copyright © 2020 Uli Schlachter, Heiko Lewin
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
 * Author: Uli Schlachter <psychon@znc.in>
 * Author: Heiko Lewin <hlewin@gmx.de>
 */
#include "cairo-test.h"


/* This test reproduces an overflow of a mask-buffer in cairo-image-compositor.c */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 0., 0., 0.);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 1., 1., 1.);
    cairo_set_line_width (cr, 1.);

    cairo_pattern_t *p = cairo_pattern_create_linear (0, 0, width, height);
    cairo_pattern_add_color_stop_rgb (p, 0, 0.99, 1, 1);
    cairo_pattern_add_color_stop_rgb (p, 1, 1, 1, 1);
    cairo_set_source (cr, p);
    cairo_pattern_destroy(p);

    cairo_move_to (cr, 0.5, -1);
    for (int i = 0; i < width; i+=3) {
	cairo_rel_line_to (cr, 2, 2);
	cairo_rel_line_to (cr, 1, -2);
    }

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}


CAIRO_TEST (bug_image_compositor,
	    "Crash in image-compositor",
	    "stroke, stress", /* keywords */
	    NULL, /* requirements */
	    10000, 1,
	    NULL, draw)
	    
