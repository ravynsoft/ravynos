/*
 * Copyright © 2006 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define TEXT_SIZE 12
#define PAD 4
#define TEXT "text"

static cairo_bool_t
text_extents_equal (const cairo_text_extents_t *A,
	            const cairo_text_extents_t *B)
{
    return A->x_bearing == B->x_bearing &&
	   A->y_bearing == B->y_bearing &&
	   A->width     == B->width     &&
	   A->height    == B->height    &&
	   A->x_advance == B->x_advance &&
	   A->y_advance == B->y_advance;
}

static cairo_test_status_t
box_text (const cairo_test_context_t *ctx, cairo_t *cr,
	  const char *utf8,
	  double x, double y)
{
    double line_width;
    cairo_text_extents_t extents = {0}, scaled_extents = {0};
    cairo_scaled_font_t *scaled_font;
    cairo_status_t status;

    cairo_save (cr);

    cairo_text_extents (cr, utf8, &extents);

    scaled_font = cairo_get_scaled_font (cr);
    cairo_scaled_font_text_extents (scaled_font, TEXT, &scaled_extents);
    status = cairo_scaled_font_status (scaled_font);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    if (! text_extents_equal (&extents, &scaled_extents)) {
        cairo_test_log (ctx,
			"Error: extents differ when they shouldn't:\n"
			"cairo_text_extents(); extents (%g, %g, %g, %g, %g, %g)\n"
			"cairo_scaled_font_text_extents(); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance,
		        scaled_extents.x_bearing, scaled_extents.y_bearing,
			scaled_extents.width, scaled_extents.height,
			scaled_extents.x_advance, scaled_extents.y_advance);
        return CAIRO_TEST_FAILURE;
    }

    line_width = cairo_get_line_width (cr);
    cairo_rectangle (cr,
		     x + extents.x_bearing - line_width / 2,
		     y + extents.y_bearing - line_width / 2,
		     extents.width  + line_width,
		     extents.height + line_width);
    cairo_stroke (cr);

    cairo_move_to (cr, x, y);
    cairo_show_text (cr, utf8);

    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_test_status_t status;
    cairo_text_extents_t extents;
    cairo_matrix_t matrix;

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, TEXT_SIZE);

    cairo_translate (cr, PAD, PAD);
    cairo_set_line_width (cr, 1.0);

    cairo_text_extents (cr, TEXT, &extents);

    /* Draw text and bounding box */
    cairo_set_source_rgb (cr, 0, 0, 0); /* black */
    status = box_text (ctx, cr, TEXT, 0, - extents.y_bearing);
    if (status)
	return status;

    /* Then draw again with the same coordinates, but with a font
     * matrix to position the text below and shifted a bit to the
     * right. */
    cairo_matrix_init_translate (&matrix, TEXT_SIZE / 2, TEXT_SIZE + PAD);
    cairo_matrix_scale (&matrix, TEXT_SIZE, TEXT_SIZE);
    cairo_set_font_matrix (cr, &matrix);

    cairo_set_source_rgb (cr, 0, 0, 1); /* blue */
    status = box_text (ctx, cr, TEXT, 0, - extents.y_bearing);
    if (status)
	return status;

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (font_matrix_translation,
	    "Test that translation in a font matrix can be used to offset a string",
	    "font", /* keywords */
	    NULL, /* requirements */
	    38, 34,
	    NULL, draw)
