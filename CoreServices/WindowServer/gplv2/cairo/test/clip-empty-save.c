/*
 * Copyright © 2007 Chris Wilson
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

#define SIZE 10

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_paint (cr);

    cairo_reset_clip (cr);
    cairo_clip (cr);

    cairo_save (cr);

    cairo_translate (cr, .5, .5);

    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_stroke (cr);

    /* https://bugs.freedesktop.org/show_bug.cgi?id=13084 */
    cairo_select_font_face (cr,
	                    CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_move_to (cr, 0., SIZE);
    cairo_show_text (cr, "cairo");

    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_empty_save,
	    "Test clipping with an empty clip path",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
