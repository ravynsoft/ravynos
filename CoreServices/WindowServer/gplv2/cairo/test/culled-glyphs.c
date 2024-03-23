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
    const char *text =
"This needs to be a very long string, wider than the surface, and yet wider."
"Ideally it should overflow the stack buffers, but do you really want to read "
"a message that long. No. So we compromise with around 300 glyphs that is "
"long enough to trigger the conditions as stated in "
"https://lists.cairographics.org/archives/cairo/2008-December/015976.html. "
"Happy now?";
    cairo_text_extents_t extents;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_set_font_size (cr, 16);
    cairo_text_extents (cr, text, &extents);
    cairo_move_to (cr, -extents.width/2, 18);
    cairo_show_text (cr, text);

    /* XXX we should exercise cairo_show_text_glyphs() as well,
     * and CAIRO_TEXT_CLUSTER_BACKWARDS
     */

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (culled_glyphs,
	    "Tests culling of glyphs and text clusters",
	    "glyphs", /* keywords */
	    NULL, /* requirements */
	    20, 20,
	    NULL, draw)

