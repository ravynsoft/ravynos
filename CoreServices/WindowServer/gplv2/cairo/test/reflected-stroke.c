/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * the author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE AUTHOR. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

static void
draw_symbol (cairo_t *cr)
{
    double dash[] = {6, 3};

    cairo_rectangle (cr, -25, -25, 50, 50);
    cairo_stroke (cr);

    cairo_move_to (cr, 0, -25);
    cairo_curve_to (cr, 12.5, -12.5, 12.5, -12.5, 0, 0);
    cairo_curve_to (cr, -12.5, 12.5, -12.5, 12.5, 0, 25);
    cairo_curve_to (cr, 12.5, 12.5, 12.5, 12.5, 0, 0);
    cairo_stroke (cr);

    cairo_save (cr);
    cairo_set_dash (cr, dash, ARRAY_LENGTH (dash), 0.);
    cairo_move_to (cr, 0, 0);
    cairo_arc (cr, 0, 0, 12.5, 0, 3 * M_PI / 2);
    cairo_close_path (cr);
    cairo_stroke (cr);
    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_save (cr);
    cairo_translate (cr, 50, 50);
    cairo_scale (cr, 1, 1);
    draw_symbol (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_translate (cr, 150, 50);
    cairo_scale (cr, -1, 1);
    draw_symbol (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_translate (cr, 150, 150);
    cairo_scale (cr, -1, -1);
    draw_symbol (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_translate (cr, 50, 150);
    cairo_scale (cr, 1, -1);
    draw_symbol (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (reflected_stroke,
	    "Exercises the stroker with a reflected ctm",
	    "stroke, transform", /* keywords */
	    NULL, /* requirements */
	    200, 200,
	    NULL, draw)
