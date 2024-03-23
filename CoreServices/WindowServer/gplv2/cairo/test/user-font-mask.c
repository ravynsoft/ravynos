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
 *	Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"

#include <stdlib.h>
#include <stdio.h>

/*#define ROTATED 1*/

#define BORDER 10
#define TEXT_SIZE 64
#define WIDTH  (TEXT_SIZE * 15 + 2*BORDER)
#ifndef ROTATED
 #define HEIGHT ((TEXT_SIZE + 2*BORDER)*2)
#else
 #define HEIGHT WIDTH
#endif
#define END_GLYPH 0
#define TEXT   "cairo"

/* Reverse the bits in a byte with 7 operations (no 64-bit):
 * Devised by Sean Anderson, July 13, 2001.
 * Source: http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
 */
#define CAIRO_BITSWAP8(c) ((((c) * 0x0802LU & 0x22110LU) | ((c) * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16)

#ifdef WORDS_BIGENDIAN
#define CAIRO_BITSWAP8_IF_LITTLE_ENDIAN(c) (c)
#else
#define CAIRO_BITSWAP8_IF_LITTLE_ENDIAN(c) CAIRO_BITSWAP8(c)
#endif



/* Simple glyph definition. data is an 8x8 bitmap.
 */
typedef struct {
    unsigned long ucs4;
    int width;
    char data[8];
} test_scaled_font_glyph_t;

static cairo_user_data_key_t test_font_face_glyphs_key;

static cairo_status_t
test_scaled_font_init (cairo_scaled_font_t  *scaled_font,
		       cairo_t              *cr,
		       cairo_font_extents_t *metrics)
{
  metrics->ascent  = 1;
  metrics->descent = 0;
  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
test_scaled_font_unicode_to_glyph (cairo_scaled_font_t *scaled_font,
				   unsigned long        unicode,
				   unsigned long       *glyph)
{
    test_scaled_font_glyph_t *glyphs = cairo_font_face_get_user_data (cairo_scaled_font_get_font_face (scaled_font),
								      &test_font_face_glyphs_key);
    int i;

    for (i = 0; glyphs[i].ucs4 != (unsigned long) -1; i++)
	if (glyphs[i].ucs4 == unicode) {
	    *glyph = i;
	    return CAIRO_STATUS_SUCCESS;
	}

    /* Not found.  Default to glyph 0 */
    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
test_scaled_font_render_glyph (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *metrics)
{
    test_scaled_font_glyph_t *glyphs = cairo_font_face_get_user_data (cairo_scaled_font_get_font_face (scaled_font),
								      &test_font_face_glyphs_key);
    int i;
    unsigned char *data;
    cairo_surface_t *image;
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;
    uint8_t byte;

    /* FIXME: We simply crash on out-of-bound glyph indices */

    metrics->x_advance = (glyphs[glyph].width + 1) / 8.0;

    image = cairo_image_surface_create (CAIRO_FORMAT_A1, glyphs[glyph].width, 8);
    if (cairo_surface_status (image))
	return cairo_surface_status (image);

    data = cairo_image_surface_get_data (image);
    for (i = 0; i < 8; i++) {
	byte = glyphs[glyph].data[i];
	*data = CAIRO_BITSWAP8_IF_LITTLE_ENDIAN (byte);
	data += cairo_image_surface_get_stride (image);
    }
    cairo_surface_mark_dirty (image);

    pattern = cairo_pattern_create_for_surface (image);
    cairo_surface_destroy (image);

    cairo_matrix_init_identity (&matrix);
    cairo_matrix_scale (&matrix, 1.0/8.0, 1.0/8.0);
    cairo_matrix_translate (&matrix, 0, -8);
    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);

    cairo_set_source (cr, pattern);
    cairo_mask (cr, pattern);
    cairo_pattern_destroy (pattern);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_user_font_face_create (cairo_font_face_t **out)
{
    static const test_scaled_font_glyph_t glyphs [] = {
	{ 'c',  6, { 0x00, 0x38, 0x44, 0x80, 0x80, 0x80, 0x44, 0x38 } },
	{ 'a',  6, { 0x00, 0x70, 0x88, 0x3c, 0x44, 0x84, 0x8c, 0x74 } },
	{ 'i',  1, { 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 } },
	{ 'r',  6, { 0x00, 0xb8, 0xc4, 0x80, 0x80, 0x80, 0x80, 0x80 } },
	{ 'o',  7, { 0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x38 } },
	{  -1,  8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    };

    cairo_font_face_t *user_font_face;
    cairo_status_t status;

    user_font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_init_func             (user_font_face, test_scaled_font_init);
    cairo_user_font_face_set_render_glyph_func     (user_font_face, test_scaled_font_render_glyph);
    cairo_user_font_face_set_unicode_to_glyph_func (user_font_face, test_scaled_font_unicode_to_glyph);

    status = cairo_font_face_set_user_data (user_font_face,
					    &test_font_face_glyphs_key,
					    (void*) glyphs, NULL);
    if (status) {
	cairo_font_face_destroy (user_font_face);
	return status;
    }

    *out = user_font_face;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_font_face_t *font_face;
    const char text[] = TEXT;
    cairo_font_extents_t font_extents;
    cairo_text_extents_t extents;
    cairo_status_t status;

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
    cairo_text_extents (cr, text, &extents);

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

    /* text in black */
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_move_to (cr, BORDER, BORDER + font_extents.ascent);
    cairo_show_text (cr, text);


    /* filled version of text in blue */
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, BORDER, BORDER + font_extents.height + 2*BORDER + font_extents.ascent);
    cairo_text_path (cr, text);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (user_font_mask,
	    "Tests a user-font using cairo_mask with bitmap images",
	    "user-font, mask", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
