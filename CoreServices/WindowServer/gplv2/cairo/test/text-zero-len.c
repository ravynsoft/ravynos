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
 * Author: Behdad Esfahbod <behdad@behdad.org>
 */

/* Related bug 5177
 *
 * In short:
 *
 * _cairo_atsui_font_text_to_glyph with a zero-sized string crashes.
 *
 * Moreover, the fallback path in cairo_scaled_font_text_to_glyphs()
 * when handling a zero-sized string, allocates a zero-sized glyph array
 * and when NULL is returned by malloc, recognizes that as an out-of-memory
 * error.  The glibc implementation of malloc() does not return NULL from
 * malloc(0), but I don't think it's a safe assumption.
 *
 * By just bailing out on zero-sized text, we fix both issues.
 */

#include "cairo-test.h"

#define NUM_TEXT 20
#define TEXT_SIZE 12

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

static cairo_bool_t
font_extents_equal (const cairo_font_extents_t *A,
	            const cairo_font_extents_t *B)
{
    return A->ascent    == B->ascent        &&
	   A->descent   == B->descent       &&
	   A->height    == B->height        &&
       A->max_x_advance == B->max_x_advance &&
       A->max_y_advance == B->max_y_advance;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_text_extents_t extents, nil_extents;
    cairo_font_extents_t font_extents, nil_font_extents;
    cairo_scaled_font_t *scaled_font;

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 16);

    cairo_move_to (cr, 10, 25);
    cairo_show_text (cr, NULL);
    cairo_show_text (cr, "");
    cairo_show_glyphs (cr, NULL, 0);
    cairo_show_glyphs (cr, (void*)8, 0);

    cairo_move_to (cr, 10, 55);
    cairo_text_path (cr, NULL);
    cairo_text_path (cr, "");
    cairo_glyph_path (cr, (void*)8, 0);
    cairo_fill (cr);

    memset (&nil_extents, 0, sizeof (cairo_text_extents_t));

    memset (&extents, 0xff, sizeof (cairo_text_extents_t));
    cairo_text_extents (cr, "", &extents);
    if (! text_extents_equal (&extents, &nil_extents)) {
	cairo_test_log (ctx, "Error: cairo_text_extents(\"\"); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance);
	return CAIRO_TEST_FAILURE;
    }

    memset (&extents, 0xff, sizeof (cairo_text_extents_t));
    cairo_text_extents (cr, NULL, &extents);
    if (! text_extents_equal (&extents, &nil_extents)) {
	cairo_test_log (ctx, "Error: cairo_text_extents(NULL); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance);
	return CAIRO_TEST_FAILURE;
    }

    memset (&extents, 0xff, sizeof (cairo_text_extents_t));
    cairo_glyph_extents (cr, (void*)8, 0, &extents);
    if (! text_extents_equal (&extents, &nil_extents)) {
	cairo_test_log (ctx, "Error: cairo_glyph_extents(); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance);
	return CAIRO_TEST_FAILURE;
    }

    scaled_font = cairo_get_scaled_font (cr);

    memset (&extents, 0xff, sizeof (cairo_text_extents_t));
    cairo_scaled_font_text_extents (scaled_font, "", &extents);
    if (! text_extents_equal (&extents, &nil_extents)) {
	cairo_test_log (ctx, "Error: cairo_scaled_font_text_extents(\"\"); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance);
	return CAIRO_TEST_FAILURE;
    }

    memset (&extents, 0xff, sizeof (cairo_text_extents_t));
    cairo_scaled_font_text_extents (scaled_font, NULL, &extents);
    if (! text_extents_equal (&extents, &nil_extents)) {
	cairo_test_log (ctx, "Error: cairo_scaled_font_text_extents(NULL); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance);
	return CAIRO_TEST_FAILURE;
    }

    memset (&extents, 0xff, sizeof (cairo_text_extents_t));
    cairo_scaled_font_glyph_extents (scaled_font, (void*)8, 0, &extents);
    if (! text_extents_equal (&extents, &nil_extents)) {
	cairo_test_log (ctx, "Error: cairo_scaled_font_glyph_extents(NULL); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance);
	return CAIRO_TEST_FAILURE;
    }

    /* Lets also try font size 0 while here */
    cairo_set_font_size (cr, 0);

    memset (&extents, 0xff, sizeof (cairo_text_extents_t));
    cairo_text_extents (cr, "test", &extents);
    if (! text_extents_equal (&extents, &nil_extents)) {
	cairo_test_log (ctx, "Error: cairo_set_font_size(0); cairo_text_extents(\"test\"); extents (%g, %g, %g, %g, %g, %g)\n",
		        extents.x_bearing, extents.y_bearing,
			extents.width, extents.height,
			extents.x_advance, extents.y_advance);
	return CAIRO_TEST_FAILURE;
    }

    memset (&nil_font_extents, 0, sizeof (cairo_font_extents_t));

    memset (&font_extents, 0xff, sizeof (cairo_font_extents_t));
    cairo_font_extents (cr,  &font_extents);
    if (! font_extents_equal (&font_extents, &nil_font_extents)) {
	cairo_test_log (ctx, "Error: cairo_set_font_size(0); cairo_font_extents(); extents (%g, %g, %g, %g, %g)\n",
		        font_extents.ascent, font_extents.descent,
			font_extents.height,
			font_extents.max_x_advance, font_extents.max_y_advance);
	return CAIRO_TEST_FAILURE;
    }

    scaled_font = cairo_get_scaled_font (cr);

    memset (&font_extents, 0xff, sizeof (cairo_font_extents_t));
    cairo_scaled_font_extents (scaled_font,  &font_extents);
    if (! font_extents_equal (&font_extents, &nil_font_extents)) {
	cairo_test_log (ctx, "Error: cairo_set_font_size(0); cairo_scaled_font_extents(); extents (%g, %g, %g, %g, %g)\n",
		        font_extents.ascent, font_extents.descent,
			font_extents.height,
			font_extents.max_x_advance, font_extents.max_y_advance);
	return CAIRO_TEST_FAILURE;
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (text_zero_len,
	    "Tests show_text and text_path with a zero-sized string",
	    "text, stress, extents", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    NULL, draw)
