/*
 * Copyright © 2011 Uli Schlachter
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

#define WIDTH 3000
#define HEIGHT 3000

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *surface;
    cairo_surface_t *image;
    cairo_t *cr2;

    /* We use a similar surface to have way smaller ref images */
    surface = cairo_surface_create_similar (cairo_get_target (cr),
					    CAIRO_CONTENT_COLOR_ALPHA,
					    WIDTH, HEIGHT);

    /* First we have to defeat xcb's deferred clear */
    cr2 = cairo_create (surface);
    cairo_test_paint_checkered (cr2);
    cairo_destroy (cr2);

    /* Now we try to map this to an image. This will try to use shared memory
     * but fail the allocation, because of the huge surface. */
    image = cairo_surface_map_to_image (surface, NULL);
    cairo_surface_unmap_image (surface, image);

    /* Finally we make sure that errors aren't lost. */
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);
    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (xcb_huge_image_shm,
	    "Force a failed shm allocation when receiving an image",
	    "xcb", /* keywords */
	    NULL, /* requirements */
	    2, 2,
	    NULL, draw)
