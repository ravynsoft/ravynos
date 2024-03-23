/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2009 Andrea Canciani
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

/* Lengths of the dashes of the dash patterns */
static const double dashes[] = { 2, 2, 4, 4 };
/* Dash offset in userspace units
 * They always grow by 2, so the dash pattern is
 * should be shifted by the same amount each time */
static const double frac_offset[] = { 0, 2, 4, 6 };
/* Dash offset relative to the whole dash pattern
 * This corresponds to the non-inverted part only if
 * the dash pattern has odd length, so the expected result
 * is the same for every int_offset if the pattern has
 * even length, and inverted each time (or shifted by half
 * period, which is the same) if the pattern has odd length. */
static const double int_offset[] = { -2, -1, 0, 1, 2 };

#define PAD 6
#define STROKE_LENGTH 32
#define IMAGE_WIDTH (PAD + (STROKE_LENGTH + PAD) * ARRAY_LENGTH (dashes))
#define IMAGE_HEIGHT (PAD + PAD * ARRAY_LENGTH (int_offset) + PAD * ARRAY_LENGTH (frac_offset) * ARRAY_LENGTH (int_offset))


static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double total;
    size_t i, j, k;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_line_width (cr, 2);

    total = 0.0;
    for (k = 0; k < ARRAY_LENGTH (dashes); ++k) {
	total += dashes[k];
	for (i = 0; i < ARRAY_LENGTH (frac_offset); ++i) {
	    for (j = 0; j < ARRAY_LENGTH (int_offset); ++j) {
		cairo_set_dash (cr, dashes, k + 1, frac_offset[i] + total * int_offset[j]);
		cairo_move_to (cr, (STROKE_LENGTH + PAD) * k + PAD, PAD * (i + j + ARRAY_LENGTH (frac_offset) * j + 1));
		cairo_line_to (cr, (STROKE_LENGTH + PAD) * (k + 1), PAD * (i + j + ARRAY_LENGTH (frac_offset) * j + 1));
		cairo_stroke (cr);
	    }
	}
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (dash_offset,
	    "Tests dashes of different length with various offsets",
	    "stroke, dash", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
