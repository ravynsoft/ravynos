/*
 * Copyright © 2007 Mozilla Corporation
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
 * Author: Vladimir Vukicevic <vladimir@pobox.com>
 */

#include "cairo-test.h"
#include <math.h>
#include <stdio.h>

#define SIZE 100
#define SIZE2 20
#define OFFSET 10

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *image;
    cairo_pattern_t *pat;
    cairo_t *cr2;

    image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, SIZE2, SIZE2);
    cr2 = cairo_create (image);
    cairo_surface_destroy (image);

    cairo_set_source_rgba (cr2, 1, 0, 0, 1);
    cairo_rectangle (cr2, 0, 0, SIZE2/2, SIZE2/2);
    cairo_fill (cr2);
    cairo_set_source_rgba (cr2, 0, 1, 0, 1);
    cairo_rectangle (cr2, SIZE2/2, 0, SIZE2/2, SIZE2/2);
    cairo_fill (cr2);
    cairo_set_source_rgba (cr2, 0, 0, 1, 1);
    cairo_rectangle (cr2, 0, SIZE2/2, SIZE2/2, SIZE2/2);
    cairo_fill (cr2);
    cairo_set_source_rgba (cr2, 1, 1, 0, 1);
    cairo_rectangle (cr2, SIZE2/2, SIZE2/2, SIZE2/2, SIZE2/2);
    cairo_fill (cr2);

    pat = cairo_pattern_create_for_surface (cairo_get_target (cr2));
    cairo_destroy (cr2);

    cairo_pattern_set_extend (pat, CAIRO_EXTEND_REPEAT);

    cairo_set_source_rgba (cr, 0, 0, 0, 1);
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_fill (cr);

    cairo_translate (cr, OFFSET, OFFSET);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source (cr, pat);
    cairo_rectangle (cr, 0, 0, SIZE - OFFSET, SIZE - OFFSET);
    cairo_fill (cr);

    cairo_pattern_destroy (pat);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (composite_integer_translate_over_repeat,
	    "Test simple compositing: integer-translation 32->32 OVER, with repeat",
	    "composite", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
