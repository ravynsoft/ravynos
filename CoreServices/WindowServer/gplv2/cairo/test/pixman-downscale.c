/*
 * Copyright © 2013 Samsung Electronics
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
 * Author: Bryce Harrington <b.harrington@samsung.com>
 */

/* This test exercises scaling a png image to smaller pixel dimensions
 *
 * Currently, this exercises several of pixman's scaling filters.
 */

#include "cairo-test.h"

#include <stdio.h>
#include <stdlib.h>

#include <cairo.h>

static const char png_filename[] = "quad-color.png";

/* Draw an image scaled down, with antialiasing disabled */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height, cairo_filter_t filter)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *image;
    double x_scale, y_scale, scale;

    cairo_set_source_rgb (cr, 1, 1, 1);
    image = cairo_test_create_surface_from_png (ctx, png_filename);
    x_scale = width * 1.0 / cairo_image_surface_get_width (image);
    y_scale = height * 1.0 / cairo_image_surface_get_height (image);
    scale = x_scale < y_scale ? x_scale : y_scale;

    cairo_save (cr);
    cairo_scale (cr, scale, scale);
    cairo_set_source_surface (cr, image, 0, 0);
    cairo_pattern_set_filter (cairo_get_source (cr), filter);
    cairo_paint (cr);
    cairo_restore (cr);
    cairo_surface_destroy (image);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_fast (cairo_t *cr, int width, int height)
{
  return draw (cr, width, height, CAIRO_FILTER_FAST);
}

static cairo_test_status_t
draw_good (cairo_t *cr, int width, int height)
{
  return draw (cr, width, height, CAIRO_FILTER_GOOD);
}

static cairo_test_status_t
draw_best (cairo_t *cr, int width, int height)
{
  return draw (cr, width, height, CAIRO_FILTER_BEST);
}

static cairo_test_status_t
draw_nearest (cairo_t *cr, int width, int height)
{
  return draw (cr, width, height, CAIRO_FILTER_NEAREST);
}

static cairo_test_status_t
draw_bilinear (cairo_t *cr, int width, int height)
{
  return draw (cr, width, height, CAIRO_FILTER_BILINEAR);
}

CAIRO_TEST (pixman_downscale_fast_96,
	    "Tests scaling to equivalent size using the fast filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    96, 96,
	    NULL, draw_fast)

CAIRO_TEST (pixman_downscale_fast_95,
	    "Tests downscaling by a single pixel using the fast filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    95, 95,
	    NULL, draw_fast)

CAIRO_TEST (pixman_downscale_fast_24,
	    "Tests downscaling by an even multiple using the fast filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    24, 24,
	    NULL, draw_fast)


CAIRO_TEST (pixman_downscale_good_96,
	    "Tests scaling to equivalent size using the good filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    96, 96,
	    NULL, draw_good)

CAIRO_TEST (pixman_downscale_good_95,
	    "Tests downscaling by a single pixel using the good filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    95, 95,
	    NULL, draw_good)

CAIRO_TEST (pixman_downscale_good_24,
	    "Tests downscaling by an even multiple using the good filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    24, 24,
	    NULL, draw_good)


CAIRO_TEST (pixman_downscale_best_96,
	    "Tests scaling to equivalent size using the best filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    96, 96,
	    NULL, draw_best)

CAIRO_TEST (pixman_downscale_best_95,
	    "Tests downscaling by a single pixel using the best filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    95, 95,
	    NULL, draw_best)

CAIRO_TEST (pixman_downscale_best_24,
	    "Tests downscaling by an even multiple using the best filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    24, 24,
	    NULL, draw_best)


CAIRO_TEST (pixman_downscale_nearest_96,
	    "Tests scaling to equivalent size using the nearest filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    96, 96,
	    NULL, draw_nearest)

CAIRO_TEST (pixman_downscale_nearest_95,
	    "Tests downscaling by a single pixel using the nearest filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    95, 95,
	    NULL, draw_nearest)

CAIRO_TEST (pixman_downscale_nearest_24,
	    "Tests downscaling by an even multiple using the nearest filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    24, 24,
	    NULL, draw_nearest)


CAIRO_TEST (pixman_downscale_bilinear_96,
	    "Tests scaling to equivalent size using the bilinear filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    96, 96,
	    NULL, draw_bilinear)

CAIRO_TEST (pixman_downscale_bilinear_95,
	    "Tests downscaling by a single pixel using the bilinear filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    95, 95,
	    NULL, draw_bilinear)

CAIRO_TEST (pixman_downscale_bilinear_24,
	    "Tests downscaling by an even multiple using the bilinear filter",
	    "image, transform, raster, downscale", /* keywords */
	    NULL, /* requirements */
	    24, 24,
	    NULL, draw_bilinear)
