/*
 * Copyright © 2022 Adrian Johnson
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

#define GLYPH_SIZE 50
#define PAD 5
#define WIDTH  (4*(GLYPH_SIZE + PAD) + PAD)
#define HEIGHT WIDTH

//#define CLIP 1
#define LOG_EXTENTS 1

static cairo_test_status_t
draw_font (cairo_t *cr, int width, int height, const char *font_file)
{
    cairo_test_status_t result;
    char buf[10];
    cairo_text_extents_t extents;
    cairo_font_options_t *font_options;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    result = cairo_test_ft_select_font_from_file (cr, font_file);
    if (result)
        return result;

    font_options = cairo_font_options_create ();
    cairo_font_options_set_color_mode (font_options, CAIRO_COLOR_MODE_NO_COLOR);
//    cairo_set_font_options (cr, font_options);
    cairo_font_options_destroy (font_options);

    cairo_set_font_size (cr, GLYPH_SIZE);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int x = j * (GLYPH_SIZE + PAD) + PAD;
            int y = i * (GLYPH_SIZE + PAD) + PAD;
            int glyph_number = 4*i + j;
            buf[0] = glyph_number < 10 ? '0' + glyph_number : 'A' + glyph_number - 10;
            buf[1] = 0;
            cairo_save (cr);
            cairo_text_extents (cr, buf, &extents);
#if LOG_EXTENTS
            cairo_test_log (cairo_test_get_context (cr),
                            "Char '%c' extents: x_bearing: %f  y_bearing: %f  width: %f  height: %f  x_advance: %f  y_advance: %f\n",
                            buf[0],
                            extents.x_bearing,
                            extents.y_bearing,
                            extents.width,
                            extents.height,
                            extents.x_advance,
                            extents.y_advance);
#endif
#if CLIP
            cairo_rectangle (cr, x, y, GLYPH_SIZE, GLYPH_SIZE);
            cairo_clip (cr);
#endif
            cairo_move_to (cr, x, y + GLYPH_SIZE);
            cairo_show_text (cr, buf);
            cairo_restore (cr);
            if (cairo_status (cr)) {
                cairo_test_log (cairo_test_get_context (cr),
                                "cairo_show_text() failed with \"%s\"\n",
                                buf);
                return CAIRO_TEST_FAILURE;
            }
        }
    }

    return CAIRO_TEST_SUCCESS;
}

#define DRAW_FUNC(name) \
static cairo_test_status_t \
draw_##name (cairo_t *cr, int width, int height) { \
    return draw_font (cr, width, height, "cairo-svg-test-" #name ".ttf"); \
}

DRAW_FUNC(doc)
CAIRO_TEST (ft_svg_render_doc,
	    "Test SVG glyph render",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_doc)

DRAW_FUNC(fill)
CAIRO_TEST (ft_svg_render_fill,
	    "Test SVG glyph render",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_fill)
    
DRAW_FUNC(gradient)
CAIRO_TEST (ft_svg_render_gradient,
	    "Test SVG glyph render",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_gradient)
    
DRAW_FUNC(path)
CAIRO_TEST (ft_svg_render_path,
	    "Test SVG glyph render",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_path)
    
DRAW_FUNC(shapes)
CAIRO_TEST (ft_svg_render_shapes,
	    "Test SVG glyph render",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_shapes)

DRAW_FUNC(stroke)
CAIRO_TEST (ft_svg_render_stroke,
	    "Test SVG glyph render",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_stroke)

DRAW_FUNC(transform)
CAIRO_TEST (ft_svg_render_transform,
	    "Test SVG glyph render",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_transform)
