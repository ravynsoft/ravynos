/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2010 Andrea Canciani
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
 * Author: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

#define NUM_EXTEND 4
#define HEIGHT 32
#define WIDTH 32
#define PAD 6

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;
    unsigned int i, j;

    cairo_extend_t extend[NUM_EXTEND] = {
	CAIRO_EXTEND_NONE,
	CAIRO_EXTEND_REPEAT,
	CAIRO_EXTEND_REFLECT,
	CAIRO_EXTEND_PAD
    };

    cairo_test_paint_checkered (cr);

    cairo_translate (cr, PAD, PAD);

    for (i = 0; i < 3; i++) {
        cairo_save (cr);
	
	for (j = 0; j < NUM_EXTEND; j++) {
	    cairo_reset_clip (cr);
	    cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
	    cairo_clip (cr);

	    if (i == 0)
		pattern = cairo_pattern_create_radial (WIDTH/2, HEIGHT/2, 0, WIDTH/2, HEIGHT/2, 0);
	    else if (i == 1)
		pattern = cairo_pattern_create_radial (WIDTH/2, HEIGHT/2, 2*PAD, WIDTH/2, HEIGHT/2, 2*PAD);
	    else if (i == 2)
		pattern = cairo_pattern_create_radial (PAD, PAD, 0, WIDTH-PAD, HEIGHT-PAD, 0);

	    cairo_pattern_add_color_stop_rgba (pattern, 0, 1, 0, 0, 1);
	    cairo_pattern_add_color_stop_rgba (pattern, sqrt (1.0 / 2.0), 0, 1, 0, 0);
	    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 1, 0.5);

	    cairo_pattern_set_extend (pattern, extend[j]);

	    cairo_set_source (cr, pattern);
	    cairo_paint (cr);

	    cairo_pattern_destroy (pattern);

	    cairo_translate (cr, WIDTH+PAD, 0);
	}

	cairo_restore (cr);
	cairo_translate (cr, 0, HEIGHT+PAD);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (degenerate_radial_gradient,
	    "Tests degenerate radial gradients",
	    "radial, pattern, extend", /* keywords */
	    NULL, /* requirements */
	    (WIDTH+PAD) * NUM_EXTEND + PAD, 3*(HEIGHT + PAD) + PAD,
	    NULL, draw)
