/*
 * Copyright © 2012 Uli Schlachter
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
 * Author: Uli Schlachter <psychon@znc.in>
 */

#include "cairo-test.h"

#define WIDTH 6000
#define HEIGHT 6000

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *surface;
    cairo_surface_t *image;
    cairo_surface_t *subimage;
    cairo_rectangle_int_t extents;
    cairo_t *cr2;

    extents.x = extents.y = 10;
    extents.width = WIDTH - 20;
    extents.height = HEIGHT - 20;

    /* We use a similar surface to have way smaller ref images */
    surface = cairo_surface_create_similar (cairo_get_target (cr),
					    CAIRO_CONTENT_COLOR_ALPHA,
					    WIDTH, HEIGHT);

    /* First we have to defeat xcb's deferred clear */
    cr2 = cairo_create (surface);
    cairo_test_paint_checkered (cr2);
    cairo_destroy (cr2);

    /* Get us an image surface with a non-natural stride */
    image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					WIDTH, HEIGHT);
    subimage = cairo_surface_map_to_image (image, &extents);

    /* Paint the subimage to the similar surface and trigger the big upload */
    cr2 = cairo_create (surface);
    cairo_set_source_surface (cr2, subimage, 0, 0);
    cairo_paint (cr2);
    cairo_destroy (cr2);

    /* Finally we make sure that errors aren't lost. */
    cairo_surface_unmap_image (image, subimage);
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);
    cairo_surface_destroy (image);
    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (xcb_huge_subimage,
	    "Test if the maximum request size is honored",
	    "xcb", /* keywords */
	    NULL, /* requirements */
	    2, 2,
	    NULL, draw)
