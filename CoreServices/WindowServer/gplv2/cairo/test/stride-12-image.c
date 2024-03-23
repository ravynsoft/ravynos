/*
 * Copyright 2012 Andrea Canciani
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
 * Author: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

static const char *png_filename = "romedalen.png";

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_format_t format = CAIRO_FORMAT_ARGB32;
    cairo_t *cr_src;
    cairo_surface_t *png, *src;
    uint8_t *data;
    int stride;

    png = cairo_test_create_surface_from_png (ctx, png_filename);

    stride = cairo_format_stride_for_width (format, width) + 12;
    data = xcalloc (stride, height);
    src = cairo_image_surface_create_for_data (data, format,
					       width, height, stride);

    cr_src = cairo_create (src);
    cairo_set_source_surface (cr_src, png, 0, 0);
    cairo_paint (cr_src);
    cairo_destroy (cr_src);

    cairo_set_source_surface (cr, src, 0, 0);
    cairo_paint (cr);

    cairo_surface_destroy (png);

    cairo_surface_finish (src);
    cairo_surface_destroy (src);

    free (data);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (stride_12_image,
	    "Test that images with a non-default stride are handled correctly.",
	    "stride, image", /* keywords */
	    NULL, /* requirements */
	    256, 192,
	    NULL, draw)
