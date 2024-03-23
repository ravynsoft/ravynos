/*
 * Copyright 2010 Intel Corporation
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
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

/* Try to replicate the misbehaviour of show_glyphs() versus glyph_path()
 * in the PDF backend reported by Ian Britten.
 */

static void
halo_around_path (cairo_t *cr, const char *str)
{
    cairo_text_path (cr, str);

    cairo_set_source_rgb (cr, 0, .5, 1);
    cairo_stroke_preserve (cr);
    cairo_set_source_rgb (cr, 1, .5, 0);
    cairo_fill (cr);
}

static void
halo_around_text (cairo_t *cr, const char *str)
{
    double x, y;

    cairo_get_current_point (cr, &x, &y);
    cairo_text_path (cr, str);

    cairo_set_source_rgb (cr, 0, .5, 1);
    cairo_stroke(cr);

    cairo_set_source_rgb (cr, 1, .5, 0);
    cairo_move_to (cr, x, y);
    cairo_show_text  (cr, str);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const char *string = "0123456789";
    cairo_text_extents_t extents;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_text_extents (cr, string, &extents);

    cairo_set_font_size (cr, 12);
    cairo_set_line_width (cr, 3);
    cairo_move_to (cr, 9, 4 + extents.height);
    halo_around_path (cr, string);

    cairo_move_to (cr, 109, 4 + extents.height);
    halo_around_text (cr, string);

    cairo_set_font_size (cr, 6);
    cairo_set_line_width (cr, 3);
    cairo_move_to (cr, 19 + extents.width, 20 + extents.height);
    halo_around_path (cr, "0");

    cairo_move_to (cr, 119 + extents.width, 20 + extents.height);
    halo_around_text (cr, "0");

    cairo_set_font_size (cr, 64);
    cairo_set_line_width (cr, 10);
    cairo_move_to (cr, 8, 70);
    halo_around_path (cr, "6");
    cairo_move_to (cr, 32, 90);
    halo_around_path (cr, "7");

    cairo_move_to (cr, 108, 70);
    halo_around_text (cr, "6");
    cairo_move_to (cr, 132, 90);
    halo_around_text (cr, "7");

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_transform (cairo_t *cr, int width, int height)
{
    const char *string = "0123456789";
    cairo_text_extents_t extents;

    cairo_translate (cr, 50, 50);
    cairo_scale (cr, M_SQRT2, M_SQRT2);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_text_extents (cr, string, &extents);

    cairo_set_line_width (cr, 3);
    cairo_move_to (cr, 9, 4 + extents.height);
    halo_around_path (cr, string);

    cairo_move_to (cr, 109, 4 + extents.height);
    halo_around_text (cr, string);

    cairo_set_font_size (cr, 6);
    cairo_set_line_width (cr, 3);
    cairo_move_to (cr, 19 + extents.width, 20 + extents.height);
    halo_around_path (cr, "0");

    cairo_move_to (cr, 119 + extents.width, 20 + extents.height);
    halo_around_text (cr, "0");

    cairo_set_font_size (cr, 64);
    cairo_set_line_width (cr, 10);
    cairo_move_to (cr, 8, 70);
    halo_around_path (cr, "6");
    cairo_move_to (cr, 32, 90);
    halo_around_path (cr, "7");

    cairo_move_to (cr, 108, 70);
    halo_around_text (cr, "6");
    cairo_move_to (cr, 132, 90);
    halo_around_text (cr, "7");

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (halo,
	    "Check the show_glyphs() vs glyph_path()",
	    "text", /* keywords */
	    NULL, /* requirements */
	    200, 100,
	    NULL, draw)

CAIRO_TEST (halo_transform,
	    "Check the show_glyphs() vs glyph_path()",
	    "text", /* keywords */
	    NULL, /* requirements */
	    400, 200,
	    NULL, draw_transform)
