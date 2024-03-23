/*
 * Copyright © 2011 Intel Corporation
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
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define STEP	5
#define WIDTH	100
#define HEIGHT	100

static void hatching (cairo_t *cr)
{
    int i;

    cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
    cairo_clip (cr);

    cairo_translate (cr, WIDTH/2, HEIGHT/2);
    cairo_rotate (cr, M_PI/4);
    cairo_translate (cr, -WIDTH/2, -HEIGHT/2);

    for (i = 0; i < WIDTH; i += STEP) {
	cairo_rectangle (cr, i, -2, 1, HEIGHT+4);
	cairo_rectangle (cr, -2, i, WIDTH+4, 1);
    }
}

static void background (cairo_t *cr)
{
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);
}

static void clip_to_grid (cairo_t *cr)
{
    int i, j;

    for (j = 0; j < HEIGHT; j += 2*STEP) {
	for (i = 0; i < WIDTH; i += 2*STEP)
	    cairo_rectangle (cr, i, j, STEP, STEP);

	j += 2*STEP;
	for (i = 0; i < WIDTH; i += 2*STEP)
	    cairo_rectangle (cr, i+STEP/2, j, STEP, STEP);
    }

    cairo_clip (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    background (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    cairo_save (cr); {
	clip_to_grid (cr);
	hatching (cr);
	cairo_set_source_rgb (cr, 1, 0, 0);
	cairo_fill (cr);
    } cairo_restore (cr);

    cairo_translate (cr, 0.25, HEIGHT+.25);

    cairo_save (cr); {
	clip_to_grid (cr);
	hatching (cr);
	cairo_set_source_rgb (cr, 0, 0, 1);
	cairo_fill (cr);
    } cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_disjoint_hatching,
	    "Test drawing through through an array of clips",
	    "clip", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, 2*HEIGHT,
	    NULL, draw)
