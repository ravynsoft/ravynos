/* imagediff - Compare two images
 *
 * Copyright © 2004 Richard D. Worth
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Richard Worth
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * Richard Worth makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * RICHARD WORTH DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL RICHARD WORTH BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Richard D. Worth <richard@theworths.org> */

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <string.h>
#include <pixman.h>

#include "cairo-test.h"

#include "pdiff.h"
#include "buffer-diff.h"

/* Don't allow any differences greater than this value, even if pdiff
 * claims that the images are identical */
#define PERCEPTUAL_DIFF_THRESHOLD 25

/* Compare two buffers, returning the number of pixels that are
 * different and the maximum difference of any single color channel in
 * result_ret.
 *
 * This function should be rewritten to compare all formats supported by
 * cairo_format_t.
 */
static void
buffer_diff_core (const unsigned char *_buf_a, int stride_a, cairo_format_t format_a,
		  const unsigned char *_buf_b, int stride_b, cairo_format_t format_b,
		  unsigned char *_buf_diff, int stride_diff, cairo_format_t format_diff,
		  int		width,
		  int		height,
		  buffer_diff_result_t *result_ret)
{
    const uint32_t *buf_a = (const uint32_t*) _buf_a;
    const uint32_t *buf_b = (const uint32_t*) _buf_b;
    uint32_t *buf_diff = (uint32_t*) _buf_diff;
    int x, y;
    buffer_diff_result_t result = {0, 0};

    assert (format_a == CAIRO_FORMAT_RGB24 || format_a == CAIRO_FORMAT_ARGB32);
    assert (format_b == CAIRO_FORMAT_RGB24 || format_b == CAIRO_FORMAT_ARGB32);
    assert (format_diff == CAIRO_FORMAT_RGB24 || format_diff == CAIRO_FORMAT_ARGB32);

    stride_a /= sizeof (uint32_t);
    stride_b /= sizeof (uint32_t);
    stride_diff /= sizeof (uint32_t);
    for (y = 0; y < height; y++) {
	const uint32_t *row_a = buf_a + y * stride_a;
	const uint32_t *row_b = buf_b + y * stride_b;
	uint32_t *row = buf_diff + y * stride_diff;

	for (x = 0; x < width; x++) {
	    uint32_t pixel_a = row_a[x];
	    uint32_t pixel_b = row_b[x];
	    /* convert pixel data to ARGB32 if necessary */
	    if (format_a == CAIRO_FORMAT_RGB24)
		pixel_a |= 0xff000000;
	    if (format_b == CAIRO_FORMAT_RGB24)
		pixel_b |= 0xff000000;

	    /* check if the pixels are the same */
	    if (pixel_a != pixel_b) {
		int channel;
		uint32_t diff_pixel = 0;

		/* calculate a difference value for all 4 channels */
		for (channel = 0; channel < 4; channel++) {
		    int value_a = (pixel_a >> (channel*8)) & 0xff;
		    int value_b = (pixel_b >> (channel*8)) & 0xff;
		    unsigned int diff;
		    diff = abs (value_a - value_b);
		    if (diff > result.max_diff)
			result.max_diff = diff;
		    diff *= 4;  /* emphasize */
		    if (diff)
			diff += 128; /* make sure it's visible */
		    if (diff > 255)
			diff = 255;
		    diff_pixel |= diff << (channel*8);
		}

		result.pixels_changed++;
		if ((diff_pixel & 0x00ffffff) == 0) {
		    /* alpha only difference, convert to luminance */
		    uint8_t alpha = diff_pixel >> 24;
		    diff_pixel = alpha * 0x010101;
		}
		row[x] = diff_pixel;
	    } else {
		row[x] = 0;
	    }
	    row[x] |= 0xff000000; /* Set ALPHA to 100% (opaque) */
	}
    }

    *result_ret = result;
}

/* Compares two image surfaces
 *
 * Provides number of pixels changed and maximum single-channel
 * difference in result.
 *
 * Also fills in a "diff" surface intended to visually show where the
 * images differ.
 */
static void
compare_surfaces (const cairo_test_context_t  *ctx,
	          cairo_surface_t	*surface_a,
		  cairo_surface_t	*surface_b,
		  cairo_surface_t	*surface_diff,
		  buffer_diff_result_t	*result)
{
    /* These default values were taken straight from the
     * perceptualdiff program. We'll probably want to tune these as
     * necessary. */
    double gamma = 2.2;
    double luminance = 100.0;
    double field_of_view = 45.0;
    int discernible_pixels_changed;

    /* First, we run cairo's old buffer_diff algorithm which looks for
     * pixel-perfect images, (we do this first since the test suite
     * runs about 3x slower if we run pdiff_compare first).
     */
    buffer_diff_core (cairo_image_surface_get_data (surface_a),
		      cairo_image_surface_get_stride (surface_a),
		      cairo_image_surface_get_format (surface_a),
		      cairo_image_surface_get_data (surface_b),
		      cairo_image_surface_get_stride (surface_b),
		      cairo_image_surface_get_format (surface_b),
		      cairo_image_surface_get_data (surface_diff),
		      cairo_image_surface_get_stride (surface_diff),
		      cairo_image_surface_get_format (surface_diff),
		      cairo_image_surface_get_width (surface_a),
		      cairo_image_surface_get_height (surface_a),
		      result);
    if (result->pixels_changed == 0)
	return;

    cairo_test_log (ctx,
	            "%d pixels differ (with maximum difference of %d) from reference image\n",
		    result->pixels_changed, result->max_diff);

    /* Then, if there are any different pixels, we give the pdiff code
     * a crack at the images. If it decides that there are no visually
     * discernible differences in any pixels, then we accept this
     * result as good enough.
     *
     * Only let pdiff have a crack at the comparison if the max difference
     * is lower than a threshold, otherwise some problems could be masked.
     */
    if (result->max_diff < PERCEPTUAL_DIFF_THRESHOLD) {
        discernible_pixels_changed = pdiff_compare (surface_a, surface_b,
                                                    gamma, luminance, field_of_view);
        if (discernible_pixels_changed == 0) {
            result->pixels_changed = 0;
            cairo_test_log (ctx,
		            "But perceptual diff finds no visually discernible difference.\n"
                            "Accepting result.\n");
        }
    }
}

void
buffer_diff_noalpha (const unsigned char *buf_a,
		     const unsigned char *buf_b,
		     unsigned char *buf_diff,
		     int	   width,
		     int	   height,
		     int	   stride,
		     buffer_diff_result_t *result)
{
    buffer_diff_core(buf_a, stride, CAIRO_FORMAT_RGB24,
		     buf_b, stride, CAIRO_FORMAT_RGB24,
		     buf_diff, stride, CAIRO_FORMAT_RGB24,
		     width, height,
		     result);
}

static cairo_bool_t
same_size (cairo_surface_t *a, cairo_surface_t *b)
{
    unsigned int width_a, height_a;
    unsigned int width_b, height_b;

    width_a = cairo_image_surface_get_width (a);
    height_a = cairo_image_surface_get_height (a);

    width_b = cairo_image_surface_get_width (b);
    height_b = cairo_image_surface_get_height (b);

    return width_a == width_b && height_a == height_b;
}

/* Image comparison code courtesy of Richard Worth <richard@theworths.org>
 * Returns number of pixels changed, (or -1 on error).
 * Also saves a "diff" image intended to visually show where the
 * images differ.
 *
 * The return value simply indicates whether a check was successfully
 * made, (as opposed to a file-not-found condition or similar). It
 * does not indicate anything about how much the images differ. For
 * that, see result.
 *
 * One failure mode is if the two images provided do not have the same
 * dimensions. In this case, this function will return
 * CAIRO_STATUS_SURFACE_TYPE_MISMATCH (which is a bit of an abuse, but
 * oh well).
 */
cairo_status_t
image_diff (const cairo_test_context_t *ctx,
	    cairo_surface_t *surface_a,
	    cairo_surface_t *surface_b,
	    cairo_surface_t *surface_diff,
	    buffer_diff_result_t *result)
{
    if (cairo_surface_status (surface_a))
	return cairo_surface_status (surface_a);

    if (cairo_surface_status (surface_b))
	return cairo_surface_status (surface_b);

    if (cairo_surface_status (surface_diff))
	return cairo_surface_status (surface_diff);

    if (! same_size (surface_a, surface_b) ||
	! same_size (surface_a, surface_diff))
    {
	cairo_test_log (ctx, "Error: Image size mismatch\n");
	return CAIRO_STATUS_SURFACE_TYPE_MISMATCH;
    }

    compare_surfaces (ctx, surface_a, surface_b, surface_diff, result);

    return CAIRO_STATUS_SUCCESS;
}

cairo_bool_t
image_diff_is_failure (const buffer_diff_result_t *result,
                       unsigned int                tolerance)
{
  return result->pixels_changed &&
         result->max_diff > tolerance;
}
