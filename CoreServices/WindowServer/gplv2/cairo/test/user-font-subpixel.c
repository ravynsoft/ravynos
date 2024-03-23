/*
 * Copyright © 2022 Behdad Esfahbod
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
 * Contributor(s):
 *	Behdad Esfahbod <behdad@behdad.org>
 */

/* Test that user-fonts can subpixel positioning.
 */

#include "cairo-test.h"


#define BORDER 10
#define REPEAT 16
#define TEXT_SIZE 24
#define WIDTH  (TEXT_SIZE * REPEAT + 2*BORDER)
#define HEIGHT (TEXT_SIZE + 2*BORDER)


static cairo_status_t
test_scaled_font_render_glyph (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *metrics)
{
    cairo_rectangle (cr, 0, .45, 1., .1);
    cairo_fill (cr);
    return CAIRO_STATUS_SUCCESS;
}

static cairo_font_face_t *
_user_font_face_create (void)
{
    cairo_font_face_t *user_font_face;

    user_font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_render_glyph_func (user_font_face, test_scaled_font_render_glyph);

    return user_font_face;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_font_face_t *font_face;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    font_face = _user_font_face_create ();

    cairo_set_font_face (cr, font_face);
    cairo_font_face_destroy (font_face);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_font_size (cr, TEXT_SIZE);

    for (unsigned i = 0; i < REPEAT; i++)
    {
      cairo_move_to (cr, BORDER + TEXT_SIZE * i, BORDER + i * .1);
      cairo_show_text (cr, "-");
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (user_font_subpixel,
	    "Tests user font subpixel rendering",
	    "font, user-font", /* keywords */
	    "cairo >= 1.17.4", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
