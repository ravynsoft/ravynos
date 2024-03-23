/*
 * Copyright © 2007 Mozilla Corporation
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
 * Author: Vladimir Vukicevic <vladimir@pobox.com>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pat;
    cairo_surface_t *surf;

    cairo_new_path (cr);
    cairo_rectangle (cr, 0, 0, 0, 0);
    cairo_clip (cr);

    cairo_push_group (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);

    cairo_new_path (cr);
    cairo_rectangle (cr, -10, 10, 20, 20);
    cairo_fill_preserve (cr);
    cairo_stroke_preserve (cr);
    cairo_paint (cr);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_show_text (cr, "ABC");

    cairo_mask (cr, cairo_get_source (cr));

    surf = cairo_surface_create_similar (cairo_get_group_target (cr), CAIRO_CONTENT_COLOR_ALPHA, 0, 0);
    pat = cairo_pattern_create_for_surface (surf);
    cairo_surface_destroy (surf);

    cairo_mask (cr, pat);
    cairo_pattern_destroy (pat);

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_zero,
	    "Verifies that 0x0 surfaces or clips don't cause problems.",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    NULL, draw)
