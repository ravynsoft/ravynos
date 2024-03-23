/*
 * Copyright © Jeff Muizelaar
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * JEFF MUIZELAAR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Jeff Muizelaar <jeff@infidigm.net>
 *	    Carl Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define MASK_WIDTH 8
#define MASK_HEIGHT 8

static unsigned char mask[MASK_WIDTH * MASK_HEIGHT] = {
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xff, 0x0, 0xff, 0x0, 0x0, 0x0,
};

static cairo_test_status_t
check_status (const cairo_test_context_t *ctx,
	      cairo_status_t status,
	      cairo_status_t expected)
{
    if (status == expected)
	return CAIRO_TEST_SUCCESS;

    cairo_test_log (ctx,
		    "Error: Expected status value %d (%s), received %d (%s)\n",
		    expected,
		    cairo_status_to_string (expected),
		    status,
		    cairo_status_to_string (status));
    return CAIRO_TEST_FAILURE;
}

static cairo_test_status_t
test_surface_with_width_and_stride (const cairo_test_context_t *ctx,
				    int width, int stride,
				    cairo_status_t expected)
{
    cairo_test_status_t status;
    cairo_surface_t *surface;
    cairo_t *cr;
    int len;
    unsigned char *data;

    cairo_test_log (ctx,
		    "Creating surface with width %d and stride %d\n",
		    width, stride);

    len = stride;
    if (len < 0)
	len = -len;
    data = xmalloc (len);

    surface = cairo_image_surface_create_for_data (data, CAIRO_FORMAT_A8,
						   width, 1, stride);
    cr = cairo_create (surface);

    cairo_paint (cr);

    status = check_status (ctx, cairo_surface_status (surface), expected);
    if (status)
	goto BAIL;

    status = check_status (ctx, cairo_status (cr), expected);
    if (status)
	goto BAIL;

  BAIL:
    cairo_destroy (cr);
    cairo_surface_destroy (surface);
    free (data);
    return status;
}

static cairo_test_status_t
draw (cairo_t *cr, int dst_width, int dst_height)
{
    int stride, row;
    unsigned char *src, *dst, *mask_aligned;
    cairo_surface_t *surface;

    /* Now test actually drawing through our mask data, allocating and
     * copying with the proper stride. */
    stride = cairo_format_stride_for_width (CAIRO_FORMAT_A8,
					    MASK_WIDTH);

    mask_aligned = xmalloc (stride * MASK_HEIGHT);

    src = mask;
    dst = mask_aligned;
    for (row = 0; row < MASK_HEIGHT; row++) {
	memcpy (dst, src, MASK_WIDTH);
	src += MASK_WIDTH;
	dst += stride;
    }

    surface = cairo_image_surface_create_for_data (mask_aligned,
						   CAIRO_FORMAT_A8,
						   MASK_WIDTH,
						   MASK_HEIGHT,
						   stride);

    /* Paint background blue */
    cairo_set_source_rgb (cr, 0, 0, 1); /* blue */
    cairo_paint (cr);

    /* Then paint red through our mask */
    cairo_set_source_rgb (cr, 1, 0, 0); /* red */
    cairo_mask_surface (cr, surface, 0, 0);
    cairo_surface_destroy (surface);

    free (mask_aligned);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_test_status_t status = CAIRO_TEST_SUCCESS;
    int test_width;

    for (test_width = 0; test_width < 40; test_width++) {
	int stride = cairo_format_stride_for_width (CAIRO_FORMAT_A8,
						test_width);
	cairo_status_t expected;

	/* First create a surface using the width as the stride,
	 * (most of these should fail).
	 */
	expected = (stride == test_width) ?
	    CAIRO_STATUS_SUCCESS : CAIRO_STATUS_INVALID_STRIDE;

	status = test_surface_with_width_and_stride (ctx,
						     test_width,
						     test_width,
						     expected);
	if (status)
	    return status;

	status = test_surface_with_width_and_stride (ctx,
						     test_width,
						     -test_width,
						     expected);
	if (status)
	    return status;


	/* Then create a surface using the correct stride,
	 * (should always succeed).
	 */
	status = test_surface_with_width_and_stride (ctx,
						     test_width,
						     stride,
						     CAIRO_STATUS_SUCCESS);
	if (status)
	    return status;

	status = test_surface_with_width_and_stride (ctx,
						     test_width,
						     -stride,
						     CAIRO_STATUS_SUCCESS);
	if (status)
	    return status;
    }

    return status;
}

CAIRO_TEST (a8_mask,
	    "test masks of CAIRO_FORMAT_A8",
	    "alpha, mask", /* keywords */
	    NULL, /* requirements */
	    8, 8,
	    preamble, draw)
