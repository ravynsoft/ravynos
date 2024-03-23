/*
 * Copyright © 2005 Red Hat, Inc.
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
#include <cairo-ft.h>

static void
_stress_font_cache (FT_Face ft_face, cairo_t *cr, int lvl);

static cairo_font_face_t *
_load_font (FT_Face ft_face, int flags, cairo_t *cr, int lvl)
{
    cairo_font_face_t *font_face;
    cairo_font_extents_t font_extents;

    _stress_font_cache (ft_face, cr, lvl+1);

    font_face = cairo_ft_font_face_create_for_ft_face (ft_face, flags);

    cairo_set_font_face (cr, font_face);
    cairo_font_extents (cr, &font_extents);

    _stress_font_cache (ft_face, cr, lvl+1);

    return font_face;
}

static void
_stress_font_cache (FT_Face ft_face, cairo_t *cr, int lvl)
{
#define A _load_font (ft_face, 0, cr, lvl)
#define B _load_font (ft_face, FT_LOAD_NO_BITMAP, cr, lvl)
#define C _load_font (ft_face, FT_LOAD_NO_RECURSE, cr, lvl)
#define D _load_font (ft_face, FT_LOAD_FORCE_AUTOHINT, cr, lvl)

    cairo_font_face_t *font_face[4];

    while (lvl++ < 5) {
	font_face[0] = A; font_face[1] = A;
	font_face[2] = A; font_face[3] = A;
	cairo_font_face_destroy (font_face[0]);
	cairo_font_face_destroy (font_face[1]);
	cairo_font_face_destroy (font_face[2]);
	cairo_font_face_destroy (font_face[3]);

	font_face[0] = A; font_face[1] = B;
	font_face[2] = C; font_face[3] = D;
	cairo_font_face_destroy (font_face[0]);
	cairo_font_face_destroy (font_face[1]);
	cairo_font_face_destroy (font_face[2]);
	cairo_font_face_destroy (font_face[3]);

	font_face[0] = A; font_face[1] = B;
	font_face[2] = C; font_face[3] = D;
	cairo_font_face_destroy (font_face[3]);
	cairo_font_face_destroy (font_face[2]);
	cairo_font_face_destroy (font_face[1]);
	cairo_font_face_destroy (font_face[0]);

	font_face[0] = A;
	font_face[1] = A;
	cairo_font_face_destroy (font_face[0]);
	font_face[2] = A;
	cairo_font_face_destroy (font_face[1]);
	font_face[3] = A;
	cairo_font_face_destroy (font_face[2]);
	cairo_font_face_destroy (font_face[3]);

	font_face[0] = A;
	font_face[1] = B;
	cairo_font_face_destroy (font_face[0]);
	font_face[2] = C;
	cairo_font_face_destroy (font_face[1]);
	font_face[3] = D;
	cairo_font_face_destroy (font_face[2]);
	cairo_font_face_destroy (font_face[3]);
    }

#undef A
#undef B
#undef C
#undef D
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    FcPattern *pattern, *resolved;
    FcResult result;
    cairo_font_face_t *font_face;
    cairo_scaled_font_t *scaled_font;
    cairo_font_options_t *font_options;
    cairo_font_extents_t font_extents;
    cairo_matrix_t font_matrix, ctm;
    FT_Face ft_face;

    /* We're trying here to get our hands on _some_ FT_Face but we do
     * not at all care which one. So we start with an empty pattern
     * and do the minimal substitution on it in order to get a valid
     * pattern.
     *
     * Do not use this in production code! */
    pattern = FcPatternCreate ();
    if (! pattern) {
	cairo_test_log (ctx, "FcPatternCreate failed.\n");
	return cairo_test_status_from_status (ctx, CAIRO_STATUS_NO_MEMORY);
    }

    FcConfigSubstitute (NULL, pattern, FcMatchPattern);
    FcDefaultSubstitute (pattern);
    resolved = FcFontMatch (NULL, pattern, &result);
    if (! resolved) {
	FcPatternDestroy (pattern);
	cairo_test_log (ctx, "FcFontMatch failed.\n");
	return cairo_test_status_from_status (ctx, CAIRO_STATUS_NO_MEMORY);
    }

    font_face = cairo_ft_font_face_create_for_pattern (resolved);
    if (cairo_font_face_status (font_face)) {
	FcPatternDestroy (resolved);
	FcPatternDestroy (pattern);
	return cairo_test_status_from_status (ctx, cairo_font_face_status (font_face));
    }

    if (cairo_font_face_get_type (font_face) != CAIRO_FONT_TYPE_FT) {
	cairo_test_log (ctx, "Unexpected value from cairo_font_face_get_type: %d (expected %d)\n",
			cairo_font_face_get_type (font_face), CAIRO_FONT_TYPE_FT);
	cairo_font_face_destroy (font_face);
	FcPatternDestroy (resolved);
	FcPatternDestroy (pattern);
	return CAIRO_TEST_FAILURE;
    }

    cairo_matrix_init_identity (&font_matrix);

    cairo_get_matrix (cr, &ctm);

    font_options = cairo_font_options_create ();

    cairo_get_font_options (cr, font_options);

    scaled_font = cairo_scaled_font_create (font_face,
					    &font_matrix,
					    &ctm,
					    font_options);

    cairo_font_options_destroy (font_options);
    cairo_font_face_destroy (font_face);
    FcPatternDestroy (pattern);
    FcPatternDestroy (resolved);

    if (cairo_scaled_font_status (scaled_font)) {
	return cairo_test_status_from_status (ctx,
					      cairo_scaled_font_status (scaled_font));
    }

    if (cairo_scaled_font_get_type (scaled_font) != CAIRO_FONT_TYPE_FT) {
	cairo_test_log (ctx, "Unexpected value from cairo_scaled_font_get_type: %d (expected %d)\n",
			cairo_scaled_font_get_type (scaled_font), CAIRO_FONT_TYPE_FT);
	cairo_scaled_font_destroy (scaled_font);
	return CAIRO_TEST_FAILURE;
    }

    ft_face = cairo_ft_scaled_font_lock_face (scaled_font);
    if (ft_face == NULL) {
	cairo_test_log (ctx, "Failed to get an ft_face with cairo_ft_scaled_font_lock_face\n");
	cairo_scaled_font_destroy (scaled_font);
	return CAIRO_TEST_FAILURE;
    }

    /* phew, that was a lot of work. But at least we didn't ever have
     * to call freetype directly, nor did we have to make many (any?)
     * assumptions about the current system.
     *
     * Now, on to the simple thing we actually want to test.
     */

    cairo_save (cr);

    /* First we want to test caching behaviour */
    _stress_font_cache (ft_face, cr, 0);

    /* Set the font_face and force cairo to actually use it for
     * something. */
    font_face = cairo_ft_font_face_create_for_ft_face (ft_face, 0);
    cairo_set_font_face (cr, font_face);
    cairo_font_extents (cr, &font_extents);

    cairo_restore (cr);

    /* Finally, even more cleanup */
    cairo_font_face_destroy (font_face);
    cairo_ft_scaled_font_unlock_face (scaled_font);
    cairo_scaled_font_destroy (scaled_font);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (ft_font_create_for_ft_face,
	    "Simple test to verify that cairo_ft_font_create_for_ft_face doesn't crash.",
	    "ft, font", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    NULL, draw)

