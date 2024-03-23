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
 * Author: Kristian Høgsberg <krh@redhat.com>
 */

#include "cairo-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <cairo.h>

#define WIDTH	32
#define HEIGHT	WIDTH

#define IMAGE_WIDTH	(3 * WIDTH)
#define IMAGE_HEIGHT	IMAGE_WIDTH

/* Draw the word cairo at NUM_TEXT different angles */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *stamp;
    cairo_t *cr2;

    /* Draw a translucent rectangle for reference where the rotated
     * image should be. */
    cairo_new_path (cr);
    cairo_rectangle (cr, WIDTH, HEIGHT, WIDTH, HEIGHT);
    cairo_set_source_rgba (cr, 1, 1, 0, 0.3);
    cairo_fill (cr);

#if 1 /* Set to 0 to generate reference image */
    cairo_translate (cr, 2 * WIDTH, 2 * HEIGHT);
    cairo_rotate (cr, M_PI);
#else
    cairo_translate (cr, WIDTH, HEIGHT);
#endif

    stamp = cairo_surface_create_similar (cairo_get_group_target (cr),
					  CAIRO_CONTENT_COLOR_ALPHA,
					  WIDTH, HEIGHT);
    cr2 = cairo_create (stamp);
    cairo_surface_destroy (stamp);
    {
	cairo_new_path (cr2);
	cairo_rectangle (cr2, WIDTH / 4, HEIGHT / 4, WIDTH / 2, HEIGHT / 2);
	cairo_set_source_rgba (cr2, 1, 0, 0, 0.8);
	cairo_fill (cr2);

	cairo_rectangle (cr2, 0, 0, WIDTH, HEIGHT);
	cairo_set_line_width (cr2, 2);
	cairo_set_source_rgb (cr2, 0, 0, 0);
	cairo_stroke (cr2);
    }
    cairo_set_source_surface (cr, cairo_get_target (cr2), 0, 0);
    cairo_destroy (cr2);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (pixman_rotate,
	    "Exposes pixman off-by-one error when rotating",
	    "image, transform", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
