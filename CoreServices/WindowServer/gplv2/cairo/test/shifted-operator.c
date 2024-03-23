/*
 * Copyright © 2021 Anton Danilkin
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
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *recording_surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
    cairo_t *cr2 = cairo_create (recording_surface);
    cairo_translate (cr2, -30, -30);
    cairo_rectangle (cr2, 0, 0, 120, 90);
    cairo_set_source_rgba (cr2, 0.7, 0, 0, 0.8);
    cairo_fill (cr2);
    cairo_set_operator (cr2, CAIRO_OPERATOR_XOR);
    cairo_rectangle (cr2, 40, 30, 120, 90);
    cairo_set_source_rgba (cr2, 0, 0, 0.9, 0.4);
    cairo_fill (cr2);
    cairo_destroy (cr2);

    cairo_pattern_t *pattern = cairo_pattern_create_for_surface (recording_surface);
    cairo_surface_destroy(recording_surface);
    cairo_matrix_t matrix;
    cairo_matrix_init_translate (&matrix, -30, -30);
    cairo_pattern_set_matrix (pattern, &matrix);
    cairo_set_source (cr, pattern);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (shifted_operator,
	    "Test drawing a rectangle shifted into negative coordinates with an operator",
	    "operator, transform, record", /* keywords */
	    NULL, /* requirements */
	    160, 120,
	    NULL, draw)
