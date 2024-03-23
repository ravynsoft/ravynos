/* -*- Mode: c; c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2006 Brian Ewins.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Brian Ewins not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Brian Ewins makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * BRIAN EWINS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL BRIAN EWINS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Brian Ewins <Brian.Ewins@gmail.com>
 */

/* Related to bug 9530
 *
 * cairo_glyph_t can contain any unsigned long in its 'index', the intention
 * being that it is large enough to hold a pointer. However, this means that
 * it can specify many glyph indexes which don't exist in the font, and may
 * exceed the range of legal glyph indexes for the font backend. It may
 * also contain special values that are not usable as indexes - e.g. 0xffff is
 * kATSDeletedGlyphcode in ATSUI, a glyph that should not be drawn.
 * The font backends should handle all legal and out-of-range values
 * consistently.
 *
 * This test expects that operations on out-of-range and missing glyphs should
 * act as if they were zero-width.
 */

#include "cairo-test.h"

#define WIDTH  100
#define HEIGHT 75
#define NUM_TEXT 20
#define TEXT_SIZE 12

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_text_extents_t extents;
    int i;
    /* Glyphs with no paths followed by 'cairo', the additional
     * text is to make the space obvious.
     */
    long int index[] = {
	0, /* 'no matching glyph' */
	0xffff, /* kATSDeletedGlyphCode */
	0x1ffff, /* out of range */
	-1L, /* out of range */
	70, 68, 76, 85, 82 /* 'cairo' */
    };

    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 16);

    for (i = 0; i < 9; i++) {
	/* since we're just drawing glyphs directly we need to position them. */
	cairo_glyph_t glyph = {
	    index[i], 10 * i, 25
	};

	/* test cairo_glyph_extents. Every glyph index should
	 * have extents, invalid glyphs should be zero-width.
	 */
	cairo_move_to (cr, glyph.x, glyph.y);
	cairo_set_line_width (cr, 1.0);
	cairo_glyph_extents (cr, &glyph, 1, &extents);
	cairo_rectangle (cr,
			 glyph.x + extents.x_bearing - 0.5,
			 glyph.y + extents.y_bearing - 0.5,
			 extents.width + 1,
			 extents.height + 1);
	cairo_set_source_rgb (cr, 1, 0, 0); /* red */
	cairo_stroke (cr);

	/* test cairo_show_glyphs. Every glyph index should be
	 * drawable, invalid glyph indexes should draw nothing.
	 */
	cairo_set_source_rgb (cr, 0, 0, 0); /* black */
	cairo_show_glyphs (cr, &glyph, 1);
	cairo_move_to (cr, glyph.x, glyph.y);

	/* test cairo_glyph_path. Every glyph index should produce
	 * a path, invalid glyph indexes should have empty paths.
	 */
	/* Change the glyph position
	 * so that the paths are visible.
	 */
	glyph.y = 55;
	cairo_move_to (cr, glyph.x, glyph.y);
	cairo_glyph_path (cr, &glyph, 1);
	cairo_fill (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (text_glyph_range,
	    "Tests show_glyphs, glyph_path, glyph_extents with out of range glyph ids."
	    "\nft and atsui font backends fail, misreporting errors from FT_Load_Glyph and ATSUGlyphGetCubicPaths",
	    "text, stress", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
