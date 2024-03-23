/*
 * Copyright © 2023 Adrian Johnson
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

#define FONT_SIZE 50
#define MARGIN 5
#define WIDTH  (FONT_SIZE*8 + MARGIN*9)
#define HEIGHT (FONT_SIZE*2 + MARGIN*3)

#define FONT_FILE "cairo-svg-test-color.ttf"

#define PALETTE_TEXT     "01"
#define FOREGROUND_TEXT  "234567"


static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_test_status_t result;
    cairo_font_options_t *font_options;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    result = cairo_test_ft_select_font_from_file (cr, FONT_FILE);
    if (result)
        return result;

    cairo_set_font_size (cr, FONT_SIZE);
    
    cairo_save (cr);
    cairo_move_to (cr, MARGIN, FONT_SIZE + MARGIN);
    font_options = cairo_font_options_create ();

    /* Default palette */
    cairo_show_text (cr, PALETTE_TEXT);

    /* Palette 1 */
    cairo_font_options_set_color_palette (font_options, 1);
    cairo_set_font_options (cr, font_options);
    cairo_show_text (cr, PALETTE_TEXT);

    /* Palette 0, override color 0 */
    cairo_font_options_set_color_palette (font_options, 0);
    cairo_font_options_set_custom_palette_color (font_options, 0, 1, 0, 1, 0.5);
    cairo_set_font_options (cr, font_options);
    cairo_show_text (cr, PALETTE_TEXT);

    /* Palette 1, override color 1 */
    cairo_font_options_set_color_palette (font_options, 1);
    cairo_font_options_set_custom_palette_color (font_options, 1, 0, 1, 1, 0.5);
    cairo_set_font_options (cr, font_options);
    cairo_show_text (cr, PALETTE_TEXT);

    cairo_font_options_destroy (font_options);
    cairo_restore (cr);

    cairo_move_to (cr, MARGIN, FONT_SIZE*2 + MARGIN*2);

    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_show_text (cr, FOREGROUND_TEXT);

    
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (ft_svg_render_color,
	    "Test cairo SVG font colors",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
