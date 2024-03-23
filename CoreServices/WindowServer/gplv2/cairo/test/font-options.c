/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#include <assert.h>

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_font_options_t *default_options;
    cairo_font_options_t *nil_options;
    cairo_surface_t *surface;
    cairo_matrix_t identity;
    cairo_t *cr;
    cairo_scaled_font_t *scaled_font;

    /* first check NULL handling of cairo_font_options_t */
    default_options = cairo_font_options_create ();
    assert (cairo_font_options_status (default_options) == CAIRO_STATUS_SUCCESS);
    nil_options = cairo_font_options_copy (NULL);
    assert (cairo_font_options_status (nil_options) == CAIRO_STATUS_NO_MEMORY);

    assert (cairo_font_options_equal (default_options, default_options));
    assert (! cairo_font_options_equal (default_options, nil_options));
    assert (! cairo_font_options_equal (NULL, nil_options));
    assert (! cairo_font_options_equal (nil_options, nil_options));
    assert (! cairo_font_options_equal (default_options, NULL));
    assert (! cairo_font_options_equal (NULL, default_options));

    assert (cairo_font_options_hash (default_options) == cairo_font_options_hash (nil_options));
    assert (cairo_font_options_hash (NULL) == cairo_font_options_hash (nil_options));
    assert (cairo_font_options_hash (default_options) == cairo_font_options_hash (NULL));

    cairo_font_options_merge (NULL, NULL);
    cairo_font_options_merge (default_options, NULL);
    cairo_font_options_merge (default_options, nil_options);

    cairo_font_options_set_antialias (NULL, CAIRO_ANTIALIAS_DEFAULT);
    cairo_font_options_get_antialias (NULL);
    assert (cairo_font_options_get_antialias (default_options) == CAIRO_ANTIALIAS_DEFAULT);

    cairo_font_options_set_subpixel_order (NULL, CAIRO_SUBPIXEL_ORDER_DEFAULT);
    cairo_font_options_get_subpixel_order (NULL);
    assert (cairo_font_options_get_subpixel_order (default_options) == CAIRO_SUBPIXEL_ORDER_DEFAULT);

    cairo_font_options_set_hint_style (NULL, CAIRO_HINT_STYLE_DEFAULT);
    cairo_font_options_get_hint_style (NULL);
    assert (cairo_font_options_get_hint_style (default_options) == CAIRO_HINT_STYLE_DEFAULT);

    cairo_font_options_set_hint_metrics (NULL, CAIRO_HINT_METRICS_DEFAULT);
    cairo_font_options_get_hint_metrics (NULL);
    assert (cairo_font_options_get_hint_metrics (default_options) == CAIRO_HINT_METRICS_DEFAULT);

    cairo_font_options_set_color_palette (NULL, CAIRO_COLOR_PALETTE_DEFAULT);
    cairo_font_options_get_color_palette (NULL);
    assert (cairo_font_options_get_color_palette (default_options) == CAIRO_COLOR_PALETTE_DEFAULT);

    cairo_font_options_destroy (NULL);
    cairo_font_options_destroy (default_options);
    cairo_font_options_destroy (nil_options);


    /* Now try creating fonts with NULLs */
    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 0, 0);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_matrix_init_identity (&identity);
    scaled_font = cairo_scaled_font_create (cairo_get_font_face (cr),
	                                    &identity, &identity,
					    NULL);
    assert (cairo_scaled_font_status (scaled_font) == CAIRO_STATUS_NULL_POINTER);
    cairo_scaled_font_get_font_options (scaled_font, NULL);
    cairo_scaled_font_destroy (scaled_font);

    assert (cairo_status (cr) == CAIRO_STATUS_SUCCESS);
    cairo_get_font_options (cr, NULL);
    cairo_set_font_options (cr, NULL);
    assert (cairo_status (cr) == CAIRO_STATUS_NULL_POINTER);

    cairo_destroy (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (font_options,
	    "Check setters and getters on cairo_font_options_t.",
	    "font, api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
