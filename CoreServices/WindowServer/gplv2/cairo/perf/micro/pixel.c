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

/* Measure the overhead in setting a single pixel */

#include "cairo-perf.h"

#include <pixman.h>

static cairo_time_t
pixel_direct (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *surface, *image;
    uint32_t *data;
    int stride, bpp;

    surface = cairo_get_target (cr);
    image = cairo_surface_map_to_image (surface, NULL);
    data = (uint32_t *) cairo_image_surface_get_data (image);
    stride = cairo_image_surface_get_stride (image) / sizeof (uint32_t);

    switch (cairo_image_surface_get_format (image)) {
    default:
    case CAIRO_FORMAT_INVALID:
    case CAIRO_FORMAT_A1: bpp = 0; break;
    case CAIRO_FORMAT_A8: bpp = 8; break;
    case CAIRO_FORMAT_RGB16_565: bpp = 16; break;
    case CAIRO_FORMAT_RGB24:
    case CAIRO_FORMAT_RGB30:
    case CAIRO_FORMAT_ARGB32: bpp = 32; break;
    case CAIRO_FORMAT_RGB96F: bpp = 96; break;
    case CAIRO_FORMAT_RGBA128F: bpp = 128; break;
    }

    cairo_perf_timer_start ();

    while (loops--)
	pixman_fill (data, stride, bpp, 0, 0, 1, 1, -1);

    cairo_perf_timer_stop ();

    cairo_surface_unmap_image (surface, image);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
pixel_paint (cairo_t *cr, int width, int height, int loops)
{
    cairo_perf_timer_start ();

    while (loops--)
	cairo_paint (cr);

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
pixel_mask (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *mask;
    cairo_t *cr2;

    mask = cairo_surface_create_similar (cairo_get_target (cr),
					 CAIRO_CONTENT_ALPHA,
					 1, 1);
    cr2 = cairo_create (mask);
    cairo_set_source_rgb (cr2, 1,1,1);
    cairo_paint (cr2);
    cairo_destroy (cr2);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask_surface (cr, mask, 0, 0);

    cairo_perf_timer_stop ();

    cairo_surface_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
pixel_rectangle (cairo_t *cr, int width, int height, int loops)
{
    cairo_new_path (cr);
    cairo_rectangle (cr, 0, 0, 1, 1);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);
    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
pixel_subrectangle (cairo_t *cr, int width, int height, int loops)
{
    cairo_new_path (cr);
    cairo_rectangle (cr, 0.1, 0.1, .8, .8);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);
    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
pixel_triangle (cairo_t *cr, int width, int height, int loops)
{
    cairo_new_path (cr);
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, 1, 1);
    cairo_line_to (cr, 0, 1);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);
    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
pixel_circle (cairo_t *cr, int width, int height, int loops)
{
    cairo_new_path (cr);
    cairo_arc (cr, 0.5, 0.5, 0.5, 0, 2 * M_PI);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);
    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
pixel_stroke (cairo_t *cr, int width, int height, int loops)
{
    cairo_set_line_width (cr, 1.);
    cairo_new_path (cr);
    cairo_move_to (cr, 0, 0.5);
    cairo_line_to (cr, 1, 0.5);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);
    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
pixel_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "pixel", NULL);
}

void
pixel (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1., 1., 1.);

    cairo_perf_run (perf, "pixel-direct", pixel_direct, NULL);
    cairo_perf_run (perf, "pixel-paint", pixel_paint, NULL);
    cairo_perf_run (perf, "pixel-mask", pixel_mask, NULL);
    cairo_perf_run (perf, "pixel-rectangle", pixel_rectangle, NULL);
    cairo_perf_run (perf, "pixel-subrectangle", pixel_subrectangle, NULL);
    cairo_perf_run (perf, "pixel-triangle", pixel_triangle, NULL);
    cairo_perf_run (perf, "pixel-circle", pixel_circle, NULL);
    cairo_perf_run (perf, "pixel-stroke", pixel_stroke, NULL);
}

cairo_bool_t
a1_pixel_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "a1-pixel", NULL);
}

void
a1_pixel (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1., 1., 1.);
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    cairo_perf_run (perf, "a1-pixel-direct", pixel_direct, NULL);
    cairo_perf_run (perf, "a1-pixel-paint", pixel_paint, NULL);
    cairo_perf_run (perf, "a1-pixel-mask", pixel_mask, NULL);
    cairo_perf_run (perf, "a1-pixel-rectangle", pixel_rectangle, NULL);
    cairo_perf_run (perf, "a1-pixel-subrectangle", pixel_subrectangle, NULL);
    cairo_perf_run (perf, "a1-pixel-triangle", pixel_triangle, NULL);
    cairo_perf_run (perf, "a1-pixel-circle", pixel_circle, NULL);
    cairo_perf_run (perf, "a1-pixel-stroke", pixel_stroke, NULL);
}
