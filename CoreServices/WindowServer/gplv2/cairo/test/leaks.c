/*
 * Copyright © 2023 Uli Schlachter
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
 * Author: Uli Schlachter <psychon@znc.in>
 */

#include "cairo-test.h"


// Once upon a time, _cairo_gstate_fini(), _cairo_scaled_font_fini_internal(),
// and _cairo_ft_scaled_font_fini() leaked font options.
static cairo_test_status_t
leaks_set_scaled_font (cairo_t *cr, int width, int height)
{
    cairo_font_options_t *opt;
    cairo_matrix_t matrix;
    cairo_scaled_font_t *font;

    cairo_matrix_init_identity (&matrix);

    opt = cairo_font_options_create ();
    cairo_font_options_set_custom_palette_color (opt, 0, 1, 1, 1, 1);

    font = cairo_scaled_font_create (cairo_get_font_face (cr), &matrix, &matrix, opt);

    cairo_set_scaled_font (cr, font);

    cairo_font_options_destroy (opt);
    cairo_scaled_font_destroy (font);

    // Fill the output so that the same ref image works for everying
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (leaks_set_scaled_font,
	    "Regression test for font options memory leak in cairo_set_scaled_font",
	    "leak", /* keywords */
	    NULL, /* requirements */
	    1, 1,
	    NULL, leaks_set_scaled_font)
