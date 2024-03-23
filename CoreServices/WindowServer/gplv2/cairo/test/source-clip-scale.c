/*
 * Copyright © 2005 Mozilla Corporation
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
#include <math.h>
#include <stdio.h>

#define SIZE 12

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *source;
    cairo_t *cr2;

    source = cairo_surface_create_similar (cairo_get_group_target (cr),
					   CAIRO_CONTENT_COLOR_ALPHA,
					   SIZE, SIZE);
    cr2 = cairo_create (source);
    cairo_surface_destroy (source);

    /* Fill the source surface with green */
    cairo_set_source_rgb (cr2, 0, 1, 0);
    cairo_paint (cr2);

    /* Draw a blue square in the middle of the source with clipping.
     * Note that we are only clipping within a save/restore block but
     * the buggy behavior demonstrates that the clip remains present
     * on the surface. */
    cairo_save (cr2);
    cairo_rectangle (cr2,
		     SIZE / 4, SIZE / 4,
		     SIZE / 2, SIZE / 2);
    cairo_clip (cr2);
    cairo_set_source_rgb (cr2, 0, 0, 1);
    cairo_paint (cr2);
    cairo_restore (cr2);

    /* Fill the destination surface with solid red (should not appear
     * in final result) */
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    /* Now draw the source surface onto the destination with scaling. */
    cairo_scale (cr, 2.0, 1.0);

    cairo_set_source_surface (cr, cairo_get_target (cr2), 0, 0);
    cairo_destroy (cr2);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (source_clip_scale,
	    "Test that a source surface is not affected by a clip when scaling",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    SIZE * 2, SIZE,
	    NULL, draw)
