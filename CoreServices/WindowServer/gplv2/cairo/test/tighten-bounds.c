/*
 * Copyright © 2011 Uli Schlachter
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Uli Schlachter <psychon@znc.in>
 */

#include "cairo-test.h"

static void path_none (cairo_t *cr, int size)
{
}

static void path_box (cairo_t *cr, int size)
{
    cairo_rectangle (cr, 0, 0, size, size);
}

static void path_box_unaligned (cairo_t *cr, int size)
{
    cairo_rectangle (cr, 0.5, 0.5, size - 1, size - 1);
}

static void path_triangle (cairo_t *cr, int size)
{
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, size/2, size);
    cairo_line_to (cr, size, 0);
    cairo_close_path (cr);
}

static void path_circle (cairo_t *cr, int size)
{
    cairo_arc (cr, size / 2.0, size / 2.0, size / 2.0, 0, 2 * M_PI);
}

static void (* const path_funcs[])(cairo_t *cr, int size) = {
    path_none,
    path_box,
    path_box_unaligned,
    path_triangle,
    path_circle
};

#define SIZE 20
#define PAD 2
#define TYPES 6
/* All-clipped is boring, thus we skip path_none for clipping */
#define CLIP_OFFSET 1
#define IMAGE_WIDTH ((ARRAY_LENGTH (path_funcs) - CLIP_OFFSET) * TYPES * (SIZE + PAD) - PAD)
#define IMAGE_HEIGHT (ARRAY_LENGTH (path_funcs) * (SIZE + PAD) - PAD)

static void
draw_idx (cairo_t *cr, int i, int j, int type)
{
    cairo_bool_t little_path;
    cairo_bool_t empty_clip;
    cairo_bool_t little_clip;

    /* The lowest bit controls the path, the rest the clip */
    little_path = type & 1;

    /* We don't want the combination "empty_clip = TRUE, little_clip = FALSE"
     * (== all clipped).
     */
    switch (type >> 1)
    {
    case 0:
	empty_clip = FALSE;
	little_clip = FALSE;
	break;
    case 1:
	empty_clip = FALSE;
	little_clip = TRUE;
	break;
    case 2:
	empty_clip = TRUE;
	little_clip = TRUE;
	break;
    default:
	return;
    }

    cairo_save (cr);

    /* Thanks to the fill rule, drawing something twice removes it again */
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

    path_funcs[i] (cr, SIZE);
    if (empty_clip)
	path_funcs[i] (cr, SIZE);
    if (little_clip)
    {
	cairo_save (cr);
	cairo_translate (cr, SIZE / 4, SIZE / 4);
	path_funcs[i] (cr, SIZE / 2);
	cairo_restore (cr);
    }
    cairo_clip (cr);

    path_funcs[j] (cr, SIZE);
    path_funcs[j] (cr, SIZE);
    if (little_path)
    {
	/* Draw the object again in the center of itself */
	cairo_save (cr);
	cairo_translate (cr, SIZE / 4, SIZE / 4);
	path_funcs[j] (cr, SIZE / 2);
	cairo_restore (cr);
    }
    cairo_fill (cr);
    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    size_t i, j, k;

    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_paint (cr);

    /* Set an unbounded operator so that we can see how accurate the bounded
     * extents were.
     */
    cairo_set_operator (cr, CAIRO_OPERATOR_IN);
    cairo_set_source_rgb (cr, 1, 1, 1);

    for (j = 0; j < ARRAY_LENGTH (path_funcs); j++) {
	cairo_save (cr);
	for (i = CLIP_OFFSET; i < ARRAY_LENGTH (path_funcs); i++) {
	    for (k = 0; k < TYPES; k++) {
		cairo_save (cr);
		cairo_rectangle (cr, 0, 0, SIZE, SIZE);
		cairo_clip (cr);
		draw_idx (cr, i, j, k);
		cairo_restore (cr);
		cairo_translate (cr, SIZE + PAD, 0);
	    }
	}
	cairo_restore (cr);
	cairo_translate (cr, 0, SIZE + PAD);
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (tighten_bounds,
	    "Tests that we tighten the bounds after tessellation.",
	    "fill", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
