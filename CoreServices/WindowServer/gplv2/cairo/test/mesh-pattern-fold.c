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
#define WIDTH (5*SIZE)
#define HEIGHT (5*SIZE)


/* This test is designed to paint a mesh pattern which folds along
 * both parameters. */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;

    cairo_test_paint_checkered (cr);

    pattern = cairo_pattern_create_mesh ();

    cairo_mesh_pattern_begin_patch (pattern);

    cairo_mesh_pattern_move_to (pattern, 1, 1);

    cairo_mesh_pattern_curve_to (pattern, 6, 0, -1, 0, 4, 1);
    cairo_mesh_pattern_curve_to (pattern, 5, 6, 5, -1, 4, 4);
    cairo_mesh_pattern_curve_to (pattern, -1, 3, 6, 3, 1, 4);
    cairo_mesh_pattern_curve_to (pattern, 2, -1, 2, 6, 1, 1);

    cairo_mesh_pattern_set_control_point (pattern, 0, 2, 3);
    cairo_mesh_pattern_set_control_point (pattern, 1, 3, 3);
    cairo_mesh_pattern_set_control_point (pattern, 2, 3, 2);
    cairo_mesh_pattern_set_control_point (pattern, 3, 2, 2);

    cairo_mesh_pattern_set_corner_color_rgb (pattern, 0, 1, 0, 0);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 1, 0, 0, 1);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 2, 0, 0, 0);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 3, 0, 1, 0);

    cairo_mesh_pattern_end_patch (pattern);

    cairo_scale (cr, SIZE, SIZE);

    cairo_set_source (cr, pattern);
    cairo_paint (cr);
    cairo_pattern_destroy (pattern);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (mesh_pattern_fold,
	    "Paint a mesh pattern with complex folds",
	    "mesh, pattern", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
