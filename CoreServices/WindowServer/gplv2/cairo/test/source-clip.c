/*
 * Copyright © 2005 Red Hat, Inc.
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
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Owen Taylor <otaylor@redhat.com>
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

    /* Draw a blue square in the middle of the source with clipping,
     * and leave the clip there. */
    cairo_rectangle (cr2,
		     SIZE / 4, SIZE / 4,
		     SIZE / 2, SIZE / 2);
    cairo_clip (cr2);
    cairo_set_source_rgb (cr2, 0, 0, 1);
    cairo_paint (cr2);

    /* Fill the destination surface with solid red (should not appear
     * in final result) */
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    /* Now draw the source surface onto the destination surface */
    cairo_set_source_surface (cr, cairo_get_target (cr2), 0, 0);
    cairo_paint (cr);

    cairo_destroy (cr2);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (source_clip,
	    "Test that a source surface is not affected by a clip",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
