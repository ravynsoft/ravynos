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

/* Compares the overhead for WebKit's drawRect() */

#include "cairo-perf.h"

#include <pixman.h>

static cairo_time_t
clip_paint (cairo_t *cr, int width, int height, int loops)
{
    int x = width/4, w = width/2;
    int y = height/4, h = height/2;

    cairo_perf_timer_start ();

    while (loops--) {
	cairo_reset_clip (cr);
	cairo_rectangle (cr, x, y, w, h);
	cairo_clip (cr);
	cairo_paint (cr);
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
rect_fill (cairo_t *cr, int width, int height, int loops)
{
    int x = width/4, w = width/2;
    int y = height/4, h = height/2;

    cairo_perf_timer_start ();

    while (loops--) {
	cairo_rectangle (cr, x, y, w, h);
	cairo_fill (cr);
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
direct (cairo_t *cr, int width, int height, int loops)
{
    int x = width/4, w = width/2;
    int y = height/4, h = height/2;
    cairo_surface_t *surface, *image;
    uint8_t *data;
    int stride, bpp;


    surface = cairo_get_target (cr);
    image = cairo_surface_map_to_image (surface, NULL);
    data = cairo_image_surface_get_data (image);
    stride = cairo_image_surface_get_stride (image);

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

    while (loops--) {
	pixman_fill ((uint32_t *)data, stride / sizeof(uint32_t), bpp,
		     x, y, w, h,
		     -1);
    }

    cairo_perf_timer_stop ();

    cairo_surface_unmap_image (surface, image);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
fill_clip_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "fillclip", NULL);
}

void
fill_clip (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1., 1., 1.);

    cairo_perf_run (perf, "fillclip-clip", clip_paint, NULL);
    cairo_perf_run (perf, "fillclip-fill", rect_fill, NULL);
    cairo_perf_run (perf, "fillclip-direct", direct, NULL);
}
