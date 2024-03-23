/*
 * Copyright 2008 Kai-Uwe Behrmann
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Kai-Uwe Behrmann not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Kai-Uwe Behrmann makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * KAI_UWE BEHRMANN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL KAI_UWE BEHRMANN BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Kai-Uwe Behrmann <ku.b@gmx.de>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

static const char png_filename[] = "romedalen.png";

static cairo_surface_t *
create_mask (cairo_t *dst, int width, int height)
{
    cairo_surface_t *mask;
    cairo_t *cr;

    mask = cairo_image_surface_create (CAIRO_FORMAT_A8, width, height);
    cr = cairo_create (mask);
    cairo_surface_destroy (mask);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_rectangle (cr, width/4, height/4, width/2, height/2);
    cairo_fill (cr);

    mask = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return mask;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *image, *mask;

    image = cairo_test_create_surface_from_png (ctx, png_filename);
    mask = create_mask (cr, 40, 40);

    /* opaque background */
    cairo_paint (cr);

    /* center */
    cairo_translate (cr,
	             (width - cairo_image_surface_get_width (image)) / 2.,
		     (height - cairo_image_surface_get_height (image)) / 2.);

    /* rotate 30 degree around the center */
    cairo_translate (cr, width/2., height/2.);
    cairo_rotate (cr, -30 * 2 * M_PI / 360);
    cairo_translate (cr, -width/2., -height/2.);

    /* place the image on our surface */
    cairo_set_source_surface (cr, image, 0, 0);

    /* reset the drawing matrix */
    cairo_identity_matrix (cr);

    /* fill nicely */
    cairo_scale (cr, width / 40., height / 40.);

    /* apply the mask */
    cairo_mask_surface (cr, mask, 0, 0);

    cairo_surface_destroy (mask);
    cairo_surface_destroy (image);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (mask_transformed_image,
	    "Test that cairo_mask() is affected properly by the CTM and not the image",
	    "mask", /* keywords */
	    NULL, /* requirements */
	    80, 80,
	    NULL, draw)
