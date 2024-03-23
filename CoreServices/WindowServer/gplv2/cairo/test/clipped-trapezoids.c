/*
 * Copyright 2008 Chris Wilson
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

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double dash[2] = { 8, 4 };
    double radius;

    radius = width;
    if (height > radius)
	radius = height;

    /* fill the background using a big circle */
    cairo_arc (cr, 0, 0, 4 * radius, 0, 2 * M_PI);
    cairo_fill (cr);

    /* a rotated square - overlapping the corners */
    cairo_save (cr);
    cairo_save (cr);
    cairo_translate (cr, width/2, height/2);
    cairo_rotate (cr, M_PI/4);
    cairo_scale (cr, M_SQRT2, M_SQRT2);
    cairo_rectangle (cr, -width/2, -height/2, width, height);
    cairo_restore (cr);
    cairo_set_source_rgba (cr, 0, 1, 0, .5);
    cairo_set_line_width (cr, radius/2);
    cairo_stroke (cr);
    cairo_restore (cr);

    /* and put some circles in the corners */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_new_sub_path (cr);
    cairo_arc (cr, 0, 0, radius/4, 0, 2 * M_PI);
    cairo_new_sub_path (cr);
    cairo_arc (cr, width, 0, radius/4, 0, 2 * M_PI);
    cairo_new_sub_path (cr);
    cairo_arc (cr, width, height, radius/4, 0, 2 * M_PI);
    cairo_new_sub_path (cr);
    cairo_arc (cr, 0, height, radius/4, 0, 2 * M_PI);
    cairo_fill (cr);

    /* a couple of pixel-aligned lines */
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, width/2, -height);
    cairo_rel_line_to (cr, 0, 3*height);
    cairo_move_to (cr, -width, height/2);
    cairo_rel_line_to (cr, 3*width, 0);
    cairo_stroke (cr);

    /* a couple of dashed diagonals */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_set_dash (cr, dash, 2, 0);
    cairo_set_line_width (cr, 4.);
    cairo_move_to (cr, -width, -height);
    cairo_line_to (cr, width+width, height+height);
    cairo_move_to (cr, width+width, -height);
    cairo_line_to (cr, -width, height+height);
    cairo_stroke (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clipped_trapezoids,
	    "Tests clipping of trapezoids larger than the surface",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    40, 40,
	    NULL, draw)
