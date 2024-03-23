/*
 * Copyright © 2007 Red Hat, Inc.
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
 * Author: Carl Worth <cworth@cworth.org>
 */

#define _ISOC99_SOURCE	/* for INFINITY */

#include "config.h"
#include "cairo-test.h"

#if !defined(INFINITY)
#define INFINITY HUGE_VAL
#endif

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_status_t status;
    cairo_surface_t *target;
    cairo_font_face_t *font_face;
    cairo_font_options_t *font_options;
    cairo_scaled_font_t *scaled_font;
    cairo_pattern_t *pattern;
    cairo_t *cr2;
    cairo_matrix_t identity, bogus, inf, invalid = {
	4.0, 4.0,
	4.0, 4.0,
	4.0, 4.0
    };

#define CHECK_STATUS(status, function_name)						\
if ((status) == CAIRO_STATUS_SUCCESS) {							\
    cairo_test_log (ctx, "Error: %s with invalid matrix passed\n",				\
		    (function_name));							\
    return CAIRO_TEST_FAILURE;								\
} else if ((status) != CAIRO_STATUS_INVALID_MATRIX) {					\
    cairo_test_log (ctx, "Error: %s with invalid matrix returned unexpected status "	\
		    "(%d): %s\n",							\
		    (function_name),							\
		    status,								\
		    cairo_status_to_string (status));					\
    return CAIRO_TEST_FAILURE;								\
}

    /* clear floating point exceptions (added by cairo_test_init()) */
#if HAVE_FEDISABLEEXCEPT
    fedisableexcept (FE_INVALID);
#endif

    /* create a bogus matrix and check results of attempted inversion */
    bogus.x0 = bogus.xy = bogus.xx = cairo_test_NaN ();
    bogus.y0 = bogus.yx = bogus.yy = bogus.xx;
    status = cairo_matrix_invert (&bogus);
    CHECK_STATUS (status, "cairo_matrix_invert(NaN)");

    inf.x0 = inf.xy = inf.xx = INFINITY;
    inf.y0 = inf.yx = inf.yy = inf.xx;
    status = cairo_matrix_invert (&inf);
    CHECK_STATUS (status, "cairo_matrix_invert(infinity)");

    /* test cairo_matrix_invert with invalid matrix */
    status = cairo_matrix_invert (&invalid);
    CHECK_STATUS (status, "cairo_matrix_invert(invalid)");


    cairo_matrix_init_identity (&identity);

    target = cairo_get_group_target (cr);

    /* test cairo_transform with invalid matrix */
    cr2 = cairo_create (target);
    cairo_transform (cr2, &invalid);

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_transform(invalid)");

    /* test cairo_transform with bogus matrix */
    cr2 = cairo_create (target);
    cairo_transform (cr2, &bogus);

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_transform(NaN)");

    /* test cairo_transform with ∞ matrix */
    cr2 = cairo_create (target);
    cairo_transform (cr2, &inf);

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_transform(infinity)");


    /* test cairo_set_matrix with invalid matrix */
    cr2 = cairo_create (target);
    cairo_set_matrix (cr2, &invalid);

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_set_matrix(invalid)");

    /* test cairo_set_matrix with bogus matrix */
    cr2 = cairo_create (target);
    cairo_set_matrix (cr2, &bogus);

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_set_matrix(NaN)");

    /* test cairo_set_matrix with ∞ matrix */
    cr2 = cairo_create (target);
    cairo_set_matrix (cr2, &inf);

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_set_matrix(infinity)");


    /* test cairo_set_font_matrix with invalid matrix */
    cr2 = cairo_create (target);
    cairo_set_font_matrix (cr2, &invalid);

    /* draw some text to force the font to be resolved */
    cairo_show_text (cr2, "hello");

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_set_font_matrix(invalid)");

    /* test cairo_set_font_matrix with bogus matrix */
    cr2 = cairo_create (target);
    cairo_set_font_matrix (cr2, &bogus);

    /* draw some text to force the font to be resolved */
    cairo_show_text (cr2, "hello");

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_set_font_matrix(NaN)");

    /* test cairo_set_font_matrix with ∞ matrix */
    cr2 = cairo_create (target);
    cairo_set_font_matrix (cr2, &inf);

    /* draw some text to force the font to be resolved */
    cairo_show_text (cr2, "hello");

    status = cairo_status (cr2);
    cairo_destroy (cr2);
    CHECK_STATUS (status, "cairo_set_font_matrix(infinity)");


    /* test cairo_scaled_font_create with invalid matrix */
    cr2 = cairo_create (target);
    font_face = cairo_get_font_face (cr2);
    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    scaled_font = cairo_scaled_font_create (font_face,
					    &invalid,
					    &identity,
					    font_options);
    status = cairo_scaled_font_status (scaled_font);
    CHECK_STATUS (status, "cairo_scaled_font_create(invalid)");

    cairo_scaled_font_destroy (scaled_font);

    scaled_font = cairo_scaled_font_create (font_face,
					    &identity,
					    &invalid,
					    font_options);
    status = cairo_scaled_font_status (scaled_font);
    CHECK_STATUS (status, "cairo_scaled_font_create(invalid)");

    cairo_scaled_font_destroy (scaled_font);
    cairo_font_options_destroy (font_options);
    cairo_destroy (cr2);

    /* test cairo_scaled_font_create with bogus matrix */
    cr2 = cairo_create (target);
    font_face = cairo_get_font_face (cr2);
    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    scaled_font = cairo_scaled_font_create (font_face,
					    &bogus,
					    &identity,
					    font_options);
    status = cairo_scaled_font_status (scaled_font);
    CHECK_STATUS (status, "cairo_scaled_font_create(NaN)");

    cairo_scaled_font_destroy (scaled_font);

    scaled_font = cairo_scaled_font_create (font_face,
					    &identity,
					    &bogus,
					    font_options);
    status = cairo_scaled_font_status (scaled_font);
    CHECK_STATUS (status, "cairo_scaled_font_create(NaN)");

    cairo_scaled_font_destroy (scaled_font);
    cairo_font_options_destroy (font_options);
    cairo_destroy (cr2);

    /* test cairo_scaled_font_create with ∞ matrix */
    cr2 = cairo_create (target);
    font_face = cairo_get_font_face (cr2);
    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    scaled_font = cairo_scaled_font_create (font_face,
					    &inf,
					    &identity,
					    font_options);
    status = cairo_scaled_font_status (scaled_font);
    CHECK_STATUS (status, "cairo_scaled_font_create(infinity)");

    cairo_scaled_font_destroy (scaled_font);

    scaled_font = cairo_scaled_font_create (font_face,
					    &identity,
					    &inf,
					    font_options);
    status = cairo_scaled_font_status (scaled_font);
    CHECK_STATUS (status, "cairo_scaled_font_create(infinity)");

    cairo_scaled_font_destroy (scaled_font);
    cairo_font_options_destroy (font_options);
    cairo_destroy (cr2);


    /* test cairo_pattern_set_matrix with invalid matrix */
    pattern = cairo_pattern_create_rgb (1.0, 1.0, 1.0);
    cairo_pattern_set_matrix (pattern, &invalid);
    status = cairo_pattern_status (pattern);
    CHECK_STATUS (status, "cairo_pattern_set_matrix(invalid)");
    cairo_pattern_destroy (pattern);

    /* test cairo_pattern_set_matrix with bogus matrix */
    pattern = cairo_pattern_create_rgb (1.0, 1.0, 1.0);
    cairo_pattern_set_matrix (pattern, &bogus);
    status = cairo_pattern_status (pattern);
    CHECK_STATUS (status, "cairo_pattern_set_matrix(NaN)");
    cairo_pattern_destroy (pattern);

    /* test cairo_pattern_set_matrix with ∞ matrix */
    pattern = cairo_pattern_create_rgb (1.0, 1.0, 1.0);
    cairo_pattern_set_matrix (pattern, &inf);
    status = cairo_pattern_status (pattern);
    CHECK_STATUS (status, "cairo_pattern_set_matrix(infinity)");
    cairo_pattern_destroy (pattern);


    /* test invalid transformations */
    cr2 = cairo_create (target);
    cairo_translate (cr2, bogus.xx, bogus.yy);
    CHECK_STATUS (status, "cairo_translate(NaN, NaN)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_translate (cr2, 0, bogus.yy);
    CHECK_STATUS (status, "cairo_translate(0, NaN)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_translate (cr2, bogus.xx, 0);
    CHECK_STATUS (status, "cairo_translate(NaN, 0)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_translate (cr2, inf.xx, inf.yy);
    CHECK_STATUS (status, "cairo_translate(∞, ∞)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_translate (cr2, 0, inf.yy);
    CHECK_STATUS (status, "cairo_translate(0, ∞)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_translate (cr2, inf.xx, 0);
    CHECK_STATUS (status, "cairo_translate(∞, 0)");
    cairo_destroy (cr2);


    cr2 = cairo_create (target);
    cairo_scale (cr2, bogus.xx, bogus.yy);
    CHECK_STATUS (status, "cairo_scale(NaN, NaN)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, 1, bogus.yy);
    CHECK_STATUS (status, "cairo_scale(1, NaN)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, bogus.xx, 1);
    CHECK_STATUS (status, "cairo_scale(NaN, 1)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, inf.xx, inf.yy);
    CHECK_STATUS (status, "cairo_scale(∞, ∞)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, 1, inf.yy);
    CHECK_STATUS (status, "cairo_scale(1, ∞)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, inf.xx, 1);
    CHECK_STATUS (status, "cairo_scale(∞, 1)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, bogus.xx, bogus.yy);
    CHECK_STATUS (status, "cairo_scale(0, 0)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, 1, bogus.yy);
    CHECK_STATUS (status, "cairo_scale(1, 0)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_scale (cr2, bogus.xx, 1);
    CHECK_STATUS (status, "cairo_scale(0, 1)");
    cairo_destroy (cr2);


    cr2 = cairo_create (target);
    cairo_rotate (cr2, bogus.xx);
    CHECK_STATUS (status, "cairo_rotate(NaN)");
    cairo_destroy (cr2);

    cr2 = cairo_create (target);
    cairo_rotate (cr2, inf.xx);
    CHECK_STATUS (status, "cairo_rotate(∞)");
    cairo_destroy (cr2);

#if HAVE_FECLEAREXCEPT
    feclearexcept (FE_INVALID);
#endif

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (invalid_matrix,
	    "Test that all relevant public functions return CAIRO_STATUS_INVALID_MATRIX as appropriate",
	    "api, matrix", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    NULL, draw)
