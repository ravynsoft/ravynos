/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2021 Manuel Stoeckl
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
 * Author: Carl Worth <cworth@cworth.org>
 * Author: Manuel Stoeckl <code@mstoeckl.com>
 */

#include "cairo-test.h"

#include <stdlib.h>

#define WIDTH 2
#define HEIGHT 2

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    char *filename;
    cairo_surface_t *surface;

    xasprintf (&filename, "%s/reference/%s",
	       ctx->srcdir, "create-from-png-16bit.base.png");

    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	cairo_test_status_t result;

	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}

	free (filename);
	return result;
    }

    /* Pretend we modify the surface data (which detaches the PNG mime data) */
    cairo_surface_flush (surface);
    cairo_surface_mark_dirty (surface);

    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
    cairo_paint (cr);

    cairo_surface_destroy (surface);

    free (filename);
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (create_from_png_16bit,
	    "Tests the creation of an image surface from a 16 bit PNG file",
	    "png", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
