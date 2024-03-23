/*
 * Copyright © 2023 Marc Jeanmougin
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
 * Author: Marc Jeanmougin <marc@jeanmougin.fr>
 */

#include "cairo-test.h"

static void
set_dither_source (cairo_t *cr, int width)
{
    cairo_pattern_t *gradient = cairo_pattern_create_linear (0, 0, width, 0);
    cairo_pattern_add_color_stop_rgba (gradient, 0., 25./255, 25./255, 25./255, 1.0);
    cairo_pattern_add_color_stop_rgba (gradient, 1., 45./255, 45./255, 45./255, 1.0);
    
    cairo_set_source (cr, gradient);
    cairo_pattern_set_dither (gradient, CAIRO_DITHER_BEST);
    cairo_pattern_destroy (gradient);
}

/* History:
 *
 * 2023: v3 of a patch to use pixman dithering with cairo
 */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    set_dither_source (cr, width);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw2 (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    set_dither_source (cr, width);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    for (int i = 0; i < 5; i++) {
        cairo_paint (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (dithergradient,
	    "Testing the creation of a dithered gradient (in argb32)",
	    "gradient, dither", /* keywords */
	    NULL, /* requirements */
	    400, 100,
	    NULL, draw)
CAIRO_TEST (dithergradient2,
	    "Testing the creation of a dithered gradient (in argb32)",
	    "gradient, dither", /* keywords */
	    NULL, /* requirements */
	    400, 100,
	    NULL, draw2)
