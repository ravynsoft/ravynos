/*
 * Copyright © 2005 Red Hat, Inc.
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
#include "cairo-boilerplate-scaled-font.h"

#define TEXT_SIZE 12

/* Bug history
 *
 * 2006-06-22  Carl Worth  <cworth@cworth.org>
 *
 *   This is a test case to demonstrate the following bug in the xlib backend:
 *
 *	Some characters aren't displayed when using xlib (cache usage missing freeze/thaw)
 *	https://bugs.freedesktop.org/show_bug.cgi?id=6955
 *
 *   We replicate this bug by using the cairo_scaled_font_set_max_glyphs_per_font
 *   function to artificially induce cache pressure. (This function was added
 *   for this very purpose.)
 *
 * 2006-06-22  Carl Worth  <cworth@cworth.org>
 *
 *   Bug was simple enough to solve by just adding a freeze/thaw pair
 *   around the scaled_font's glyph cache in
 *   _cairo_xlib_surface_show_glyphs, (I went ahead and added
 *   _cairo_sacled_font_freeze/thaw_cache functions for this).
 */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, TEXT_SIZE);

    cairo_set_source_rgb (cr, 0, 0, 0); /* black */

    cairo_boilerplate_scaled_font_set_max_glyphs_cached (cairo_get_scaled_font (cr), 1);

    cairo_move_to (cr, 1, TEXT_SIZE);
    cairo_show_text (cr, "the five boxing wizards jump quickly");

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (glyph_cache_pressure,
	    "Ensure that all backends behave well under artificial glyph cache pressure",
	    "stress", /* keywords */
	    NULL, /* requirements */
	    223, TEXT_SIZE + 4,
	    NULL, draw)
