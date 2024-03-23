/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

/* This test tries to emulate the behaviour of most toolkits; it tries
 * to simulate typical usage of a single surface with multiple exposures.
 *
 * The first goal of the test is to reproduce the XSetClipMask(NULL) bug
 * reintroduced in 1.6.2 (but was originally fixed in 40558cb15). As I've
 * made the same mistake again, it is worth adding a regression test...
 */

#include "cairo-test.h"

#include <stdio.h>
#include <stdlib.h>

#include "cairo.h"

#include "buffer-diff.h"

#define SIZE 160
#define NLOOPS 10

static const char *png_filename = "romedalen.png";

static void
draw_mask (cairo_t *cr)
{
    cairo_surface_t *surface;
    cairo_t *cr2;

    surface = cairo_surface_create_similar (cairo_get_group_target (cr),
	                                    CAIRO_CONTENT_ALPHA,
					    50, 50);
    cr2 = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_rectangle (cr2,
	             0, 0,
	             40, 40);
    cairo_rectangle (cr2,
	             10, 10,
	             40, 40);
    cairo_clip (cr2);

    cairo_move_to (cr2, 0, 25);
    cairo_line_to (cr2, 50, 25);
    cairo_move_to (cr2, 25, 0);
    cairo_line_to (cr2, 25, 50);
    cairo_set_source_rgb (cr2, 1, 1, 1);
    cairo_stroke (cr2);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_mask_surface (cr, cairo_get_target (cr2), 50, 50);
    cairo_destroy (cr2);
}

static cairo_surface_t *
clone_similar_surface (cairo_surface_t * target, cairo_surface_t *surface)
{
    cairo_t *cr;
    cairo_surface_t *similar;

    similar = cairo_surface_create_similar (target,
	                              cairo_surface_get_content (surface),
				      cairo_image_surface_get_width (surface),
				      cairo_image_surface_get_height (surface));

    cr = cairo_create (similar);
    cairo_surface_destroy (similar);

    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    similar = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return similar;
}

static void
draw_image (const cairo_test_context_t *ctx,
	    cairo_t *cr,
	    cairo_surface_t *image)
{
    cairo_set_source_surface (cr, image, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
}

static void
draw (const cairo_test_context_t *ctx,
      cairo_t *cr,
      cairo_surface_t *image,
      cairo_rectangle_t *region,
      int n_regions)
{
    cairo_save (cr);
    if (region != NULL) {
	int i;
	for (i = 0; i < n_regions; i++) {
	    cairo_rectangle (cr,
			     region[i].x, region[i].y,
			     region[i].width, region[i].height);
	}
	cairo_clip (cr);
    }
    cairo_push_group (cr);
    draw_image (ctx, cr, image);
    draw_mask (cr);
    cairo_pop_group_to_source (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);
}

static cairo_test_status_t
draw_func (cairo_t *cr, int width, int height)
{
    cairo_rectangle_t region[4];
    const cairo_test_context_t *ctx;
    cairo_surface_t *source, *image;
    int i, j;

    ctx = cairo_test_get_context (cr);

    source = cairo_test_create_surface_from_png (ctx, png_filename);
    image = clone_similar_surface (cairo_get_group_target (cr), source);
    cairo_surface_destroy (source);

    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR);
    draw (ctx, cr, image, NULL, 0);
    for (i = 0; i < NLOOPS; i++) {
	for (j = 0; j < NLOOPS; j++) {
	    region[0].x = i * SIZE / NLOOPS;
	    region[0].y = i * SIZE / NLOOPS;
	    region[0].width = SIZE / 4;
	    region[0].height = SIZE / 4;

	    region[1].x = j * SIZE / NLOOPS;
	    region[1].y = j * SIZE / NLOOPS;
	    region[1].width = SIZE / 4;
	    region[1].height = SIZE / 4;

	    region[2].x = i * SIZE / NLOOPS;
	    region[2].y = j * SIZE / NLOOPS;
	    region[2].width = SIZE / 4;
	    region[2].height = SIZE / 4;

	    region[3].x = j * SIZE / NLOOPS;
	    region[3].y = i * SIZE / NLOOPS;
	    region[3].width = SIZE / 4;
	    region[3].height = SIZE / 4;

	    draw (ctx, cr, image, region, 4);
	}
    }

    cairo_pop_group_to_source (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    cairo_surface_destroy (image);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (xlib_expose_event,
	    "Emulate a typical expose event",
	    "xlib", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw_func)
