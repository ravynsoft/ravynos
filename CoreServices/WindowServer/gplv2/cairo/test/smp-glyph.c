/*
 * Copyright © 2017 Andrea Canciani
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Andrea Canciani not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Andrea Canciani makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * ANDREA CANCIANI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL ANDREA CANCIANI BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

#define TEXT_SIZE 42

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0); /* black */

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size (cr, TEXT_SIZE);
    cairo_translate (cr, 0, TEXT_SIZE);

    /* U+1F030, DOMINO TILE HORIZONTAL BACK */
    cairo_show_text (cr, "\xf0\x9f\x80\xb0");

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (smp_glyph,
	    "Test glyphs for symbols in the Supplementary Multilingual Plane",
	    "text, glyphs", /* keywords */
	    NULL, /* requirements */
	    64, 64,
	    NULL, draw)
