/*
 * Copyright © 2005, 2007 Red Hat, Inc.
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

#define SIZE 20
#define PAD 2

static cairo_pattern_t *
create_image_source (int size)
{
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    cairo_t *cr;

    /* Create an image surface with my favorite four colors in each
     * quadrant. */
    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, size, size);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr, 0, 0, size / 2, size / 2);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_rectangle (cr, size / 2, 0, size - size / 2, size / 2);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_rectangle (cr, 0, size / 2, size / 2, size - size / 2);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_rectangle (cr, size / 2, size / 2, size - size / 2, size - size / 2);
    cairo_fill (cr);

    pattern = cairo_pattern_create_for_surface (cairo_get_target (cr));
    cairo_destroy (cr);

    return pattern;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *source;
    int surface_size = sqrt ((SIZE - 2*PAD)*(SIZE - 2*PAD)/2);

    /* Use a gray (neutral) background, so we can spot if the backend pads
     * with any other colour.
     */
    cairo_set_source_rgb (cr, .5, .5, .5);
    cairo_paint (cr);

    cairo_translate(cr, SIZE/2, SIZE/2);
    cairo_rotate (cr, M_PI / 4.0);
    cairo_translate (cr, -surface_size/2, -surface_size/2);

    source = create_image_source (surface_size);
    cairo_pattern_set_filter (source, CAIRO_FILTER_NEAREST);
    cairo_set_source(cr, source);
    cairo_pattern_destroy (source);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
clip_draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *source;
    int surface_size = sqrt ((SIZE - 2*PAD)*(SIZE - 2*PAD)/2);

    /* Use a gray (neutral) background, so we can spot if the backend pads
     * with any other colour.
     */
    cairo_set_source_rgb (cr, .5, .5, .5);
    cairo_paint (cr);

    cairo_rectangle (cr, 2*PAD, 2*PAD, SIZE-4*PAD, SIZE-4*PAD);
    cairo_clip (cr);

    cairo_translate(cr, SIZE/2, SIZE/2);
    cairo_rotate (cr, M_PI / 4.0);
    cairo_translate (cr, -surface_size/2, -surface_size/2);

    source = create_image_source (surface_size);
    cairo_pattern_set_filter (source, CAIRO_FILTER_NEAREST);
    cairo_set_source(cr, source);
    cairo_pattern_destroy (source);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_clip (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *source;
    int surface_size = sqrt ((SIZE - 2*PAD)*(SIZE - 2*PAD)/2);

    /* Use a gray (neutral) background, so we can spot if the backend pads
     * with any other colour.
     */
    cairo_set_source_rgb (cr, .5, .5, .5);
    cairo_paint (cr);

    cairo_translate(cr, SIZE/2, SIZE/2);
    cairo_rotate (cr, M_PI / 4.0);
    cairo_translate (cr, -surface_size/2, -surface_size/2);

    cairo_rectangle (cr, PAD, PAD, surface_size-2*PAD, surface_size-2*PAD);
    cairo_clip (cr);

    source = create_image_source (surface_size);
    cairo_pattern_set_filter (source, CAIRO_FILTER_NEAREST);
    cairo_set_source(cr, source);
    cairo_pattern_destroy (source);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (rotate_image_surface_paint,
	    "Test call sequence: image_surface_create; rotate; set_source_surface; paint"
	    "\nThis test is known to fail on the ps backend currently",
	    "image, transform, paint", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)

CAIRO_TEST (clip_rotate_image_surface_paint,
	    "Test call sequence: image_surface_create; rotate; set_source_surface; paint"
	    "\nThis test is known to fail on the ps backend currently",
	    "image, transform, paint", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, clip_draw)
CAIRO_TEST (rotate_clip_image_surface_paint,
	    "Test call sequence: image_surface_create; rotate; set_source_surface; paint"
	    "\nThis test is known to fail on the ps backend currently",
	    "image, transform, paint", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw_clip)
