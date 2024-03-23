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
 * Authors: Owen Taylor <otaylor@redhat.com>
 *          Kristian Høgsberg <krh@redhat.com>
 */

#include "cairo-test.h"
#include <math.h>
#include <stdio.h>

#define WIDTH 16
#define HEIGHT 16
#define PAD 2

static const char *png_filename = "romedalen.png";
static cairo_surface_t *image;

static void
set_solid_pattern (const cairo_test_context_t *ctx, cairo_t *cr, int x, int y)
{
    cairo_set_source_rgb (cr, 0, 0, 0.6);
}

static void
set_translucent_pattern (const cairo_test_context_t *ctx, cairo_t *cr, int x, int y)
{
    cairo_set_source_rgba (cr, 0, 0, 0.6, 0.5);
}

static void
set_gradient_pattern (const cairo_test_context_t *ctx, cairo_t *cr, int x, int y)
{
    cairo_pattern_t *pattern;

    pattern =
	cairo_pattern_create_linear (x, y, x + WIDTH, y + HEIGHT);
    cairo_pattern_add_color_stop_rgba (pattern, 0, 1, 1, 1, 1);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0.4, 1);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
}

static void
set_image_pattern (const cairo_test_context_t *ctx, cairo_t *cr, int x, int y)
{
    cairo_pattern_t *pattern;

    if (image == NULL || cairo_surface_status (image)) {
	cairo_surface_destroy (image);
	image = cairo_test_create_surface_from_png (ctx, png_filename);
    }

    pattern = cairo_pattern_create_for_surface (image);
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
}

static void
mask_polygon (cairo_t *cr, int x, int y)
{
    cairo_surface_t *mask_surface;
    cairo_t *cr2;

    mask_surface = cairo_surface_create_similar (cairo_get_group_target (cr),
						 CAIRO_CONTENT_ALPHA,
						 WIDTH, HEIGHT);
    cr2 = cairo_create (mask_surface);
    cairo_surface_destroy (mask_surface);

    cairo_save (cr2);
    cairo_set_operator (cr2, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr2);
    cairo_restore (cr2);

    cairo_set_source_rgb (cr2, 1, 1, 1); /* white */

    cairo_new_path (cr2);
    cairo_move_to (cr2, 0, 0);
    cairo_line_to (cr2, 0, HEIGHT);
    cairo_line_to (cr2, WIDTH / 2, 3 * HEIGHT / 4);
    cairo_line_to (cr2, WIDTH, HEIGHT);
    cairo_line_to (cr2, WIDTH, 0);
    cairo_line_to (cr2, WIDTH / 2, HEIGHT / 4);
    cairo_close_path (cr2);
    cairo_fill (cr2);

    cairo_mask_surface (cr, cairo_get_target (cr2), x, y);
    cairo_destroy (cr2);
}

static void
mask_alpha (cairo_t *cr, int x, int y)
{
    cairo_paint_with_alpha (cr, 0.75);
}

static void
mask_gradient (cairo_t *cr, int x, int y)
{
    cairo_pattern_t *pattern;

    pattern = cairo_pattern_create_linear (x, y,
					   x + WIDTH, y + HEIGHT);

    cairo_pattern_add_color_stop_rgba (pattern,
				       0,
				       1, 1, 1, 1);
    cairo_pattern_add_color_stop_rgba (pattern,
				       1,
				       1, 1, 1, 0);

    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);
}

static void
clip_none (cairo_t *cr, int x, int y)
{
}

static void
clip_rects (cairo_t *cr, int x, int y)
{
    int height = HEIGHT / 3;

    cairo_new_path (cr);
    cairo_rectangle (cr, x, y, WIDTH, height);
    cairo_rectangle (cr, x, y + 2 * height, WIDTH, height);
    cairo_clip (cr);
}

static void
clip_circle (cairo_t *cr, int x, int y)
{
    cairo_new_path (cr);
    cairo_arc (cr, x + WIDTH / 2, y + HEIGHT / 2,
	       WIDTH / 2, 0, 2 * M_PI);
    cairo_clip (cr);
    cairo_new_path (cr);
}

static void (* const pattern_funcs[])(const cairo_test_context_t *ctx, cairo_t *cr, int x, int y) = {
    set_solid_pattern,
    set_translucent_pattern,
    set_gradient_pattern,
    set_image_pattern,
};

static void (* const mask_funcs[])(cairo_t *cr, int x, int y) = {
    mask_alpha,
    mask_gradient,
    mask_polygon,
};

static void (* const clip_funcs[])(cairo_t *cr, int x, int y) = {
    clip_none,
    clip_rects,
    clip_circle,
};

#define IMAGE_WIDTH (ARRAY_LENGTH (pattern_funcs) * (WIDTH + PAD) + PAD)
#define IMAGE_HEIGHT (ARRAY_LENGTH (mask_funcs) * ARRAY_LENGTH (clip_funcs) * (HEIGHT + PAD) + PAD)

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *tmp_surface;
    size_t i, j, k;
    cairo_t *cr2;

    /* Some of our drawing is unbounded, so we draw each test to
     * a temporary surface and copy over.
     */
    tmp_surface = cairo_surface_create_similar (cairo_get_group_target (cr),
						CAIRO_CONTENT_COLOR_ALPHA,
						IMAGE_WIDTH, IMAGE_HEIGHT);
    cr2 = cairo_create (tmp_surface);
    cairo_surface_destroy (tmp_surface);

    for (k = 0; k < ARRAY_LENGTH (clip_funcs); k++) {
	for (j = 0; j < ARRAY_LENGTH (mask_funcs); j++) {
	    for (i = 0; i < ARRAY_LENGTH (pattern_funcs); i++) {
		int x = i * (WIDTH + PAD) + PAD;
		int y = (ARRAY_LENGTH (mask_funcs) * k + j) * (HEIGHT + PAD) + PAD;

		/* Clear intermediate surface we are going to be drawing onto */
		cairo_save (cr2);
		cairo_set_operator (cr2, CAIRO_OPERATOR_CLEAR);
		cairo_paint (cr2);
		cairo_restore (cr2);

		/* draw */
		cairo_save (cr2);

		clip_funcs[k] (cr2, x, y);
		pattern_funcs[i] (ctx, cr2, x, y);
		mask_funcs[j] (cr2, x, y);

		cairo_restore (cr2);

		/* Copy back to the main pixmap */
		cairo_set_source_surface (cr, cairo_get_target (cr2), 0, 0);
		cairo_rectangle (cr, x, y, WIDTH, HEIGHT);
		cairo_fill (cr);
	    }
	}
    }

    cairo_destroy (cr2);

    cairo_surface_destroy (image);
    image = NULL;

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (mask,
	    "Tests of cairo_mask",
	    "mask", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)

