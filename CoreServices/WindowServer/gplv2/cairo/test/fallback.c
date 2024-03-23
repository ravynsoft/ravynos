/*
 * Copyright © 2012 Adrian Johnson
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

#define SIZE 40
#define WIDTH (7*SIZE)
#define HEIGHT (5*SIZE)

#define FALLBACK_RES_X 300
#define FALLBACK_RES_Y 150

static void
rectangles (cairo_t *cr)
{
    cairo_save (cr);

    cairo_rotate (cr, M_PI/8);
    cairo_translate (cr, 2*SIZE, SIZE/16);
    cairo_scale (cr, 1.5, 1.5);

    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_set_source_rgba (cr, 1, 0, 0, 0.5);
    cairo_fill (cr);

    /* Select an operator not supported by PDF/PS/SVG to trigger fallback */
    cairo_set_operator (cr, CAIRO_OPERATOR_SATURATE);

    cairo_rectangle (cr, SIZE/2, SIZE/2, SIZE, SIZE);
    cairo_set_source_rgba (cr, 0, 1, 0, 0.5);
    cairo_fill (cr);

    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_set_fallback_resolution (cairo_get_target (cr), FALLBACK_RES_X, FALLBACK_RES_Y);

    rectangles (cr);
    cairo_translate (cr, 3*SIZE, 0);
    cairo_push_group (cr);
    rectangles (cr);
    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (fallback,
	    "Check that fallback images are correct when fallback resolution is not 72ppi",
	    "fallback", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
