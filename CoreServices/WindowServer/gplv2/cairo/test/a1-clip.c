/*
 * Copyright © 2008 Red Hat, Inc.
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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define POINTS	10
#define STEP	(1.0 / POINTS)
#define PAD	1
#define WIDTH	(PAD + POINTS * 2 + PAD)
#define HEIGHT	(WIDTH)

static cairo_test_status_t
paint (cairo_t *cr, int width, int height)
{
    int i, j;

    /* Fill background white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* Draw in black */
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_translate (cr, PAD, PAD);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    for (i = 0; i < POINTS; i++)
	for (j = 0; j < POINTS; j++) {
	    cairo_save (cr);
	    cairo_rectangle (cr, 2 * i + i * STEP, 2 * j + j * STEP, 1, 1);
	    cairo_clip (cr);
	    cairo_paint (cr);
	    cairo_restore (cr);
	}

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
fill_equal (cairo_t *cr, int width, int height)
{
    int i, j;

    /* Fill background white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* Draw in black */
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_translate (cr, PAD, PAD);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    for (i = 0; i < POINTS; i++)
	for (j = 0; j < POINTS; j++) {
	    cairo_save (cr);
	    cairo_rectangle (cr, 2 * i + i * STEP, 2 * j + j * STEP, 1, 1);
	    cairo_clip_preserve (cr);
	    cairo_fill (cr);
	    cairo_restore (cr);
	}

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
fill (cairo_t *cr, int width, int height)
{
    int i, j;

    /* Fill background white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* Draw in black */
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_translate (cr, PAD, PAD);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    for (i = 0; i < POINTS; i++)
	for (j = 0; j < POINTS; j++) {
	    cairo_save (cr);
	    cairo_rectangle (cr, 2 * i + i * STEP, 2 * j + j * STEP, 1, 1);
	    cairo_clip (cr);
	    cairo_rectangle (cr, 2 * i, 2 * j, 2, 2);
	    cairo_fill (cr);
	    cairo_restore (cr);
	}

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
stroke (cairo_t *cr, int width, int height)
{
    int i, j;

    /* Fill background white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* Draw in black */
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_translate (cr, PAD, PAD);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    cairo_set_line_width (cr, 2);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);

    for (i = 0; i < POINTS; i++)
	for (j = 0; j < POINTS; j++) {
	    cairo_save (cr);
	    cairo_rectangle (cr, 2 * i + i * STEP, 2 * j + j * STEP, 1, 1);
	    cairo_clip (cr);
	    cairo_move_to (cr, 2 * i, 2 * j + 1);
	    cairo_line_to (cr, 2 * i + 2, 2 * j + 1);
	    cairo_stroke (cr);
	    cairo_restore (cr);
	}

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (a1_clip_paint,
	    "Test sample position when drawing trapezoids with ANTIALIAS_NONE",
	    "alpha, clip", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, paint)

CAIRO_TEST (a1_clip_fill,
	    "Test sample position when drawing trapezoids with ANTIALIAS_NONE",
	    "alpha, clip", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, fill)

CAIRO_TEST (a1_clip_fill_equal,
	    "Test sample position when drawing trapezoids with ANTIALIAS_NONE",
	    "alpha, clip", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, fill_equal)

CAIRO_TEST (a1_clip_stroke,
	    "Test sample position when drawing trapezoids with ANTIALIAS_NONE",
	    "alpha, clip", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, stroke)
