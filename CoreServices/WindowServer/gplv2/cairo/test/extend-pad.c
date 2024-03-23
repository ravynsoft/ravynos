/*
 * Copyright © 2007 Red Hat, Inc.
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
 * Author: Behdad Esfahbod <behdad@behdad.org>
 */

#include "cairo-test.h"
#include <math.h>
#include <stdio.h>

#define SIZE 90

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *surface;
    cairo_t *cr_surface;

    /* Create a 4-pixel image surface with my favorite four colors in each
     * quadrant. */
    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 2, 2);
    cr_surface = cairo_create (surface);
    cairo_surface_destroy (surface);

    /* upper-left = white */
    cairo_set_source_rgb (cr_surface, 1, 1, 1);
    cairo_rectangle (cr_surface, 0, 0, 1, 1);
    cairo_fill (cr_surface);

    /* upper-right = red */
    cairo_set_source_rgb (cr_surface, 1, 0, 0);
    cairo_rectangle (cr_surface, 1, 0, 1, 1);
    cairo_fill (cr_surface);

    /* lower-left = green */
    cairo_set_source_rgb (cr_surface, 0, 1, 0);
    cairo_rectangle (cr_surface, 0, 1, 1, 1);
    cairo_fill (cr_surface);

    /* lower-right = blue */
    cairo_set_source_rgb (cr_surface, 0, 0, 1);
    cairo_rectangle (cr_surface, 1, 1, 1, 1);
    cairo_fill (cr_surface);

    /* Now use extend pad to cover the entire surface with those 4 colors */
    cairo_set_source_surface (cr, cairo_get_target (cr_surface),
			      width/2  - 1,
			      height/2 - 1);
    cairo_destroy (cr_surface);
    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_PAD);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (extend_pad,
	    "Test CAIRO_EXTEND_PAD for surface patterns",
	    "extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
