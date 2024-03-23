/*
 * Copyright © 2011 Adrian Johnson
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

#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 80


static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;

    cairo_test_paint_checkered (cr);

    cairo_scale (cr, 0.3, 0.3);
    cairo_translate (cr, 50, 50);

    pattern = cairo_pattern_create_linear (70, 100, 130, 100);
    cairo_pattern_add_color_stop_rgba (pattern, 0,  1, 0, 0,  1.0);
    cairo_pattern_add_color_stop_rgba (pattern, 1,  0, 1, 0,  0.5);

    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
    cairo_set_source (cr, pattern);

    cairo_move_to(cr, 20, 20);
    cairo_curve_to(cr,
                   130, 0,
                   70, 200,
                   180, 180);
    cairo_set_line_width (cr, 20);
    cairo_stroke (cr);

    cairo_pattern_destroy (pattern);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (stroke_pattern,
	    "Patterned stroke",
	    "stroke, pattern", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
