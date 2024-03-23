/*
 * Copyright 2009 Chris Wilson
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

#include "cairo-test.h"

#define WIDTH 20
#define HEIGHT 20

static const char *png_filename = "romedalen.png";

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *image;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    image = cairo_test_create_surface_from_png (ctx, png_filename);
    cairo_set_source_surface (cr, image, 0, 0);
    cairo_surface_destroy (image);

    /* simple clip */
    cairo_save (cr);
    cairo_rectangle (cr, 2, 2, 16, 16);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, WIDTH, 0);

    /* unaligned clip */
    cairo_save (cr);
    cairo_rectangle (cr, 2.5, 2.5, 15, 15);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, -WIDTH, HEIGHT);

    /* aligned-clip */
    cairo_save (cr);
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_rectangle (cr, 0, 0, 20, 20);
    cairo_rectangle (cr, 3, 3, 10, 10);
    cairo_rectangle (cr, 7, 7, 10, 10);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, WIDTH, 0);

    /* force a clip-mask */
    cairo_save (cr);
    cairo_arc (cr, 10, 10, 10, 0, 2 * M_PI);
    cairo_new_sub_path (cr);
    cairo_arc_negative (cr, 10, 10, 5, 2 * M_PI, 0);
    cairo_new_sub_path (cr);
    cairo_arc (cr, 10, 10, 2, 0, 2 * M_PI);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_image,
	    "Tests painting an image through complex clips.",
	    "clip, paint", /* keywords */
	    NULL, /* requirements */
	    2 * WIDTH, 2 * HEIGHT,
	    NULL, draw)
