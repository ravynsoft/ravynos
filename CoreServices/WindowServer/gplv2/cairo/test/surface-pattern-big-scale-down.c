/*
 * Copyright © 2006 Mozilla Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Mozilla Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Mozilla Corporation makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * MOZILLA CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL MOZILLA CORPORATION BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Vladimir Vukicevic <vladimir@pobox.com>
 */

#include "cairo-test.h"

#define SRC_WIDTH 2048
#define SRC_HEIGHT 32

static cairo_surface_t *
create_source_surface (int w, int h)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, SRC_WIDTH, SRC_HEIGHT);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_rectangle (cr, 0, 0, w/2, h/2);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
    cairo_rectangle (cr, w/2, 0, w/2, h/2);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
    cairo_rectangle (cr, 0, h/2, w/2, h/2);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 1.0, 1.0, 0.0);
    cairo_rectangle (cr, w/2, h/2, w/2, h/2);
    cairo_fill (cr);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static void
draw_n (cairo_t *cr, cairo_pattern_t *pat, double dest_size, int n)
{
  cairo_matrix_t mat;

  cairo_matrix_init_scale (&mat, SRC_WIDTH / dest_size, SRC_HEIGHT / dest_size);
  cairo_matrix_translate (&mat, n * -dest_size, 0.0);
  cairo_pattern_set_matrix (pat, &mat);

  cairo_set_source (cr, pat);
  cairo_new_path (cr);
  cairo_rectangle (cr, n * dest_size, 0.0, dest_size, dest_size);
  cairo_fill (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *surface;
    cairo_pattern_t *pat;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    surface = create_source_surface (SRC_WIDTH, SRC_HEIGHT);

    pat = cairo_pattern_create_for_surface (surface);
    cairo_surface_destroy (surface);

    /* We want to draw at a position such that n * SRC_WIDTH * (SRC_WIDTH/16.0) > 32768.
     * x = n * 16.
     *
     * To show the bug, we want to draw on either side of the boundary;
     * in our case here, n = 16 results in 32768, and n = 17 results in > 32768.
     *
     * Drawing at 16 and 17 is sufficient to show the problem.
     */

#if 1
    /* n = 16 */
    draw_n (cr, pat, 16.0, 16);

    /* n = 17 */
    draw_n (cr, pat, 16.0, 17);
#else
    {
	int n;
	for (n = 0; n < 32; n++)
	    draw_n (cr, pat, 16.0, n);
    }
#endif

    cairo_pattern_destroy (pat);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (surface_pattern_big_scale_down,
	    "Test scaled-down transformed not-repeated surface patterns with large images and offsets",
	    "transform", /* keywords */
	    NULL, /* requirements */
	    512, 16,
	    NULL, draw)
