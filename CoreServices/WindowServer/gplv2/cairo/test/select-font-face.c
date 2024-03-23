/*
 * Copyright © 2006 Red Hat, Inc.
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

#define TEXT_SIZE 12

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* We draw in the default black, so paint white first. */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0); /* black */

    cairo_set_font_size (cr, TEXT_SIZE);
    cairo_move_to (cr, 0, TEXT_SIZE);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Serif",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_show_text (cr, "i-am-serif");

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_show_text (cr, " i-am-sans");

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans Mono",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_show_text (cr, " i-am-mono");

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (select_font_face,
	    "Tests using cairo_select_font_face to draw text in different faces",
	    "font", /* keywords */
	    NULL, /* requirements */
	    192, TEXT_SIZE + 4,
	    NULL, draw)
