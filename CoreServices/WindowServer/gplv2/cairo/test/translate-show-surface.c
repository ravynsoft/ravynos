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

/* Bug history
 *
 * 2005-04-11 Carl Worth <cworth@cworth.org>
 *
 *   It appears that calling cairo_show_surface after cairo_translate
 *   somehow applies the translation twice to the surface being
 *   shown. This is pretty easy to demonstrate by bringing up xsvg on
 *   an SVG file with an <image> and panning around a bit with the
 *   arrow keys.
 *
 *   This is almost certainly a regression, and I suspect there may be
 *   some interaction with the fix for move-to-show-surface.
 *
 * 2005-04-12 Carl Worth <cworth@cworth.org>
 *
 *   I committed a fix for this bug today.
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *surface;
    uint32_t colors[4] = {
	0xffffffff, 0xffff0000,
	0xff00ff00, 0xff0000ff
    };
    int i;

    for (i=0; i < 4; i++) {
	surface = cairo_image_surface_create_for_data ((unsigned char *) &colors[i],
						       CAIRO_FORMAT_RGB24,
						       1, 1, 4);
	cairo_save (cr);
	{
	    cairo_translate (cr, i % 2, i / 2);
	    cairo_set_source_surface (cr, surface, 0, 0);
	    cairo_paint (cr);
	}
	cairo_restore (cr);
	cairo_surface_finish (surface); /* colors will go out of scope */
	cairo_surface_destroy (surface);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (translate_show_surface,
	    "Tests calls to cairo_show_surface after cairo_translate",
	    "transform", /* keywords */
	    NULL, /* requirements */
	    2, 2,
	    NULL, draw)
