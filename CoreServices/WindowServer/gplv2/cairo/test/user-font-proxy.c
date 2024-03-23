/*
 * Copyright © 2006, 2008 Red Hat, Inc.
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
 * Contributor(s):
 *	Kristian Høgsberg <krh@redhat.com>
 *	Behdad Esfahbod <behdad@behdad.org>
 */

#include "cairo-test.h"

#include <stdlib.h>
#include <stdio.h>

/*#define ROTATED 1*/

#define BORDER 10
#define TEXT_SIZE 64
#define WIDTH  (TEXT_SIZE * 12 + 2*BORDER)
#ifndef ROTATED
 #define HEIGHT ((TEXT_SIZE + 2*BORDER)*2)
#else
 #define HEIGHT WIDTH
#endif

#define TEXT1   "cairo user-font."
#define TEXT2   " zg"

static cairo_user_data_key_t fallback_font_key;

static cairo_status_t
test_scaled_font_init (cairo_scaled_font_t  *scaled_font,
		       cairo_t              *cr,
		       cairo_font_extents_t *extents)
{
    cairo_status_t status;

    cairo_set_font_face (cr,
			 cairo_font_face_get_user_data (cairo_scaled_font_get_font_face (scaled_font),
							&fallback_font_key));

    status = cairo_scaled_font_set_user_data (scaled_font,
					      &fallback_font_key,
					      cairo_scaled_font_reference (cairo_get_scaled_font (cr)),
					      (cairo_destroy_func_t) cairo_scaled_font_destroy);
    if (unlikely (status)) {
	cairo_scaled_font_destroy (cairo_get_scaled_font (cr));
	return status;
    }

    cairo_font_extents (cr, extents);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
test_scaled_font_render_glyph (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *extents)
{
    cairo_glyph_t cairo_glyph;

    cairo_glyph.index = glyph;
    cairo_glyph.x = 0;
    cairo_glyph.y = 0;

    cairo_set_font_face (cr,
			 cairo_font_face_get_user_data (cairo_scaled_font_get_font_face (scaled_font),
							&fallback_font_key));

    cairo_show_glyphs (cr, &cairo_glyph, 1);
    cairo_glyph_extents (cr, &cairo_glyph, 1, extents);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
test_scaled_font_text_to_glyphs (cairo_scaled_font_t        *scaled_font,
				 const char	            *utf8,
				 int		             utf8_len,
				 cairo_glyph_t	           **glyphs,
				 int		            *num_glyphs,
				 cairo_text_cluster_t      **clusters,
				 int		            *num_clusters,
				 cairo_text_cluster_flags_t *cluster_flags)
{
  cairo_scaled_font_t *fallback_scaled_font;

  fallback_scaled_font = cairo_scaled_font_get_user_data (scaled_font,
							  &fallback_font_key);

  return cairo_scaled_font_text_to_glyphs (fallback_scaled_font, 0, 0,
					   utf8, utf8_len,
					   glyphs, num_glyphs,
					   clusters, num_clusters, cluster_flags);
}

static cairo_status_t
_user_font_face_create (cairo_font_face_t **out)
{
    cairo_font_face_t *user_font_face;
    cairo_font_face_t *fallback_font_face;
    cairo_status_t status;

    user_font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_init_func             (user_font_face, test_scaled_font_init);
    cairo_user_font_face_set_render_glyph_func     (user_font_face, test_scaled_font_render_glyph);
    cairo_user_font_face_set_text_to_glyphs_func   (user_font_face, test_scaled_font_text_to_glyphs);

    /* This also happens to be default font face on cairo_t, so does
     * not make much sense here.  For demonstration only.
     */
    fallback_font_face = cairo_toy_font_face_create (CAIRO_TEST_FONT_FAMILY " Sans",
						     CAIRO_FONT_SLANT_NORMAL,
						     CAIRO_FONT_WEIGHT_NORMAL);

    status = cairo_font_face_set_user_data (user_font_face,
					    &fallback_font_key,
					    fallback_font_face,
					    (cairo_destroy_func_t) cairo_font_face_destroy);
    if (status) {
	cairo_font_face_destroy (fallback_font_face);
	cairo_font_face_destroy (user_font_face);
	return status;
    }

    *out = user_font_face;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_font_extents_t font_extents;
    cairo_text_extents_t extents;
    cairo_font_face_t *font_face;
    cairo_status_t status;
    char full_text[100];

    strcpy(full_text, TEXT1);
    strcat(full_text, TEXT2);
    strcat(full_text, TEXT2);
    strcat(full_text, TEXT2);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

#ifdef ROTATED
    cairo_translate (cr, TEXT_SIZE, 0);
    cairo_rotate (cr, .6);
#endif

    status = _user_font_face_create (&font_face);
    if (status) {
	return cairo_test_status_from_status (cairo_test_get_context (cr),
					      status);
    }

    cairo_set_font_face (cr, font_face);
    cairo_font_face_destroy (font_face);

    cairo_set_font_size (cr, TEXT_SIZE);

    cairo_font_extents (cr, &font_extents);
    cairo_text_extents (cr, full_text, &extents);

    /* logical boundaries in red */
    cairo_move_to (cr, 0, BORDER);
    cairo_rel_line_to (cr, WIDTH, 0);
    cairo_move_to (cr, 0, BORDER + font_extents.ascent);
    cairo_rel_line_to (cr, WIDTH, 0);
    cairo_move_to (cr, 0, BORDER + font_extents.ascent + font_extents.descent);
    cairo_rel_line_to (cr, WIDTH, 0);
    cairo_move_to (cr, BORDER, 0);
    cairo_rel_line_to (cr, 0, 2*BORDER + TEXT_SIZE);
    cairo_move_to (cr, BORDER + extents.x_advance, 0);
    cairo_rel_line_to (cr, 0, 2*BORDER + TEXT_SIZE);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_set_line_width (cr, 2);
    cairo_stroke (cr);

    /* ink boundaries in green */
    cairo_rectangle (cr,
		     BORDER + extents.x_bearing, BORDER + font_extents.ascent + extents.y_bearing,
		     extents.width, extents.height);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_set_line_width (cr, 2);
    cairo_stroke (cr);


    /* TEXT1 in black */
    cairo_move_to (cr, BORDER, BORDER + font_extents.ascent);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_show_text (cr, TEXT1);

    /* Draw TEXT2 three times with three different foreground colors.
     * This checks that cairo uses the foreground color and does not cache
     * glyph images when the foreground color changes.
     */
    cairo_show_text (cr, TEXT2);
    cairo_set_source_rgb (cr, 0, 0.5, 0);
    cairo_show_text (cr, TEXT2);
    cairo_set_source_rgb (cr, 0.2, 0.5, 0.5);
    cairo_show_text (cr, TEXT2);

    /* filled version of text in light blue */
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, BORDER, BORDER + font_extents.height + BORDER + font_extents.ascent);
    cairo_text_path (cr, full_text);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (user_font_proxy,
	    "Tests a user-font using a native font in its render_glyph",
	    "font, user-font", /* keywords */
	    "cairo >= 1.7.4", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
