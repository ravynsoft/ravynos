/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2009 Chris Wilson
 * Copyright 2010 Andrea Canciani
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
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define WIDTH 60
#define HEIGHT 60

static void
stroke (cairo_t *cr)
{
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_IN);
    cairo_set_source_rgb (cr, 0, 0.7, 0);
    cairo_arc (cr, 10, 10, 7.5, 0, 2 * M_PI);
    cairo_move_to (cr, 0, 20);
    cairo_line_to (cr, 20, 0);
    cairo_stroke (cr);
}

static void
fill (cairo_t *cr)
{
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_IN);
    cairo_set_source_rgb (cr, 0, 0.7, 0);
    cairo_new_sub_path (cr);
    cairo_arc (cr, 10, 10, 8.5, 0, 2 * M_PI);
    cairo_new_sub_path (cr);
    cairo_arc_negative (cr, 10, 10, 6.5, 2 * M_PI, 0);

    cairo_move_to (cr, -1, 19);
    cairo_line_to (cr,  1, 21);
    cairo_line_to (cr, 21,  1);
    cairo_line_to (cr, 19, -1);
    cairo_line_to (cr, -1, 19);

    cairo_fill (cr);
}

static void
clip_simple (cairo_t *cr)
{
    cairo_rectangle (cr, 0, 0, 20, 20);
    cairo_clip (cr);
}

static void
clip_unaligned (cairo_t *cr)
{
    cairo_rectangle (cr, 0.5, 0.5, 20, 20);
    cairo_clip (cr);
}

static void
clip_aligned (cairo_t *cr)
{
    cairo_fill_rule_t orig_rule;

    orig_rule = cairo_get_fill_rule (cr);
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_rectangle (cr, 0, 0, 20, 20);
    cairo_rectangle (cr, 3, 3, 10, 10);
    cairo_rectangle (cr, 7, 7, 10, 10);
    cairo_clip (cr);
    cairo_set_fill_rule (cr, orig_rule);
}

static void
clip_mask (cairo_t *cr)
{
    cairo_arc (cr, 10, 10, 10, 0, 2 * M_PI);
    cairo_new_sub_path (cr);
    cairo_arc_negative (cr, 10, 10, 5, 2 * M_PI, 0);
    cairo_new_sub_path (cr);
    cairo_arc (cr, 10, 10, 2, 0, 2 * M_PI);
    cairo_clip (cr);
}

static void (* const clip_funcs[])(cairo_t *cr) = {
    clip_simple,
    clip_unaligned,
    clip_aligned,
    clip_mask
};

static double translations[][2] = {
    { 10, 10 },
    { WIDTH, 0 },
    { -WIDTH, HEIGHT },
    { WIDTH, 0 }
};

static cairo_test_status_t
draw (cairo_t *cr, void (*shapes)(cairo_t *))
{
    unsigned int i;
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    for (i = 0; i < ARRAY_LENGTH (clip_funcs); i++) {
	cairo_translate (cr, translations[i][0], translations[i][1]);

	cairo_save (cr);
	cairo_scale (cr, 2, 2);
	clip_funcs[i] (cr);
	shapes (cr);
	cairo_restore (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_stroke (cairo_t *cr, int width, int height)
{
    return draw (cr, stroke);
}

static cairo_test_status_t
draw_fill_nz (cairo_t *cr, int width, int height)
{
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);
    return draw (cr, fill);
}

static cairo_test_status_t
draw_fill_eo (cairo_t *cr, int width, int height)
{
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    return draw (cr, fill);
}

CAIRO_TEST (clip_stroke_unbounded,
	    "Tests unbounded stroke through complex clips.",
	    "clip, stroke, unbounded", /* keywords */
	    NULL, /* requirements */
	    2 * WIDTH, 2 * HEIGHT,
	    NULL, draw_stroke)

CAIRO_TEST (clip_fill_nz_unbounded,
	    "Tests unbounded fill through complex clips (with winding fill rule).",
	    "clip, fill, unbounded", /* keywords */
	    NULL, /* requirements */
	    2 * WIDTH, 2 * HEIGHT,
	    NULL, draw_fill_nz)

CAIRO_TEST (clip_fill_eo_unbounded,
	    "Tests unbounded fill through complex clips (with even-odd fill rule).",
	    "clip, fill, unbounded", /* keywords */
	    NULL, /* requirements */
	    2 * WIDTH, 2 * HEIGHT,
	    NULL, draw_fill_eo)
