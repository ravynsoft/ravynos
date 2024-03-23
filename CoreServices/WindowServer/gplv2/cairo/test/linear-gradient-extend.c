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
#define HEIGHT 16
#define WIDTH 16
#define PAD 3

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

    pattern = cairo_pattern_create_linear (0, 2*PAD, 0, HEIGHT - 2*PAD);

    cairo_pattern_add_color_stop_rgb (pattern, 0, 0, 0, 1);
    cairo_pattern_add_color_stop_rgb (pattern, 1, 0, 0, 1);

    cairo_translate (cr, PAD, PAD);

    for (i = 0; i < 2; i++) {
        cairo_save (cr);
	
	for (j = 0; j < NUM_EXTEND; j++) {
	    cairo_pattern_set_extend (pattern, extend[j]);

	    cairo_reset_clip (cr);
	    cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
	    cairo_clip (cr);

	    if (i & 1) {
	        cairo_set_source_rgb (cr, 0, 1, 0);
		cairo_mask (cr, pattern);
	    } else {
	        cairo_set_source (cr, pattern);
	        cairo_paint (cr);
	    }

	    cairo_translate (cr, WIDTH+PAD, 0);
	}

	cairo_restore (cr);
	cairo_translate (cr, 0, HEIGHT+PAD);
    }

    cairo_pattern_destroy (pattern);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (linear_gradient_extend,
	    "Tests gradient to solid reduction of linear gradients",
	    "linear, pattern, extend", /* keywords */
	    NULL, /* requirements */
	    (WIDTH+PAD) * NUM_EXTEND + PAD, 2*(HEIGHT + PAD) + PAD,
	    NULL, draw)
