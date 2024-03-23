/*
 * Copyright © 2021 Adrian Johnson
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
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"
#include <cairo-ft.h>

#define SIZE 40
#define HEIGHT SIZE
#define WIDTH  (SIZE * 1.5)
#define FONT "Noto Color Emoji"

static const char smiley_face_utf8[] = { 0xf0, 0x9f, 0x99, 0x82, 0x00 }; /* U+1F642 */

static cairo_test_status_t
set_color_emoji_font (cairo_t *cr)
{
    cairo_font_options_t *font_options;
    cairo_font_face_t *font_face;
    FcPattern *pattern;
    FcPattern *resolved;
    FcChar8 *font_name;
    FcResult result;

    pattern = FcPatternCreate ();
    if (pattern == NULL)
	return CAIRO_TEST_NO_MEMORY;

    FcPatternAddString (pattern, FC_FAMILY, (FcChar8 *) FONT);
    FcConfigSubstitute (NULL, pattern, FcMatchPattern);

    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    cairo_ft_font_options_substitute (font_options, pattern);

    FcDefaultSubstitute (pattern);
    resolved = FcFontMatch (NULL, pattern, &result);
    if (resolved == NULL) {
	FcPatternDestroy (pattern);
	return CAIRO_TEST_NO_MEMORY;
    }

    if (FcPatternGetString (resolved, FC_FAMILY, 0, &font_name) == FcResultMatch) {
        if (strcmp((char*)font_name, FONT) != 0) {
            const cairo_test_context_t *ctx = cairo_test_get_context (cr);
            cairo_test_log (ctx, "Could not find %s font\n", FONT);
            return CAIRO_TEST_UNTESTED;
        }
    } else {
        return CAIRO_TEST_FAILURE;
    }

    font_face = cairo_ft_font_face_create_for_pattern (resolved);
    cairo_set_font_face (cr, font_face);

    cairo_font_options_destroy (font_options);
    cairo_font_face_destroy (font_face);
    FcPatternDestroy (pattern);
    FcPatternDestroy (resolved);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_test_status_t result;
    cairo_font_options_t *font_options;

    result = set_color_emoji_font (cr);
    if (result != CAIRO_TEST_SUCCESS)
        return result;

    cairo_set_font_size (cr, SIZE/2);
    cairo_move_to (cr, SIZE/8, 0.7 * SIZE);

    cairo_show_text(cr, smiley_face_utf8);

    /* Show that the color mode font option can disable color rendering */
    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    cairo_font_options_set_color_mode (font_options, CAIRO_COLOR_MODE_NO_COLOR);
    cairo_set_font_options (cr, font_options);
    cairo_font_options_destroy (font_options);

    cairo_show_text(cr, smiley_face_utf8);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (ft_color_font,
	    "Test color font",
	    "ft, font", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
