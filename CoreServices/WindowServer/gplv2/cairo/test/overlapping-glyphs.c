/*
 * Copyright © 2009 Chris Wilson
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

#include <assert.h>

#define TEXT_SIZE 12
#define HEIGHT (TEXT_SIZE + 4)
#define WIDTH 50

#define MAX_GLYPHS 80

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_glyph_t glyphs_stack[MAX_GLYPHS], *glyphs;
    const char *cairo = "Cairo";
    const char *giza = "Giza";
    cairo_text_extents_t cairo_extents;
    cairo_text_extents_t giza_extents;
    int count, num_glyphs;
    double x0, y0;

    /* We draw in the default black, so paint white first. */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, TEXT_SIZE);

    /* We want to overlap two strings, so compute overlapping glyphs.  */

    cairo_text_extents (cr, cairo, &cairo_extents);
    cairo_text_extents (cr, giza, &giza_extents);

    x0 = WIDTH/2. - (cairo_extents.width/2. + cairo_extents.x_bearing);
    y0 = HEIGHT/2. - (cairo_extents.height/2. + cairo_extents.y_bearing);
    glyphs = glyphs_stack;
    count = MAX_GLYPHS;
    cairo_scaled_font_text_to_glyphs (cairo_get_scaled_font (cr),
				      x0, y0,
				      cairo, strlen (cairo),
				      &glyphs, &count,
				      NULL, NULL,
				      NULL);
    assert (glyphs == glyphs_stack);
    num_glyphs = count;

    x0 = WIDTH/2. - (giza_extents.width/2. + giza_extents.x_bearing);
    y0 = HEIGHT/2. - (giza_extents.height/2. + giza_extents.y_bearing);
    glyphs = glyphs_stack + count;
    count = MAX_GLYPHS - count;
    cairo_scaled_font_text_to_glyphs (cairo_get_scaled_font (cr),
				      x0, y0,
				      giza, strlen (giza),
				      &glyphs, &count,
				      NULL, NULL,
				      NULL);
    assert (glyphs == glyphs_stack + num_glyphs);
    glyphs = glyphs_stack;
    num_glyphs += count;

    cairo_set_source_rgba (cr, 0, 0, 0, .5); /* translucent black, gray! */
    cairo_show_glyphs (cr, glyphs, num_glyphs);

    /* and compare with filling */
    cairo_translate (cr, 0, HEIGHT);
    cairo_glyph_path (cr, glyphs, num_glyphs);
    cairo_fill (cr);

    /* switch to using an unbounded operator for added complexity */
    cairo_set_operator (cr, CAIRO_OPERATOR_IN);

    cairo_translate (cr, WIDTH, -HEIGHT);
    cairo_save (cr);
    cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
    cairo_clip (cr);
    cairo_show_glyphs (cr, glyphs, num_glyphs);
    cairo_restore (cr);

    cairo_translate (cr, 0, HEIGHT);
    cairo_save (cr);
    cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
    cairo_clip (cr);
    cairo_glyph_path (cr, glyphs, num_glyphs);
    cairo_fill (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (overlapping_glyphs,
	    "Test handing of overlapping glyphs",
	    "text, glyphs", /* keywords */
	    NULL, /* requirements */
	    2 * WIDTH, 2 * HEIGHT,
	    NULL, draw)

