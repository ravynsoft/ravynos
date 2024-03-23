/*
 * Copyright © 2021 Adrian Johnson
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
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

/* Test pdf-operators text positioning. This test is designed to expose rounding
 * errors in the PDF operations for relative positioning in very long strings.
 *
 * The text width is expected to match the width of the rectangle
 * enclosing the text.
 */

#include "cairo-test.h"

#define WIDTH  10000
#define HEIGHT 60

/* Using a non integer size helps expose rounding errors */
#define FONT_SIZE 10.12345678912345

#define WORD "Text"
#define NUM_WORDS 450

#define BORDER 10

static cairo_user_data_key_t font_face_key;

static cairo_status_t
user_font_init (cairo_scaled_font_t  *scaled_font,
                cairo_t              *cr,
                cairo_font_extents_t *metrics)
{
    cairo_font_face_t *font_face = cairo_font_face_get_user_data (cairo_scaled_font_get_font_face (scaled_font),
                                                                  &font_face_key);
    cairo_set_font_face (cr, font_face);
    cairo_font_extents (cr, metrics);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
user_font_render_glyph (cairo_scaled_font_t  *scaled_font,
                        unsigned long         index,
                        cairo_t              *cr,
                        cairo_text_extents_t *metrics)
{
    char text[2];
    cairo_font_face_t *font_face = cairo_font_face_get_user_data (cairo_scaled_font_get_font_face (scaled_font),
                                                                  &font_face_key);

    text[0] = index; /* Only using ASCII for this test */
    text[1] = 0;
    cairo_set_font_face (cr, font_face);
    cairo_text_extents (cr, text, metrics);
    cairo_text_path (cr, text);
    cairo_fill (cr);
    return CAIRO_STATUS_SUCCESS;
}

static cairo_font_face_t *
create_user_font_face (cairo_font_face_t *orig_font)
{
    cairo_font_face_t *user_font_face;

    user_font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_init_func (user_font_face, user_font_init);
    cairo_user_font_face_set_render_glyph_func (user_font_face, user_font_render_glyph);
    cairo_font_face_set_user_data (user_font_face, &font_face_key, (void*) orig_font, NULL);
    return user_font_face;
}

static void
draw_text (cairo_t *cr, const char *text)
{
    cairo_text_extents_t extents;

    cairo_move_to (cr, BORDER, BORDER);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_show_text (cr, text);
    cairo_text_extents (cr, text,&extents);

    cairo_rectangle (cr,
                     BORDER + extents.x_bearing,
                     BORDER + extents.y_bearing,
                     extents.width,
                     extents.height);
    cairo_set_line_width (cr, 1);
    cairo_stroke (cr);
}


static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;
    char *text;
    cairo_font_face_t *font_face;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_select_font_face (cr, "Dejavu Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size (cr, FONT_SIZE);

    text = malloc (strlen(WORD) * NUM_WORDS + 1);
    text[0] = '\0';
    for (i = 0; i < NUM_WORDS; i++)
	strcat (text, WORD);

    cairo_save (cr);
    cairo_translate (cr, BORDER, BORDER);
    draw_text (cr, text);
    cairo_restore (cr);

    font_face = create_user_font_face (cairo_get_font_face (cr));
    cairo_set_font_face (cr, font_face);
    cairo_font_face_destroy (font_face);
    cairo_set_font_size (cr, FONT_SIZE);

    cairo_save (cr);
    cairo_translate (cr, BORDER, BORDER*3);
    draw_text (cr, text);
    cairo_restore (cr);

    free (text);
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (pdf_operators_text,
	    "Test pdf-operators.c glyph positioning",
	    "pdf", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
