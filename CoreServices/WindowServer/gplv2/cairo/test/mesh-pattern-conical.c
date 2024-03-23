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
#include <math.h>

#define PAT_WIDTH  100
#define PAT_HEIGHT 100
#define SIZE PAT_WIDTH
#define PAD 2
#define WIDTH (PAD + SIZE + PAD)
#define HEIGHT WIDTH


/*
 * This test is designed to paint a mesh pattern which contains 8
 * circular sectors approximating a conical gradient.
 */

#define CENTER_X 50
#define CENTER_Y 50
#define RADIUS   50

static void
sector_patch (cairo_pattern_t *pattern,
	      double angle_A,
	      double A_r, double A_g, double A_b,
	      double angle_B,
	      double B_r, double B_g, double B_b)
{
    double r_sin_A, r_cos_A;
    double r_sin_B, r_cos_B;
    double h;

    r_sin_A = RADIUS * sin (angle_A);
    r_cos_A = RADIUS * cos (angle_A);
    r_sin_B = RADIUS * sin (angle_B);
    r_cos_B = RADIUS * cos (angle_B);

    h = 4.0/3.0 * tan ((angle_B - angle_A) / 4.0);

    cairo_mesh_pattern_begin_patch (pattern);

    cairo_mesh_pattern_move_to (pattern, CENTER_X, CENTER_Y);
    cairo_mesh_pattern_line_to (pattern,
				CENTER_X + r_cos_A,
				CENTER_Y + r_sin_A);

    cairo_mesh_pattern_curve_to (pattern,
				 CENTER_X + r_cos_A - h * r_sin_A,
				 CENTER_Y + r_sin_A + h * r_cos_A,
				 CENTER_X + r_cos_B + h * r_sin_B,
				 CENTER_Y + r_sin_B - h * r_cos_B,
				 CENTER_X + r_cos_B,
				 CENTER_Y + r_sin_B);

    cairo_mesh_pattern_set_corner_color_rgb (pattern, 0, 1, 1, 1);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 1, A_r, A_g, A_b);
    cairo_mesh_pattern_set_corner_color_rgb (pattern, 2, B_r, B_g, B_b);

    cairo_mesh_pattern_end_patch (pattern);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_translate (cr, PAD, PAD);

    pattern = cairo_pattern_create_mesh ();
    sector_patch (pattern,
		  0,         1, 0, 0,
		  M_PI/4,    1, 1, 0);
    sector_patch (pattern,
		  M_PI/4,    0, 1, 0,
		  M_PI/2,    0, 1, 1);
    sector_patch (pattern,
		  M_PI/2,    0, 0, 1,
		  3*M_PI/4,  1, 0, 1);
    sector_patch (pattern,
		  3*M_PI/4,  1, 0, 0,
		  M_PI,      1, 1, 0);
    sector_patch (pattern,
		  -M_PI,     1, 1, 0,
		  -3*M_PI/4, 0, 1, 0);
    sector_patch (pattern,
		  -3*M_PI/4, 0, 1, 0,
		  -M_PI/2,   0, 1, 1);
    sector_patch (pattern,
		  -M_PI/2,   0, 1, 1,
		  -M_PI/4,   0, 0, 1);
    sector_patch (pattern,
		  -M_PI/4,   0, 0, 1,
		  0,         1, 0, 0);

    cairo_set_source (cr, pattern);
    cairo_paint (cr);
    cairo_pattern_destroy (pattern);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (mesh_pattern_conical,
	    "Paint a conical pattern using a mesh pattern",
	    "conical, mesh, pattern", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
