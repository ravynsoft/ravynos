/*
 * Copyright 2009 Chris Wilson
 * Copyright 2011 Intel Corporation
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

#define WIDTH 20
#define HEIGHT 20

static void clip_mask (cairo_t *cr)
{
    cairo_move_to (cr, 10, 0);
    cairo_line_to (cr, 0, 10);
    cairo_line_to (cr, 10, 20);
    cairo_line_to (cr, 20, 10);
    cairo_clip (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    clip_mask (cr);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);

    cairo_rectangle (cr, 0, 0, 4, 4);
    cairo_clip (cr);
    clip_mask (cr);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    cairo_rectangle (cr, 20, 0, -4, 4);
    cairo_clip (cr);
    clip_mask (cr);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    cairo_rectangle (cr, 20, 20, -4, -4);
    cairo_clip (cr);
    clip_mask (cr);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    cairo_rectangle (cr, 0, 20, 4, -4);
    cairo_clip (cr);
    clip_mask (cr);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    cairo_set_source_rgb (cr, 0, 0, 1);

    cairo_rectangle (cr, 8, 8, 4, 4);
    cairo_clip (cr);
    clip_mask (cr);
    cairo_paint (cr);
    cairo_reset_clip (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_intersect,
	    "Tests intersection of a simple clip with a clip-mask",
	    "clip, paint", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
