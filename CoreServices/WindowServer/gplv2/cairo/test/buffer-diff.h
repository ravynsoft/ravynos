/* imagediff - Compare two images
 *
 * Copyright © 2004 Richard D. Worth
 * Copyright © 2006 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the authors
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Richard D. Worth <richard@theworths.org>
 *	    Carl Worth <cworth@cworth.org>
 */

#ifndef BUFFER_DIFF_H
#define BUFFER_DIFF_H

#include "cairo-test.h"

typedef struct _buffer_diff_result {
    unsigned int pixels_changed;
    unsigned int max_diff;
} buffer_diff_result_t;

/* Compares two image buffers ignoring the alpha channel.
 *
 * Provides number of pixels changed and maximum single-channel
 * difference in result.
 *
 * Also fills in a "diff" buffer intended to visually show where the
 * images differ.
 */
void
buffer_diff_noalpha (const unsigned char *buf_a,
		     const unsigned char *buf_b,
		     unsigned char *buf_diff,
		     int	    width,
		     int	    height,
		     int	    stride,
		     buffer_diff_result_t *result);

/* The central algorithm to compare two images, and return the differences
 * in the surface_diff.
 *
 * Provides number of pixels changed and maximum single-channel
 * difference in result.
 */
cairo_status_t
image_diff (const cairo_test_context_t *ctx,
	    cairo_surface_t *surface_a,
	    cairo_surface_t *surface_b,
	    cairo_surface_t *surface_diff,
	    buffer_diff_result_t *result);

cairo_bool_t
image_diff_is_failure (const buffer_diff_result_t *result,
                       unsigned int                tolerance);

#endif
