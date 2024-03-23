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
 * Author: Kristian Høgsberg <krh@redhat.com>
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
draw_rect (cairo_t *cr, int x, int y)
{
    cairo_new_path (cr);
    cairo_rectangle (cr, x, y, WIDTH, HEIGHT);
    cairo_fill (cr);
}

static void
draw_rects (cairo_t *cr, int x, int y)
{
    int width = WIDTH / 3;
    int height = HEIGHT / 2;

    cairo_new_path (cr);
    cairo_rectangle (cr, x, y, width, height);
    cairo_rectangle (cr, x + width, y + height, width, height);
    cairo_rectangle (cr, x + 2 * width, y, width, height);
    cairo_fill (cr);
}

static void
draw_polygon (cairo_t *cr, int x, int y)
{
    cairo_new_path (cr);
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x, y + HEIGHT);
    cairo_line_to (cr, x + WIDTH / 2, y + 3 * HEIGHT / 4);
    cairo_line_to (cr, x + WIDTH, y + HEIGHT);
    cairo_line_to (cr, x + WIDTH, y);
    cairo_line_to (cr, x + WIDTH / 2, y + HEIGHT / 4);
    cairo_close_path (cr);
    cairo_fill (cr);
}

static void
clip_none (cairo_t *cr, int x, int y)
{
}

static void
clip_rect (cairo_t *cr, int x, int y)
{
    cairo_new_path (cr);
    cairo_rectangle (cr, x + (int)WIDTH / 6, y + (int)HEIGHT / 6,
		     4 * ((int)WIDTH  / 6), 4 * ((int)WIDTH / 6));
    cairo_clip (cr);
    cairo_new_path (cr);
}

static void
clip_rects (cairo_t *cr, int x, int y)
{
    int height = HEIGHT / 3;

    cairo_new_path (cr);
    cairo_rectangle (cr, x, y, WIDTH, height);
    cairo_rectangle (cr, x, y + 2 * height, WIDTH, height);
    cairo_clip (cr);
    cairo_new_path (cr);
}

static void
clip_circle (cairo_t *cr, int x, int y)
{
    cairo_new_path (cr);
    cairo_arc (cr, x + WIDTH / 2, y + HEIGHT / 2,
	       WIDTH / 3, 0, 2 * M_PI);
    cairo_clip (cr);
    cairo_new_path (cr);
}

static void (* const pattern_funcs[])(const cairo_test_context_t *ctx, cairo_t *cr, int x, int y) = {
    set_solid_pattern,
    set_translucent_pattern,
    set_gradient_pattern,
    set_image_pattern,
};

static void (* const draw_funcs[])(cairo_t *cr, int x, int y) = {
    draw_rect,
    draw_rects,
    draw_polygon,
};

static void (* const clip_funcs[])(cairo_t *cr, int x, int y) = {
    clip_none,
    clip_rect,
    clip_rects,
    clip_circle,
};

#define IMAGE_WIDTH (ARRAY_LENGTH (pattern_funcs) * (WIDTH + PAD) + PAD)
#define IMAGE_HEIGHT (ARRAY_LENGTH (draw_funcs) * ARRAY_LENGTH (clip_funcs) * (HEIGHT + PAD) + PAD)

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    size_t i, j, k, x, y;

    for (k = 0; k < ARRAY_LENGTH (clip_funcs); k++) {
	for (j = 0; j < ARRAY_LENGTH (draw_funcs); j++) {
	    for (i = 0; i < ARRAY_LENGTH (pattern_funcs); i++) {
		x = i * (WIDTH + PAD) + PAD;
		y = (ARRAY_LENGTH (draw_funcs) * k + j) * (HEIGHT + PAD) + PAD;

		cairo_save (cr);

		cairo_move_to (cr, x, y);
		clip_funcs[k] (cr, x, y);
		pattern_funcs[i] (ctx, cr, x, y);
		draw_funcs[j] (cr, x, y);
		if (cairo_status (cr))
		    cairo_test_log (ctx, "%d %d HERE!\n", (int)i, (int)j);

		cairo_restore (cr);
	    }
	}
    }

    if (cairo_status (cr) != CAIRO_STATUS_SUCCESS)
	cairo_test_log (ctx, "%d %d .HERE!\n", (int)i, (int)j);

    cairo_surface_destroy (image);
    image = NULL;

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (trap_clip,
	    "Trapezoid clipping",
	    "clip, trap", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
