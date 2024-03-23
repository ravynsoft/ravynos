/*
 * Copyright © 2006, 2008 Red Hat, Inc.
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
 * Contributor(s):
 *	Kristian Høgsberg <krh@redhat.com>
 *	Behdad Esfahbod <behdad@behdad.org>
 */

/* Test that user-fonts can handle color and non-color glyphs in the
 * same font.
 */

#include "cairo-test.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>


#define BORDER 10
#define TEXT_SIZE 64
#define WIDTH  (TEXT_SIZE * 11 + 2*BORDER)
#define HEIGHT (4*TEXT_SIZE + 5*BORDER)

#define TEXT          "abcdefghij"

/* These characters will be drawn twice with a different foreground color */
#define FG_TEXT       "acfh"

/* Uppercase draws the same text but forces the use of the non-color
 * render callback */
#define TEXT_NO_COLOR    "ABCDEFGHIJ"
#define FG_TEXT_NO_COLOR "ACFH"

#define TEXT_PATH       "aabccdeffghhij"


static cairo_status_t
test_scaled_font_init (cairo_scaled_font_t  *scaled_font,
		       cairo_t              *cr,
		       cairo_font_extents_t *metrics)
{
    metrics->ascent  = .75;
    metrics->descent = .25;
    return CAIRO_STATUS_SUCCESS;
}

static void
render_glyph_solid (cairo_t *cr,
                    double width,
                    double height,
                    cairo_bool_t color,
                    cairo_scaled_font_t *scaled_font)
{
    if (color)
        cairo_set_source_rgba (cr, 0.7, 0.2, 0.1, 0.9);
    cairo_rectangle (cr, 0, 0, width/2, height/2);
    cairo_fill (cr);

    if (color) {
        if (scaled_font)
            cairo_set_source (cr, cairo_user_scaled_font_get_foreground_marker (scaled_font));
        else
            cairo_set_source_rgba (cr, 0.2, 0.5, 0.3, 0.9);
    }
    cairo_rectangle (cr, width/4, height/4, width/2, height/2);
    cairo_fill (cr);

    if (color)
        cairo_set_source_rgba (cr, 0.2, 0.3, 0.5, 0.9);
    cairo_rectangle (cr, width/2, height/2, width/2, height/2);
    cairo_fill (cr);
}

static void
render_glyph_linear (cairo_t *cr,
                     double width,
                     double height,
                     cairo_bool_t color,
                     cairo_scaled_font_t *scaled_font)
{
    cairo_pattern_t *pat;
    cairo_pattern_t *fg;

    pat = cairo_pattern_create_linear (0.0, 0.0, width, height);
    if (scaled_font) {
        double r, g, b, a;

        fg = cairo_user_scaled_font_get_foreground_source (scaled_font);
        if (cairo_pattern_get_rgba (fg, &r, &g, &b, &a) != CAIRO_STATUS_SUCCESS) {
            r = g = b = 0;
            a = 1;
        }
        cairo_pattern_add_color_stop_rgba (pat, 0,  r, g, b, a);
        cairo_pattern_add_color_stop_rgb  (pat, 1,  0, 0, 1);
    } else {
        cairo_pattern_add_color_stop_rgb (pat, 0,   1, 0.4, 0.2);
        cairo_pattern_add_color_stop_rgb (pat, 0.5, 0.2, 1, 0.4);
        cairo_pattern_add_color_stop_rgb (pat, 1,   0.2, 0.3, 1);
    }

    cairo_set_source (cr, pat);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);
}

static void
render_glyph_text (cairo_t *cr, double width, double height, cairo_bool_t color)
{
    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 0.5);

    if (color)
        cairo_set_source_rgb (cr, 0.5, 0.7, 0);
    cairo_move_to (cr, width*0.1, height/2);
    cairo_show_text (cr, "a");

    if (color)
        cairo_set_source_rgb (cr, 0, 0.5, 0.7);
    cairo_move_to (cr, width*0.4, height*0.9);
    cairo_show_text (cr, "z");
}

static cairo_status_t
test_scaled_font_render_glyph_common (cairo_scaled_font_t  *scaled_font,
                                      unsigned long         glyph,
                                      cairo_t              *cr,
                                      cairo_text_extents_t *metrics,
                                      cairo_bool_t          color)
{
    double width = 0.5;
    double height = 0.8;

    metrics->x_advance = 0.75;
    cairo_translate (cr,  0.125, -0.6);
    switch (glyph) {
        case 'a':
            render_glyph_solid (cr, width, height, color, scaled_font);
            break;
        case 'b':
            render_glyph_solid (cr, width, height, color, NULL);
            break;
        case 'c':
            render_glyph_linear (cr, width, height, color, scaled_font);
            break;
        case 'd':
            render_glyph_linear (cr, width, height, color, NULL);
            break;
        case 'e':
            render_glyph_text (cr, width, height, color);
            break;
        case 'f':
            cairo_push_group (cr);
            render_glyph_solid (cr, width, height, color, scaled_font);
            cairo_pop_group_to_source (cr);
            cairo_paint (cr);
            break;
        case 'g':
            cairo_push_group (cr);
            render_glyph_solid (cr, width, height, color, NULL);
            cairo_pop_group_to_source (cr);
            cairo_paint (cr);
            break;
        case 'h':
            cairo_push_group (cr);
            render_glyph_linear (cr, width, height, color, scaled_font);
            cairo_pop_group_to_source (cr);
            cairo_paint (cr);
            break;
        case 'i':
            cairo_push_group (cr);
            render_glyph_linear (cr, width, height, color, NULL);
            cairo_pop_group_to_source (cr);
            cairo_paint (cr);
            break;
        case 'j':
            cairo_push_group (cr);
            render_glyph_text (cr, width, height, color);
            cairo_pop_group_to_source (cr);
            cairo_paint (cr);
            break;
    }

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
test_scaled_font_render_color_glyph_callback (cairo_scaled_font_t  *scaled_font,
                                     unsigned long         glyph,
                                     cairo_t              *cr,
                                     cairo_text_extents_t *metrics)
{
    if (isupper(glyph))
        return CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED;

    return test_scaled_font_render_glyph_common (scaled_font, glyph, cr, metrics, TRUE);
}

static cairo_status_t
test_scaled_font_render_glyph_callback (cairo_scaled_font_t  *scaled_font,
                                        unsigned long         glyph,
                                        cairo_t              *cr,
                                        cairo_text_extents_t *metrics)
{
    int c = glyph;
    if (isupper(c))
        c = tolower(c);

    return test_scaled_font_render_glyph_common (scaled_font, c, cr, metrics, FALSE);
}

static cairo_status_t
_user_font_face_create (cairo_font_face_t **out)
{

    cairo_font_face_t *user_font_face;

    user_font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_init_func (user_font_face, test_scaled_font_init);
    cairo_user_font_face_set_render_color_glyph_func (user_font_face,
                                                      test_scaled_font_render_color_glyph_callback);
    cairo_user_font_face_set_render_glyph_func (user_font_face,
                                                test_scaled_font_render_glyph_callback);

    *out = user_font_face;
    return CAIRO_STATUS_SUCCESS;
}

/* Any text characters that are in fg_text will be drawn with a different color */
static void
draw_line (cairo_t *cr, const char *text, const char *fg_text)
{
    char buf[10];

    for (unsigned i = 0; i < strlen(text); i++) {
        buf[0] = text[i];
        buf[1] = 0;

        if (strchr (fg_text, text[i])) {
            cairo_set_source_rgb (cr, 1, 0, 0);
            cairo_show_text (cr, buf);
        }

        cairo_set_source_rgb (cr, 0, 1, 0);
        cairo_show_text (cr, buf);
    }
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_font_face_t *font_face;
    const char text[] = TEXT;
    cairo_font_extents_t font_extents;
    cairo_text_extents_t extents;
    cairo_status_t status;
    cairo_font_options_t *font_options;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

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

    /* Line 1: text in color */
    cairo_move_to (cr, BORDER, BORDER + font_extents.ascent);
    draw_line (cr, TEXT, FG_TEXT);

    /* Line 2: text in non-color (color render callback returns
     * CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED.
     */
    cairo_move_to (cr, BORDER, BORDER + font_extents.height + 1*BORDER + font_extents.ascent);
    draw_line (cr, TEXT_NO_COLOR, FG_TEXT_NO_COLOR);

    /* Line 3: Filled version of color text in blue */
    cairo_move_to (cr, BORDER, BORDER + 2*font_extents.height + 2*BORDER + font_extents.ascent);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_text_path (cr, TEXT_PATH);
    cairo_fill (cr);

    /* Line 4: color glyphs with CAIRO_COLOR_MODE_NO_COLOR font option. */
    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    cairo_font_options_set_color_mode (font_options, CAIRO_COLOR_MODE_NO_COLOR);
    cairo_set_font_options (cr, font_options);
    cairo_font_options_destroy (font_options);

    cairo_move_to (cr, BORDER, BORDER + 3*font_extents.height + 3*BORDER + font_extents.ascent);
    draw_line (cr, TEXT, FG_TEXT);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (user_font_color,
	    "Tests user font color feature",
	    "font, user-font", /* keywords */
	    "cairo >= 1.17.4", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
