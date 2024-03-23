/*
 * Copyright © 2005 Tim Rowley
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
 * Author: Tim Rowley
 */

#include "cairo-test.h"

#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 64


static void
draw_text_pattern (cairo_t *cr, double alpha)
{
    cairo_pattern_t *pat;

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    pat = cairo_pattern_create_linear (0.0, 0.0, 1, 1);
    cairo_pattern_add_color_stop_rgba (pat, 1, 1, 0, 0, alpha);
    cairo_pattern_add_color_stop_rgba (pat, 0, 0, 0, 1, alpha);
    cairo_set_source (cr, pat);

    /* test rectangle - make sure the gradient is set correctly */
    cairo_rectangle (cr, 0, 0, 0.1, 1);
    cairo_fill (cr);

    cairo_set_font_size (cr, 0.4);
    cairo_move_to (cr, 0.1, 0.6);
    cairo_show_text (cr, "cairo");

    cairo_pattern_destroy (pat);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_scale (cr, width/2, height);
    draw_text_pattern (cr, 1.0);
    cairo_translate (cr, 1, 0);
    draw_text_pattern (cr, 0.5);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (text_pattern,
	    "Patterned Text",
	    "text, pattern", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
