/*
 * Copyright © 2006 Jinghua Luo
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
 * JINGHUA LUO DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Jinghua Luo <sunmoon1997@gmail.com>
 * Derived from:
 *  text-antialias-none.c,
 *  ft-font-create-for-ft-face.c.
 * Original Author: Carl D. Worth <cworth@cworth.org>
 */
#include "cairo-test.h"
#include <cairo-ft.h>

#define WIDTH  40
#define HEIGHT 30
#define TEXT_SIZE 12

static cairo_status_t
create_scaled_font (cairo_t * cr,
		    cairo_scaled_font_t **out)
{
    FcPattern *pattern, *resolved;
    FcResult result;
    cairo_font_face_t *font_face;
    cairo_scaled_font_t *scaled_font;
    cairo_font_options_t *font_options;
    cairo_matrix_t font_matrix, ctm;
    cairo_status_t status;
    double pixel_size;

    font_options = cairo_font_options_create ();

    cairo_get_font_options (cr, font_options);

    pattern = FcPatternCreate ();
    if (pattern == NULL)
	return CAIRO_STATUS_NO_MEMORY;

    FcPatternAddString (pattern, FC_FAMILY, (FcChar8 *) CAIRO_TEST_FONT_FAMILY " Sans");
    FcPatternAddDouble (pattern, FC_SIZE, TEXT_SIZE);
    FcConfigSubstitute (NULL, pattern, FcMatchPattern);

    cairo_ft_font_options_substitute (font_options, pattern);

    FcDefaultSubstitute (pattern);
    resolved = FcFontMatch (NULL, pattern, &result);
    if (resolved == NULL) {
	FcPatternDestroy (pattern);
	return CAIRO_STATUS_NO_MEMORY;
    }

    /* turn antialiasing off */
    FcPatternDel (resolved, FC_ANTIALIAS);
    FcPatternAddBool (resolved, FC_ANTIALIAS, FcFalse);

    FcPatternGetDouble (resolved, FC_PIXEL_SIZE, 0, &pixel_size);

    font_face = cairo_ft_font_face_create_for_pattern (resolved);

    cairo_matrix_init_identity (&font_matrix);
    cairo_matrix_scale (&font_matrix, pixel_size, pixel_size);

    cairo_get_matrix (cr, &ctm);

    scaled_font = cairo_scaled_font_create (font_face,
					    &font_matrix,
					    &ctm,
					    font_options);

    cairo_font_options_destroy (font_options);
    cairo_font_face_destroy (font_face);
    FcPatternDestroy (pattern);
    FcPatternDestroy (resolved);

    status = cairo_scaled_font_status (scaled_font);
    if (status) {
	cairo_scaled_font_destroy (scaled_font);
	return status;
    }

    *out = scaled_font;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_text_extents_t extents;
    cairo_scaled_font_t *scaled_font;
    cairo_status_t status;
    const char black[] = "black", blue[] = "blue";

    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    status = create_scaled_font (cr, &scaled_font);
    if (status) {
	return cairo_test_status_from_status (cairo_test_get_context (cr),
					      status);
    }

    cairo_set_scaled_font (cr, scaled_font);

    cairo_set_source_rgb (cr, 0, 0, 0); /* black */
    cairo_text_extents (cr, black, &extents);
    cairo_move_to (cr, -extents.x_bearing, -extents.y_bearing);
    cairo_show_text (cr, black);
    cairo_translate (cr, 0, -extents.y_bearing + 1);

    cairo_set_source_rgb (cr, 0, 0, 1); /* blue */
    cairo_text_extents (cr, blue, &extents);
    cairo_move_to (cr, -extents.x_bearing, -extents.y_bearing);
    cairo_show_text (cr, blue);

    cairo_scaled_font_destroy (scaled_font);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (ft_text_antialias_none,
	    "Tests text rendering with no antialiasing",
	    "ft, text", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
