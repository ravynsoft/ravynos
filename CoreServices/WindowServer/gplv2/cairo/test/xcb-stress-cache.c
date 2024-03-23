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

#define WIDTH 512
#define HEIGHT 512

/* This is a random pick. This results in 512 (width) * 512 (height) *
 * 2 (surfaces per run) * 4 (ARGB32) * ITERATIONS = 200 MiB of image data. */
#define ITERATIONS 100

/* This tries to trigger a bug in the xcb backend where a Picture is freed to
 * early. It goes something like this:
 *
 * - _composite_mask calls _cairo_xcb_picture_for_pattern to get a xcb_picture_t
 *   for the source.
 * - _cairo_xcb_picture_for_pattern calls _cairo_xcb_surface_picture which calls
 *   _cairo_xcb_screen_store_surface_picture which adds the picture to a cache.
 * - _cairo_xcb_surface_picture also attached the picture as a snapshot to
 *   the source surface using cairo_surface_finish as detach_func.
 * - _composite_mask calls _cairo_xcb_picture_for_pattern to get a xcb_picture_t
 *   for the mask.
 * - The resulting picture surface is added to the cache again, but the cache is
 *   already full, so a random cache entry is picked and removed.
 * - The surface that was added before is picked and gets fed to
 *   _surface_cache_entry_destroy.
 * - This calls _cairo_surface_detach_snapshot which causes the
 *   detach_func from above to be called, so the surface is finished and the
 *   associated picture is FreePicture'd.
 * - _composite_mask now uses a Picture that was already freed.
 *
 * So this depends on the screen's surface cache to be full which is why we do
 * all this looping.
 */

static cairo_surface_t *
create_image (void)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
    /* Paint something random to the image */
    cr = cairo_create (surface);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 1, 1);
    cairo_rectangle (cr, 0, 0, WIDTH/2.0, HEIGHT/2.0);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 1, 0, 1);
    cairo_rectangle (cr, WIDTH/2.0, HEIGHT/2.0, WIDTH/2.0, HEIGHT/2.0);
    cairo_fill (cr);
    cairo_destroy (cr);

    return surface;
}

static cairo_surface_t *
dirty_cache (cairo_t *cr)
{
    cairo_surface_t *surface;

    /* Set a source surface... */
    surface = create_image ();
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_surface_destroy (surface);
    /* ...and create a mask surface, so that we can hit the early FreePicture */
    surface = create_image ();
    cairo_mask_surface (cr, surface, 0, 0);

    return surface;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;
    cairo_surface_t *array[ITERATIONS];

    /* We have to keep the associated cairo_surface_t alive so that they aren't
     * removed from the cache */
    for (i = 0; i < ITERATIONS; i++)
	array[i] = dirty_cache (cr);
    for (i = 0; i < ITERATIONS; i++)
	cairo_surface_destroy (array[i]);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (xcb_stress_cache,
	    "Stress test for a image surface cache in cairo-xcb",
	    "xcb, stress", /* keywords */
	    NULL, /* requirements */
	    2, 2,
	    NULL, draw)
