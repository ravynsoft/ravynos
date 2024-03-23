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
#define WIDTH  (TEXT_SIZE * 16 + 2*BORDER)
#ifndef ROTATED
 #define HEIGHT ((TEXT_SIZE + 2*BORDER)*3)
#else
 #define HEIGHT WIDTH
#endif

#define TEXT1   "cairo user-font."
#define TEXT2   " zg"

#define END_GLYPH 0
#define STROKE 126
#define CLOSE 127

/* Simple glyph definition: 1 - 15 means lineto (or moveto for first
 * point) for one of the points on this grid:
 *
 *      1  2  3
 *      4  5  6
 *      7  8  9
 * ----10 11 12----(baseline)
 *     13 14 15
 */
typedef struct {
    unsigned long ucs4;
    int width;
    char data[16];
} test_scaled_font_glyph_t;

static cairo_user_data_key_t test_font_face_glyphs_key;

static cairo_status_t
test_scaled_font_init (cairo_scaled_font_t  *scaled_font,
		       cairo_t              *cr,
		       cairo_font_extents_t *metrics)
{
  metrics->ascent  = .75;
  metrics->descent = .25;
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
    const char *data;
    div_t d;
    double x, y;

    /* FIXME: We simply crash on out-of-bound glyph indices */

    metrics->x_advance = glyphs[glyph].width / 4.0;

    cairo_set_line_width (cr, 0.1);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

    data = glyphs[glyph].data;
    for (i = 0; data[i] != END_GLYPH; i++) {
	switch (data[i]) {
	case STROKE:
	    cairo_new_sub_path (cr);
	    break;

	case CLOSE:
	    cairo_close_path (cr);
	    break;

	default:
	    d = div (data[i] - 1, 3);
	    x = d.rem / 4.0 + 0.125;
	    y = d.quot / 5.0 + 0.4 - 1.0;
	    cairo_line_to (cr, x, y);
	}
    }
    cairo_stroke (cr);

    return CAIRO_STATUS_SUCCESS;
}


/* If color_render is TRUE, use the render_color_glyph callback
 * instead of the render_glyph callbac. The output should be identical
 * in this test since the render function does not alter the cairo_t
 * source.
 */
static cairo_status_t
_user_font_face_create (cairo_font_face_t **out, cairo_bool_t color_render)
{
    /* Simple glyph definition: 1 - 15 means lineto (or moveto for first
     * point) for one of the points on this grid:
     *
     *      1  2  3
     *      4  5  6
     *      7  8  9
     * ----10 11 12----(baseline)
     *     13 14 15
     */
    static const test_scaled_font_glyph_t glyphs [] = {
	{ 'a',  3, { 4, 6, 12, 10, 7, 9, STROKE, END_GLYPH } },
	{ 'c',  3, { 6, 4, 10, 12, STROKE, END_GLYPH } },
	{ 'e',  3, { 12, 10, 4, 6, 9, 7, STROKE, END_GLYPH } },
	{ 'f',  3, { 3, 2, 11, STROKE, 4, 6, STROKE, END_GLYPH } },
	{ 'g',  3, { 12, 10, 4, 6, 15, 13, STROKE, END_GLYPH } },
	{ 'h',  3, { 1, 10, STROKE, 7, 5, 6, 12, STROKE, END_GLYPH } },
	{ 'i',  1, { 1, 1, STROKE, 4, 10, STROKE, END_GLYPH } },
	{ 'l',  1, { 1, 10, STROKE, END_GLYPH } },
	{ 'n',  3, { 10, 4, STROKE, 7, 5, 6, 12, STROKE, END_GLYPH } },
	{ 'o',  3, { 4, 10, 12, 6, CLOSE, END_GLYPH } },
	{ 'r',  3, { 4, 10, STROKE, 7, 5, 6, STROKE, END_GLYPH } },
	{ 's',  3, { 6, 4, 7, 9, 12, 10, STROKE, END_GLYPH } },
	{ 't',  3, { 2, 11, 12, STROKE, 4, 6, STROKE, END_GLYPH } },
	{ 'u',  3, { 4, 10, 12, 6, STROKE, END_GLYPH } },
	{ 'z',  3, { 4, 6, 10, 12, STROKE, END_GLYPH } },
	{ ' ',  1, { END_GLYPH } },
	{ '-',  2, { 7, 8, STROKE, END_GLYPH } },
	{ '.',  1, { 10, 10, STROKE, END_GLYPH } },
	{  -1,  0, { END_GLYPH } },
    };

    cairo_font_face_t *user_font_face;
    cairo_status_t status;

    user_font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_init_func (user_font_face, test_scaled_font_init);
    if (color_render)
        cairo_user_font_face_set_render_color_glyph_func (user_font_face, test_scaled_font_render_glyph);
    else
        cairo_user_font_face_set_render_glyph_func (user_font_face, test_scaled_font_render_glyph);

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

static void
draw_line (cairo_t *cr)
{
    /* TEXT1 in black */
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
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_font_face_t *font_face;
    char full_text[100];
    cairo_font_extents_t font_extents;
    cairo_text_extents_t extents;
    cairo_status_t status;

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

    status = _user_font_face_create (&font_face, FALSE);
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

    /* First line. TEXT1 in black. TEXT2 in different colors. */
    cairo_move_to (cr, BORDER, BORDER + font_extents.ascent);
    draw_line (cr);

    /* Now draw the second line using the render_color_glyph
     * callback. The text should be all black because the default
     * color of render function is used instead of the foreground
     * color.
     */
    status = _user_font_face_create (&font_face, TRUE);
    if (status) {
	return cairo_test_status_from_status (cairo_test_get_context (cr),
					      status);
    }

    cairo_set_font_face (cr, font_face);
    cairo_font_face_destroy (font_face);

    cairo_set_font_size (cr, TEXT_SIZE);

    cairo_move_to (cr, BORDER, BORDER + font_extents.height + 2*BORDER + font_extents.ascent);
    draw_line (cr);

    /* Third line. Filled version of text in blue */
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, BORDER, BORDER + font_extents.height + 4*BORDER + 2*font_extents.ascent);
    cairo_text_path (cr, full_text);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (user_font,
	    "Tests user font feature",
	    "font, user-font", /* keywords */
	    "cairo >= 1.7.4", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
