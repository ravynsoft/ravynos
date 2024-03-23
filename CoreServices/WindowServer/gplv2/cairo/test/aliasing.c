/*
 * Copyright 2010 Intel Corporation
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

/* A fun little test to explore color fringing in various experimental
 * subpixel rasterisation techniques.
 */

#define WIDTH 60
#define HEIGHT 40

static const  struct color {
    double red, green, blue;
} color[] = {
    { 1, 1, 1 },
    { 0, 0, 0 },
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 },
    { 1, 1, 0 },
    { 0, 1, 1 },
    { 1, 0, 1 },
    { .5, .5, .5 },
};

#define NUM_COLORS ARRAY_LENGTH (color)

static void
object (cairo_t *cr, const struct color *fg, const struct color *bg)
{
    cairo_set_source_rgb (cr, bg->red, bg->green, bg->blue);
    cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, fg->red, fg->green, fg->blue);
    cairo_save (cr);
    cairo_scale (cr, WIDTH, HEIGHT);
    cairo_arc (cr, .5, .5, .5 - 4. / MAX (WIDTH, HEIGHT), 0, 2 * M_PI);
    cairo_fill (cr);
    cairo_arc (cr, .5, .5, .5 - 2. / MAX (WIDTH, HEIGHT), 0, 2 * M_PI);
    cairo_restore (cr);
    cairo_set_line_width (cr, 1.);
    cairo_stroke (cr);

    cairo_set_source_rgb (cr, bg->red, bg->green, bg->blue);
    cairo_set_line_width (cr, 4.);
    cairo_move_to (cr, 4, HEIGHT-4);
    cairo_line_to (cr, WIDTH-12, 4);
    cairo_move_to (cr, 12, HEIGHT-4);
    cairo_line_to (cr, WIDTH-4, 4);
    cairo_stroke (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    unsigned int i, j;

    for (i = 0; i < NUM_COLORS; i++) {
	for (j = 0; j < NUM_COLORS; j++) {
	    cairo_save (cr);
	    cairo_translate (cr, i * WIDTH, j * HEIGHT);
	    object (cr, &color[i], &color[j]);
	    cairo_restore (cr);
	}
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (aliasing,
	    "Check for subpixel aliasing and color fringing",
	    "rasterisation", /* keywords */
	    "target=raster", /* requirements */
	    NUM_COLORS * WIDTH, NUM_COLORS * HEIGHT,
	    NULL, draw)
