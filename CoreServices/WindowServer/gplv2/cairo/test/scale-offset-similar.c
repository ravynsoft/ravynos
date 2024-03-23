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

/*
 * Test case derived from the bug report by Michel Iwaniec:
 * https://lists.cairographics.org/archives/cairo/2008-November/015660.html
 */

#include "cairo-test.h"

static cairo_surface_t *
create_source (cairo_surface_t *target, int width, int height)
{
    cairo_surface_t *similar;
    cairo_t *cr;

    similar = cairo_surface_create_similar (target,
					    CAIRO_CONTENT_COLOR,
					    width, height);
    cr = cairo_create (similar);
    cairo_surface_destroy (similar);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr,
		     width - 4, height - 4,
		     2, 2);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_rectangle (cr,
		     width - 2, height - 4,
		     2, 2);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_rectangle (cr,
		     width - 4, height - 2,
		     2, 2);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_rectangle (cr,
		     width - 2, height - 2,
		     2, 2);
    cairo_fill (cr);

    similar = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return similar;
}

static void
draw_grid (cairo_t *cr, cairo_pattern_t *pattern, int dst_x, int dst_y)
{
    cairo_matrix_t m;

    cairo_save (cr);
    cairo_translate (cr, dst_x, dst_y);
    cairo_scale (cr, 16, 16);
    cairo_rotate (cr, 1);

    cairo_matrix_init_translate (&m, 2560-4, 1280-4);
    cairo_pattern_set_matrix (pattern, &m);
    cairo_set_source (cr, pattern);
    cairo_rectangle (cr, 0, 0, 4, 4);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, .7, .7, .7);
    cairo_set_line_width (cr, 1./16);
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, 4, 0);
    cairo_move_to (cr, 0, 2);
    cairo_line_to (cr, 4, 2);
    cairo_move_to (cr, 0, 4);
    cairo_line_to (cr, 4, 4);
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, 0, 4);
    cairo_move_to (cr, 2, 0);
    cairo_line_to (cr, 2, 4);
    cairo_move_to (cr, 4, 0);
    cairo_line_to (cr, 4, 4);
    cairo_stroke (cr);

    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *source;
    cairo_pattern_t *pattern;

    cairo_paint (cr);

    source = create_source (cairo_get_target (cr), 2560, 1280);
    pattern = cairo_pattern_create_for_surface (source);
    cairo_surface_destroy (source);

    cairo_pattern_set_filter (pattern, CAIRO_FILTER_NEAREST);
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_NONE);

    draw_grid (cr, pattern, 50, 0);
    draw_grid (cr, pattern, 130, 0);
    draw_grid (cr, pattern, 210, 0);
    draw_grid (cr, pattern, 290, 0);

    draw_grid (cr, pattern, 50,  230);
    draw_grid (cr, pattern, 130, 230);
    draw_grid (cr, pattern, 210, 230);
    draw_grid (cr, pattern, 290, 230);

    cairo_pattern_destroy (pattern);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (scale_offset_similar,
	    "Tests drawing surfaces under various scales and transforms",
	    "surface, scale-offset", /* keywords */
	    NULL, /* requirements */
	    320, 320,
	    NULL, draw)

