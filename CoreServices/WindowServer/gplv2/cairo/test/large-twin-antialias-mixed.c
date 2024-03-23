/*
 * Copyright 2009 Chris Wilson
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

static cairo_scaled_font_t *
create_twin (cairo_t *cr, cairo_antialias_t antialias)
{
    cairo_font_options_t *options;

    cairo_select_font_face (cr,
			    "@cairo:",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    options = cairo_font_options_create ();
    cairo_font_options_set_antialias (options, antialias);
    cairo_set_font_options (cr, options);
    cairo_font_options_destroy (options);

    return cairo_scaled_font_reference (cairo_get_scaled_font (cr));
}


static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_scaled_font_t *subpixel, *gray, *none;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_set_font_size (cr, 64);
    subpixel = create_twin (cr, CAIRO_ANTIALIAS_SUBPIXEL);
    gray = create_twin (cr, CAIRO_ANTIALIAS_GRAY);
    none = create_twin (cr, CAIRO_ANTIALIAS_NONE);

    cairo_move_to (cr, 4, 64);
    cairo_set_scaled_font (cr, subpixel);
    cairo_show_text (cr, "Is cairo's");
    cairo_set_scaled_font (cr, gray);
    cairo_show_text (cr, " twin");
    cairo_set_scaled_font (cr, none);
    cairo_show_text (cr, " giza?");

    cairo_move_to (cr, 4, 128+16);
    cairo_set_scaled_font (cr, gray);
    cairo_show_text (cr, "Is cairo's");
    cairo_set_scaled_font (cr, none);
    cairo_show_text (cr, " twin");
    cairo_set_scaled_font (cr, subpixel);
    cairo_show_text (cr, " giza?");

    cairo_move_to (cr, 4, 192+32);
    cairo_set_scaled_font (cr, none);
    cairo_show_text (cr, "Is cairo's");
    cairo_set_scaled_font (cr, gray);
    cairo_show_text (cr, " twin");
    cairo_set_scaled_font (cr, subpixel);
    cairo_show_text (cr, " giza?");

    cairo_scaled_font_destroy (none);
    cairo_scaled_font_destroy (gray);
    cairo_scaled_font_destroy (subpixel);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (large_twin_antialias_mixed,
	    "Tests the internal font (with intermixed antialiasing)",
	    "twin, font", /* keywords */
	    "target=raster", /* requirements */
	    524, 240,
	    NULL, draw)
