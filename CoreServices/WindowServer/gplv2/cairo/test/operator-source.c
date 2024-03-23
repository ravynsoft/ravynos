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
 * Authors: Kristian Høgsberg <krh@redhat.com>
 *          Owen Taylor <otaylor@redhat.com>
 *          Uli Schlachter <psychon@znc.in>
 */

#include "cairo-test.h"
#include <math.h>
#include <stdio.h>

#define WIDTH 16
#define HEIGHT 16
#define PAD 2

static void
set_solid_pattern (cairo_t *cr, int x, int y)
{
    cairo_set_source_rgb (cr, 1.0, 0, 0.0);
}

static void
set_translucent_pattern (cairo_t *cr, int x, int y)
{
    cairo_set_source_rgba (cr, 1, 0, 0, 0.5);
}

static void
set_gradient_pattern (cairo_t *cr, int x, int y)
{
    cairo_pattern_t *pattern;

    pattern = cairo_pattern_create_linear (x, y, x + WIDTH, y + HEIGHT);
    cairo_pattern_add_color_stop_rgba (pattern, 0.2, 1, 0, 0, 1);
    cairo_pattern_add_color_stop_rgba (pattern, 0.8, 1, 0, 0, 0.0);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
}

static void
set_surface_pattern (cairo_t *cr, int x, int y)
{
    cairo_surface_t *source_surface;
    cairo_t *cr2;

    double width = (int)(0.6 * WIDTH);
    double height = (int)(0.6 * HEIGHT);
    x += 0.2 * WIDTH;
    y += 0.2 * HEIGHT;

    source_surface = cairo_surface_create_similar (cairo_get_group_target (cr),
						   CAIRO_CONTENT_COLOR_ALPHA,
						   width, height);
    cr2 = cairo_create (source_surface);
    cairo_surface_destroy (source_surface);

    cairo_set_source_rgb (cr2, 1, 0, 0); /* red */
    cairo_paint (cr2);

    cairo_set_source_rgb (cr2, 1, 1, 1); /* white */

    cairo_arc (cr2, 0.5 * width, 0.5 * height, 0.5 * height, 0, 2 * M_PI);
    cairo_fill (cr2);

    cairo_set_source_surface (cr, cairo_get_target (cr2), x, y);
    cairo_destroy (cr2);
}

static void
draw_mask (cairo_t *cr, int x, int y)
{
    cairo_surface_t *mask_surface;
    cairo_t *cr2;

    double width = (int)(0.9 * WIDTH);
    double height = (int)(0.9 * HEIGHT);
    x += 0.05 * WIDTH;
    y += 0.05 * HEIGHT;

    mask_surface = cairo_surface_create_similar (cairo_get_group_target (cr),
						 CAIRO_CONTENT_ALPHA,
						 width, height);
    cr2 = cairo_create (mask_surface);
    cairo_surface_destroy (mask_surface);

    cairo_set_source_rgb (cr2, 1, 1, 1); /* white */

    cairo_arc (cr2, 0.5 * width, 0.5 * height, 0.45 * height, 0, 2 * M_PI);
    cairo_fill (cr2);

    cairo_mask_surface (cr, cairo_get_target (cr2), x, y);
    cairo_destroy (cr2);
}

static void
draw_glyphs (cairo_t *cr, int x, int y)
{
    cairo_text_extents_t extents;

    cairo_set_font_size (cr, 0.8 * HEIGHT);

    cairo_text_extents (cr, "FG", &extents);
    cairo_move_to (cr,
		   x + floor ((WIDTH - extents.width) / 2 + 0.5) - extents.x_bearing,
		   y + floor ((HEIGHT - extents.height) / 2 + 0.5) - extents.y_bearing);
    cairo_show_text (cr, "FG");
}

static void
draw_polygon (cairo_t *cr, int x, int y)
{
    double width = (int)(0.9 * WIDTH);
    double height = (int)(0.9 * HEIGHT);
    x += 0.05 * WIDTH;
    y += 0.05 * HEIGHT;

    cairo_new_path (cr);
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x, y + height);
    cairo_line_to (cr, x + width / 2, y + 3 * height / 4);
    cairo_line_to (cr, x + width, y + height);
    cairo_line_to (cr, x + width, y);
    cairo_line_to (cr, x + width / 2, y + height / 4);
    cairo_close_path (cr);
    cairo_fill (cr);
}

static void
draw_rects (cairo_t *cr, int x, int y, double offset)
{
    double block_width = (int)(0.33 * WIDTH + 0.5) - offset/3;
    double block_height = (int)(0.33 * HEIGHT + 0.5) - offset/3;
    int i, j;

    x += offset/2;
    y += offset/2;

    for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	    if ((i + j) % 2 == 0)
		cairo_rectangle (cr,
				 x + block_width * i, y + block_height * j,
				 block_width,         block_height);

    cairo_fill (cr);
}

static void
draw_aligned_rects (cairo_t *cr, int x, int y)
{
    draw_rects (cr, x, y, 0);
}

static void
draw_unaligned_rects (cairo_t *cr, int x, int y)
{
    draw_rects (cr, x, y, 2.1);
}

static void (* const pattern_funcs[])(cairo_t *cr, int x, int y) = {
    set_solid_pattern,
    set_translucent_pattern,
    set_gradient_pattern,
    set_surface_pattern,
};

static void (* const draw_funcs[])(cairo_t *cr, int x, int y) = {
    draw_mask,
    draw_glyphs,
    draw_polygon,
    draw_aligned_rects,
    draw_unaligned_rects
};

#define IMAGE_WIDTH (ARRAY_LENGTH (pattern_funcs) * (WIDTH + PAD) + PAD)
#define IMAGE_HEIGHT (ARRAY_LENGTH (draw_funcs) * (HEIGHT + PAD) + PAD)

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    size_t i, j, x, y;
    cairo_pattern_t *pattern;

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    for (j = 0; j < ARRAY_LENGTH (draw_funcs); j++) {
	for (i = 0; i < ARRAY_LENGTH (pattern_funcs); i++) {
	    x = i * (WIDTH + PAD) + PAD;
	    y = j * (HEIGHT + PAD) + PAD;

	    cairo_save (cr);

	    pattern = cairo_pattern_create_linear (x + WIDTH, y,
						   x,         y + HEIGHT);
	    cairo_pattern_add_color_stop_rgba (pattern, 0.2,
					       0.0, 0.0, 1.0, 1.0); /* Solid blue */
	    cairo_pattern_add_color_stop_rgba (pattern, 0.8,
					       0.0, 0.0, 1.0, 0.0); /* Transparent blue */
	    cairo_set_source (cr, pattern);
	    cairo_pattern_destroy (pattern);

	    cairo_rectangle (cr, x, y, WIDTH, HEIGHT);
	    cairo_fill_preserve (cr);
	    cairo_clip (cr);

	    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	    pattern_funcs[i] (cr, x, y);
	    draw_funcs[j] (cr, x, y);
	    if (cairo_status (cr))
		cairo_test_log (ctx, "%d %d HERE!\n", (int)i, (int)j);

	    cairo_restore (cr);
	}
    }

    if (cairo_status (cr) != CAIRO_STATUS_SUCCESS)
	cairo_test_log (ctx, "%d %d .HERE!\n", (int)i, (int)j);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (operator_source,
	    "Test of CAIRO_OPERATOR_SOURCE",
	    "operator", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
