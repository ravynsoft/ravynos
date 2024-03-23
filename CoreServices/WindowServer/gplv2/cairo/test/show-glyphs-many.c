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

#include <string.h>

/* Bug history
 *
 * 2006-01-07  Jon Hellan  <hellan@acm.org>
 *
 *   Jon opened the following bug report:
 *
 *	_XError from XRenderCompositeText8
 *	https://bugs.freedesktop.org/show_bug.cgi?id=5528
 *
 * 2006-03-02  Carl Worth  <cworth@cworth.org>
 *
 *   I wrote this test case to demonstrate the bug.
 *
 *   Approach:
 *
 *	Draw 65535 glyphs white-on-white all on top of each other.
 *
 *   Rationale:
 *
 *	The number 65535 comes from the original bug report.
 *
 *	I would use cairo_show_text with a long string of 'x's say,
 *	but then the surface would need to be enormous to contain
 *	them. A smaller surface could be used, but I fear that at some
 *	point the off-surface glyph drawing would be optimized away
 *	and not exercise the bug.
 *
 *	So, to keep the surface size under control, I use
 *	cairo_show_glyphs which allows me to place the glyphs all on
 *	top of each other. But, since cairo doesn't provide any
 *	character-to-glyphs mapping, I can't get a reliable glyph
 *	index (for character 'x' for example). So I just "guess" a
 *	glyph index and use white-on-white drawing to ignore the
 *	result. (I don't care what's drawn---I just want to ensure
 *	that things don't crash.)
 *
 *  Status: I replicated bug. The largest value of NUM_GLYPHS for
 *      which I saw success is 21842.
 *
 * 2008-30-08 Chris Wilson <chris@chris-wilson.co.uk>
 *   This is also a valid test case for:
 *
 *     Bug 5913 crash on overlong string
 *     https://bugs.freedesktop.org/show_bug.cgi?id=5913
 *
 *  which is still causing a crash in the Xlib backend - presumably, just
 *  a miscalculation of the length of the available request.
 */

#define TEXT_SIZE 12
#define NUM_GLYPHS 65535

static cairo_test_status_t
get_glyph (const cairo_test_context_t *ctx,
	   cairo_scaled_font_t *scaled_font,
	   const char *utf8,
	   cairo_glyph_t *glyph)
{
    cairo_glyph_t *text_to_glyphs;
    cairo_status_t status;
    int i;

    text_to_glyphs = glyph;
    i = 1;
    status = cairo_scaled_font_text_to_glyphs (scaled_font,
					       0, 0,
					       utf8, -1,
					       &text_to_glyphs, &i,
					       NULL, NULL,
					       0);
    if (status != CAIRO_STATUS_SUCCESS)
	return cairo_test_status_from_status (ctx, status);

    if (text_to_glyphs != glyph) {
	*glyph = text_to_glyphs[0];
	cairo_glyph_free (text_to_glyphs);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_glyph_t *glyphs = xmalloc (NUM_GLYPHS * sizeof (cairo_glyph_t));
    cairo_scaled_font_t *scaled_font;
    const char *characters[] = { /* try to exercise different widths of index */
	"m", /* Latin letter m, index=0x50 */
	"μ", /* Greek letter mu, index=0x349 */
	NULL,
    }, **utf8;
    int i, j;
    cairo_status_t status;

    /* Paint white background. */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, TEXT_SIZE);
    scaled_font = cairo_get_scaled_font (cr);

    for (utf8 = characters; *utf8 != NULL; utf8++) {
	status = get_glyph (ctx, scaled_font, *utf8, &glyphs[0]);
	if (status)
	    goto BAIL;

	if (glyphs[0].index) {
	    glyphs[0].x = 1.0;
	    glyphs[0].y = height - 1;
	    for (i=1; i < NUM_GLYPHS; i++)
		glyphs[i] = glyphs[0];

	    cairo_show_glyphs (cr, glyphs, NUM_GLYPHS);
	}
    }

    /* we can pack ~21k 1-byte glyphs into a single XRenderCompositeGlyphs8 */
    status = get_glyph (ctx, scaled_font, "m", &glyphs[0]);
    if (status)
	goto BAIL;
    for (i=1; i < 21500; i++)
	glyphs[i] = glyphs[0];
    /* so check expanding the current 1-byte request for 2-byte glyphs */
    status = get_glyph (ctx, scaled_font, "μ", &glyphs[i]);
    if (status)
	goto BAIL;
    for (j=i+1; j < NUM_GLYPHS; j++)
	glyphs[j] = glyphs[i];

    cairo_show_glyphs (cr, glyphs, NUM_GLYPHS);

  BAIL:
    free(glyphs);

    return status;
}

CAIRO_TEST (show_glyphs_many,
	    "Test that cairo_show_glyphs works when handed 'many' glyphs",
	    "text, stress", /* keywords */
	    NULL, /* requirements */
	    9, 11,
	    NULL, draw)
