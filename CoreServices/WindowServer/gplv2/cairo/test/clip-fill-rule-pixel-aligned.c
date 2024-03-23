/*
 * Copyright © 2004 Red Hat, Inc.
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
 */

#include "cairo-test.h"

#define PAD 1
#define SIZE 5

static void
pixel_aligned_path (cairo_t *cr)
{
    cairo_save (cr);
    {
	cairo_scale (cr, SIZE, SIZE);
	cairo_move_to     (cr,  1,  0);
	cairo_rel_line_to (cr,  1,  0);
	cairo_rel_line_to (cr,  0,  3);
	cairo_rel_line_to (cr,  1,  0);
	cairo_rel_line_to (cr,  0, -1);
	cairo_rel_line_to (cr, -3,  0);
	cairo_rel_line_to (cr,  0, -1);
	cairo_rel_line_to (cr,  4,  0);
	cairo_rel_line_to (cr,  0,  3);
	cairo_rel_line_to (cr, -3,  0);
	cairo_rel_line_to (cr,  0, -4);
	cairo_close_path (cr);
    }
    cairo_restore (cr);
}

/* Use clipping to draw the same path twice, once with each fill rule */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 0, 0); /* red */

    cairo_translate (cr, PAD, PAD);

    cairo_save (cr);
    {
	pixel_aligned_path (cr);
	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_clip (cr);
	cairo_paint (cr);
    }
    cairo_restore (cr);

    cairo_translate (cr, SIZE*4 + PAD, 0);

    cairo_save (cr);
    {
	pixel_aligned_path (cr);
	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);
	cairo_clip (cr);
	cairo_paint (cr);
    }
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_fill_rule_pixel_aligned,
	    "Tests interaction of clipping and cairo_set_fill_rule with a pixel-aligned path",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    PAD + (SIZE*4) + PAD + (SIZE*4) + PAD,
	    PAD + (SIZE*4) + PAD,
	    NULL, draw)
