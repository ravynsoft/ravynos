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
 * Authors: Carl D. Worth <cworth@cworth.org>
 *	    Owen Taylor <otaylor@redhat.com>
 */

#include "cairo-test.h"

/* This test exercises code that computes the extents of a surface
 * pattern with CAIRO_FILTER_BILINEAR, (where the filtering
 * effectively increases the extents of the pattern).
 *
 * The original bug was reported by Owen Taylor here:
 *
 *	bad clipping with EXTEND_NONE
 *	https://bugs.freedesktop.org/show_bug.cgi?id=15349
 */

#define SCALE	10
#define PAD	3
#define WIDTH	(PAD + 3 * SCALE + PAD)
#define HEIGHT	WIDTH

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *image;
    cairo_t *cr2;

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 2, 2);

    /* Fill with an opaque background to avoid a separate rgb24 ref image */
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    /* First check handling of pattern extents > surface extents */
    cairo_save (cr);
    cairo_scale (cr, width/2., height/2.);

    /* Create a solid black source to merge with the background */
    cr2 = cairo_create (image);
    cairo_set_source_rgb (cr2, 0, 0 ,0);
    cairo_paint (cr2);
    cairo_set_source_surface (cr, cairo_get_target (cr2), 0, 0);
    cairo_destroy (cr2);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_BILINEAR);
    cairo_paint (cr);
    cairo_restore (cr);

    /* Then scale to smaller so we can see the full bilinear extents */
    cairo_save (cr);
    cairo_translate (cr, PAD, PAD);
    cairo_scale (cr, SCALE, SCALE);
    cairo_translate (cr, 0.5, 0.5);

    /* Create a 2x2 blue+red checkerboard source */
    cr2 = cairo_create (image);
    cairo_set_source_rgb (cr2, 1, 0 ,0); /* red */
    cairo_paint (cr2);
    cairo_set_source_rgb (cr2, 0, 0, 1); /* blue */
    cairo_rectangle (cr2, 0, 1, 1, 1);
    cairo_rectangle (cr2, 1, 0, 1, 1);
    cairo_fill (cr2);
    cairo_set_source_surface (cr, cairo_get_target (cr2), 0, 0);
    cairo_destroy (cr2);

    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_BILINEAR);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_surface_destroy (image);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (filter_bilinear_extents,
	    "Test that pattern extents are properly computed for CAIRO_FILTER_BILINEAR",
	    "extents", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
