/*
 * Copyright © 2011 Intel Corporation
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
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#include <stdio.h>
#include <errno.h>

/* Basic test to exercise the new mime-surface callback. */

#define WIDTH 200
#define HEIGHT 80

static char *png_filename = NULL;

/* Lazy way of determining PNG dimensions... */
static void
png_dimensions (const char *filename,
		cairo_content_t *content, int *width, int *height)
{
    cairo_surface_t *surface;

    surface = cairo_image_surface_create_from_png (filename);
    *content = cairo_surface_get_content (surface);
    *width = cairo_image_surface_get_width (surface);
    *height = cairo_image_surface_get_height (surface);
    cairo_surface_destroy (surface);
}

static cairo_surface_t *
png_acquire (cairo_pattern_t *pattern, void *closure,
	     cairo_surface_t *target,
	     const cairo_rectangle_int_t *extents)
{
    return cairo_image_surface_create_from_png (closure);
}

static cairo_surface_t *
red_acquire (cairo_pattern_t *pattern, void *closure,
	     cairo_surface_t *target,
	     const cairo_rectangle_int_t *extents)
{
    cairo_surface_t *image;
    cairo_t *cr;

    image = cairo_surface_create_similar_image (target,
						CAIRO_FORMAT_RGB24,
						extents->width,
						extents->height);
    cairo_surface_set_device_offset (image, extents->x, extents->y);

    cr = cairo_create (image);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);

    return image;
}

static void
release (cairo_pattern_t *pattern, void *closure, cairo_surface_t *image)
{
    cairo_surface_destroy (image);
}

static void
free_filename(void)
{
    free (png_filename);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *png, *red;
    cairo_content_t content;
    int png_width, png_height;
    int i, j;

    if (png_filename == NULL) {
      const cairo_test_context_t *ctx = cairo_test_get_context (cr);
      xasprintf (&png_filename, "%s/png.png", ctx->srcdir);
      atexit (free_filename);
    }

    png_dimensions (png_filename, &content, &png_width, &png_height);

    png = cairo_pattern_create_raster_source ((void*)png_filename,
					      content, png_width, png_height);
    cairo_raster_source_pattern_set_acquire (png, png_acquire, release);

    red = cairo_pattern_create_raster_source (NULL,
					      CAIRO_CONTENT_COLOR, WIDTH, HEIGHT);
    cairo_raster_source_pattern_set_acquire (red, red_acquire, release);

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_paint (cr);

    cairo_translate (cr, 0, (HEIGHT-png_height)/2);
    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++) {
	    cairo_pattern_t *source;
	    if ((i ^ j) & 1)
		source = red;
	    else
		source = png;
	    cairo_set_source (cr, source);
	    cairo_rectangle (cr, i * WIDTH/4, j * png_height/4, WIDTH/4, png_height/4);
	    cairo_fill (cr);
	}
    }

    cairo_pattern_destroy (red);
    cairo_pattern_destroy (png);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (raster_source,
	    "Check that the mime-surface embedding works",
	    "api", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
