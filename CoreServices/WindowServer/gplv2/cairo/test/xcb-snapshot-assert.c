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
#include "cairo.h"

static cairo_surface_t *
create_image (int width, int height)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    /* Paint something random to the image */
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 0, 1, 1);
    cairo_paint (cr);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *image;

    /* Image has to have same geometry as xcb surface to be added as a snapshot */
    image = create_image (width, height);
    cairo_set_source_surface (cr, image, 0, 0);
    cairo_surface_destroy (image);

    /* This attaches the tested xcb surface as a snapshot */
    cairo_paint (cr);

    /* Now cairo is modifying a snapshot which fails an
     * assert in _cairo_surface_begin_modification */
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (xcb_snapshot_assert,
	    "Test a wrong _cairo_surface_attach_snapshot call",
	    "xcb", /* keywords */
	    NULL, /* requirements */
	    2, 2,
	    NULL, draw)
