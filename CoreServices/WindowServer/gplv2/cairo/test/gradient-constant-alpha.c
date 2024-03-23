/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2008 Chris Wilson
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
 * Author: Carl D. Worth <cworth@cworth.org>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *gradient;

    gradient = cairo_pattern_create_linear (0, 0,
					    0, height);
    cairo_pattern_add_color_stop_rgba (gradient, 0.0,
				       1.0, 0.0, 0.0,
				       0.5);
    cairo_pattern_add_color_stop_rgba (gradient, 0.0,
				       0.0, 1.0, 0.0,
				       0.5);
    cairo_pattern_add_color_stop_rgba (gradient, 1.0,
				       0.0, 0.0, 1.0,
				       0.5);

    cairo_set_source (cr, gradient);

    cairo_paint (cr);

    cairo_pattern_destroy (gradient);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (gradient_constant_alpha,
	    "Tests drawing of a gradient with constant alpha values in the color stops",
	    "gradient, alpha", /* keywords */
	    NULL,
	    10, 10,
	    NULL, draw)
