/*
 * Copyright © 2005 Billy Biggs
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Billy Biggs not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Billy Biggs makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * BILLY BIGGS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL BILLY BIGGS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Billy Biggs <vektor@dumbterm.net>
 */

#include "cairo-test.h"

/* The star shape from the SVG test suite, from the fill rule test */
static void
big_star_path (cairo_t *cr)
{
    cairo_move_to (cr, 40, 0);
    cairo_rel_line_to (cr, 25, 80);
    cairo_rel_line_to (cr, -65, -50);
    cairo_rel_line_to (cr, 80, 0);
    cairo_rel_line_to (cr, -65, 50);
    cairo_close_path (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    /* Try a circle */
    cairo_arc (cr, 40, 40, 20, 0, 2 * M_PI);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill (cr);

    /* Try using clipping to draw a circle */
    cairo_arc (cr, 100, 40, 20, 0, 2 * M_PI);
    cairo_clip (cr);
    cairo_rectangle (cr, 80, 20, 40, 40);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_fill (cr);

    /* Reset the clipping */
    cairo_reset_clip (cr);

    /* Draw a bunch of lines */
    cairo_set_line_width (cr, 1.0);
    cairo_set_source_rgb (cr, 0, 1, 0);
    for (i = 0; i < 10; i++) {
        cairo_move_to (cr, 10, 70 + (i * 4));
        cairo_line_to (cr, 120, 70 + (i * 18));
        cairo_stroke (cr);
    }

    /* Try filling a poly */
    cairo_translate (cr, 160, 120);
    cairo_set_source_rgb (cr, 1, 1, 0);
    big_star_path (cr);
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_fill (cr);
    cairo_translate (cr, -160, -120);

    /* How about some curves? */
    cairo_set_source_rgb (cr, 1, 0, 1);
    for (i = 0; i < 10; i++) {
        cairo_move_to (cr, 150, 50 + (i * 5));
        cairo_curve_to (cr, 250, 50, 200, (i * 10), 300, 50 + (i * 10));
        cairo_stroke (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (unantialiased_shapes,
	    "Test shape drawing without antialiasing",
	    "fill, stroke", /* keywords */
	    "target=raster", /* requirements */
	    320, 240,
	    NULL, draw)
