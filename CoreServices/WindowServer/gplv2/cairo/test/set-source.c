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

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;
    uint32_t color = 0x8019334c;
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;

    surface = cairo_image_surface_create_for_data ((unsigned char *) &color,
						   CAIRO_FORMAT_ARGB32, 1, 1, 4);
    pattern = cairo_pattern_create_for_surface (surface);
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);

    /* Several different means of making mostly the same color (though
     * we can't get anything but alpha==1.0 out of
     * cairo_set_source_rgb. */
    for (i=0; i < width; i++) {
	switch (i) {
	case 0:
	    cairo_set_source_rgb (cr, .6, .7, .8);
	    break;
	case 1:
	    cairo_set_source_rgba (cr, .2, .4, .6, 0.5);
	    break;
	case 2:
#if WE_HAD_SUPPORT_FOR_PREMULTIPLIED
	    cairo_set_source_rgba_premultiplied (cr, .1, .2, .3, 0.5);
#else
	    cairo_set_source_rgba (cr, .2, .4, .6, 0.5);
#endif
	    break;
	case 3:
	default:
	    cairo_set_source (cr, pattern);
	    break;
	}

	cairo_rectangle (cr, i, 0, 1, height);
	cairo_fill (cr);
    }

    cairo_pattern_destroy (pattern);
    cairo_surface_finish (surface); /* data will go out of scope */
    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (set_source,
	    "Tests calls to various set_source functions",
	    "api", /* keywords */
	    NULL, /* requirements */
	    5, 5,
	    NULL, draw)
