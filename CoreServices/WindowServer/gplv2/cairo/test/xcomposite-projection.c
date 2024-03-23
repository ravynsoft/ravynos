/*
 * Copyright 2009 Benjamin Otte
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Benjamin Otte <otte@gnome.org>
 */

#include "cairo-test.h"

/*
 * Exercise a bug in the projection of a rotated trapezoid mask.
 * I used CAIRO_ANTIALIAS_NONE and a single-color source in the test to get
 * rid of aliasing issues in the output images. This makes some issues
 * slightly less visible, but still fails for all of them. If you want to
 * get a clearer view:
 * #define ANTIALIAS CAIRO_ANTIALIAS_DEFAULT
 */

#define ANTIALIAS CAIRO_ANTIALIAS_NONE

static const char png_filename[] = "romedalen.png";

static cairo_pattern_t *
get_source (const cairo_test_context_t *ctx,
	    int *width, int *height)
{
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;

    if (ANTIALIAS == CAIRO_ANTIALIAS_NONE) {
	cairo_t *cr;

	surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 256, 192);
	cr = cairo_create (surface);

	cairo_set_source_rgb (cr, 0.75, 0.25, 0.25);
	cairo_paint (cr);

	pattern = cairo_pattern_create_for_surface (cairo_get_target (cr));
	cairo_destroy (cr);
    } else {
	surface = cairo_test_create_surface_from_png (ctx, png_filename);
	pattern = cairo_pattern_create_for_surface (surface);
    }

    *width = cairo_image_surface_get_width (surface);
    *height = cairo_image_surface_get_height (surface);
    cairo_surface_destroy (surface);

    return pattern;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *image;
    int img_width, img_height;

    image = get_source (cairo_test_get_context (cr),
			&img_width, &img_height);

    /* we don't want to debug antialiasing artifacts */
    cairo_set_antialias (cr, ANTIALIAS);

    /* dark grey background */
    cairo_set_source_rgb (cr, 0.25, 0.25, 0.25);
    cairo_paint (cr);

    /* magic transform */
    cairo_translate (cr, 10, -40);
    cairo_rotate (cr, -0.05);

    /* place the image on our surface */
    cairo_set_source (cr, image);

    /* paint the image */
    cairo_rectangle (cr, 0, 0, img_width, img_height);
    cairo_fill (cr);

    cairo_pattern_destroy (image);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (xcomposite_projection,
	    "Test a bug with XRenderComposite reference computation when projecting the first trapezoid onto 16.16 space",
	    "xlib", /* keywords */
	    "target=raster", /* requirements */
	    300, 150,
	    NULL, draw)
