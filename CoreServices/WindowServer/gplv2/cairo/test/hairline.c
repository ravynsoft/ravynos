/*
 * Copyright © 2021 Rick Yorgason
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
 */

#include "cairo-test.h"

/**
 * draw:
 * @cr: a #cairo_t
 * @width: Width of the test image
 * @height: Height of the test image
 * @scale_width: Percentage to scale the width
 * @scale_height: Percentage to scale the height
 * @fit_to_scale: Whether or not to adjust the image to fit in the scale parameters
 * regardless of the scale parameters
 * @correct_scale: Tests if the hairlines render correctly regardless of 
 * whether or not the scale is set "correctly", as per
 * https://cairographics.org/cookbook/ellipses/ 
 */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height, double scale_width, double scale_height, cairo_bool_t fit_to_scale, cairo_bool_t correct_scale)
{
	cairo_matrix_t save_matrix;
	double fit_width = fit_to_scale ? scale_width : 1.0;
	double fit_height = fit_to_scale ? scale_height : 1.0;
	double fit_max = MAX(fit_width, fit_height);
	double dash[] = {3.0};

	if (cairo_get_hairline (cr) == TRUE) {
		return CAIRO_TEST_ERROR;
	}

	/* Clear background */
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_paint (cr);

	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_set_line_width (cr, 100.0); /* If everything is working right, this value should never get used */

	/* Hairline sample */
	if (correct_scale) {
		cairo_get_matrix (cr, &save_matrix);
	}
	cairo_scale (cr, scale_width, scale_height);

	cairo_set_hairline (cr, TRUE);
	if (cairo_get_hairline (cr) == FALSE) {
		return CAIRO_TEST_ERROR;
	}

	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_move_to (cr, width/fit_width/2, 0);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_move_to (cr, 0, height/fit_height/2);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_move_to (cr, width/fit_width/4, 0);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_arc (cr, width/fit_width/2, height/fit_height/2, width/fit_max/4, M_PI*0.5, M_PI*1.0);

	if (correct_scale) {
		cairo_set_matrix (cr, &save_matrix);
	}
	cairo_stroke (cr);

	/* Dashed sample */
	if (correct_scale) {
		cairo_get_matrix (cr, &save_matrix);
		cairo_scale (cr, scale_width, scale_height);
	}
	cairo_set_dash (cr, dash, 1, 0);
	cairo_arc (cr, width/fit_width/2, height/fit_height/2, width/fit_max/4, M_PI*1.0, M_PI*1.5);
	if (correct_scale) {
		cairo_set_matrix (cr, &save_matrix);
	}
	cairo_stroke (cr);

	/* Control sample */
	if (correct_scale) {
		cairo_get_matrix (cr, &save_matrix);
		cairo_scale (cr, scale_width, scale_height);
	}

	cairo_set_line_width (cr, 3.0);
	cairo_set_hairline (cr, FALSE);
	if (cairo_get_hairline (cr) == TRUE) {
		return CAIRO_TEST_ERROR;
	}

	cairo_set_dash (cr, 0, 0, 0);

	cairo_move_to (cr, width/fit_width, height/fit_height);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_move_to (cr, width/fit_width/2, height/fit_height);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_move_to (cr, width/fit_width, height/fit_height/2);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_move_to (cr, width/fit_width*0.75, height/fit_height);
	cairo_line_to (cr, width/fit_width/2, height/fit_height/2);
	cairo_arc (cr, width/fit_width/2, height/fit_height/2, width/fit_max/4, M_PI*1.5, M_PI*2.0);

	if (correct_scale) {
		cairo_set_matrix (cr, &save_matrix);
	}
	cairo_stroke (cr);

	/* Dashed sample */
	if (correct_scale) {
		cairo_get_matrix (cr, &save_matrix);
		cairo_scale (cr, scale_width, scale_height);
	}
	cairo_set_dash (cr, dash, 1, 0);
	cairo_arc (cr, width/fit_width/2, height/fit_height/2, width/fit_max/4, 0, M_PI*0.5);
	if (correct_scale) {
		cairo_set_matrix (cr, &save_matrix);
	}
	cairo_stroke (cr);

	return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_typical (cairo_t *cr, int width, int height)
{
	return draw (cr, width, height, 1.0, 1.0, TRUE, TRUE);
}

static cairo_test_status_t
draw_scaled (cairo_t *cr, int width, int height)
{
	return draw (cr, width, height, 0.5, 0.5, FALSE, TRUE);
}

static cairo_test_status_t
draw_anisotropic (cairo_t *cr, int width, int height)
{
	return draw (cr, width, height, 2.0, 5.0, TRUE, TRUE);
}

static cairo_test_status_t
draw_anisotropic_incorrect (cairo_t *cr, int width, int height)
{
	return draw (cr, width, height, 2.0, 5.0, TRUE, FALSE);
}

CAIRO_TEST (hairline,
		    "Tests hairlines are drawn at a single pixel width",
			"path, stroke, hairline", /* keywords */
			NULL, /* requirements */
			49, 49,
			NULL, draw_typical)

CAIRO_TEST (hairline_big,
		    "Tests hairlines are drawn at a single pixel width",
			"path, stroke, hairline", /* keywords */
			NULL, /* requirements */
			99, 99,
			NULL, draw_typical)

CAIRO_TEST (hairline_scaled,
		    "Tests hairlines are drawn at a single pixel width",
			"path, stroke, hairline", /* keywords */
			NULL, /* requirements */
			99, 99,
			NULL, draw_scaled)

CAIRO_TEST (hairline_anisotropic,
		    "Tests hairlines with a really lopsided scale parameter",
			"path, stroke, hairline", /* keywords */
			NULL, /* requirements */
			99, 99,
			NULL, draw_anisotropic)

CAIRO_TEST (hairline_anisotropic_incorrect,
		    "Tests hairlines with a really lopsided scale parameter",
			"path, stroke, hairline", /* keywords */
			NULL, /* requirements */
			99, 99,
			NULL, draw_anisotropic_incorrect)
