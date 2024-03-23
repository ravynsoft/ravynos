/*
 * Copyright © 2009 Adrian Johnson
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
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"

#define SIZE 100
#define PAD 15
#define WIDTH (PAD + SIZE + PAD)
#define HEIGHT WIDTH


/* This test is designed to paint a mesh pattern with a simple
 * fold. */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;

    cairo_test_paint_checkered (cr);

    cairo_translate (cr, PAD, PAD);

    pattern = cairo_pattern_create_mesh ();

    cairo_mesh_pattern_begin_patch (pattern);

    cairo_mesh_pattern_move_to (pattern, 0, 0);
    cairo_mesh_pattern_curve_to (pattern,  30, -30, 60,  30, 100, 0);
    cairo_mesh_pattern_curve_to (pattern, 130, 140, 60, -40, 100, 100);
    cairo_mesh_pattern_curve_to (pattern,  60,  70, 30, 130,   0, 100);
    cairo_mesh_pattern_curve_to (pattern, -30, -40, 30, 140,   0, 0);

    cairo_mesh_pattern_set_corner_color_rgb (pattern, 0, 1, 0, 0);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 1, 0, 1, 0);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 2, 0, 0, 1);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 3, 1, 1, 0);

    cairo_mesh_pattern_end_patch (pattern);

    cairo_set_source (cr, pattern);
    cairo_paint (cr);
    cairo_pattern_destroy (pattern);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (mesh_pattern_overlap,
	    "Paint a mesh pattern with a simple fold",
	    "mesh, pattern", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
