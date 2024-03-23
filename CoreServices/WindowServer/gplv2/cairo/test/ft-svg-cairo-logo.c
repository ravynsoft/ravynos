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

#define FONT_SIZE 200
#define MARGIN 5
#define WIDTH  (FONT_SIZE + MARGIN*2)
#define HEIGHT (FONT_SIZE + MARGIN*2)

#define FONT_FILE "cairo-logo-font.ttf"

/* Character code in font of the logo */
#define CAIRO_LOGO_CHAR "A"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_test_status_t result;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    result = cairo_test_ft_select_font_from_file (cr, FONT_FILE);
    if (result)
        return result;

    cairo_set_font_size (cr, FONT_SIZE);
    cairo_move_to (cr, MARGIN, FONT_SIZE + MARGIN);

    cairo_show_text (cr, CAIRO_LOGO_CHAR);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (ft_svg_cairo_logo,
	    "Test cairo logo SVG font",
	    "svgrender", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
