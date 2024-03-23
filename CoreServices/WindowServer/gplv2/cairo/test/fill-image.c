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

#define PAD 10
#define SIZE 100
#define IMAGE_SIZE (SIZE-PAD*2)
#define LINE_WIDTH 10

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *image;
    cairo_t *cr_image;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
	                                IMAGE_SIZE, IMAGE_SIZE);
    cr_image = cairo_create (image);
    cairo_surface_destroy (image);

    /* Create the image */
    cairo_set_source_rgb (cr_image, 0, 0, 0);
    cairo_paint (cr_image);

    cairo_set_source_rgb (cr_image, 0, 1, 0);
    cairo_new_sub_path (cr_image);
    cairo_arc (cr_image, IMAGE_SIZE/2, IMAGE_SIZE/2, IMAGE_SIZE/2 - LINE_WIDTH, 0, M_PI * 2.0);
    cairo_close_path (cr_image);
    cairo_new_sub_path (cr_image);
    cairo_arc_negative (cr_image, IMAGE_SIZE/2, IMAGE_SIZE/2, IMAGE_SIZE/2, 0, -M_PI * 2.0);
    cairo_close_path (cr_image);
    cairo_fill (cr_image);

    /* Now stroke^Wfill with it */
    cairo_translate (cr, PAD, PAD);

    cairo_set_source_surface (cr, cairo_get_target (cr_image), 0, 0);
    cairo_destroy (cr_image);

    cairo_new_sub_path (cr);
    cairo_arc (cr, IMAGE_SIZE/2, IMAGE_SIZE/2, IMAGE_SIZE/2 - LINE_WIDTH, 0, M_PI * 2.0);
    cairo_close_path (cr);
    cairo_new_sub_path (cr);
    cairo_arc_negative (cr, IMAGE_SIZE/2, IMAGE_SIZE/2, IMAGE_SIZE/2, 0, -M_PI * 2.0);
    cairo_close_path (cr);

    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (fill_image,
	    "Test filling with an image source, with a non-identity CTM",
	    "fill, image, transform", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
