/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2007 Emmanuel Pacaud
 * Copyright © 2008 Benjamin Otte
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
 *          Emmanuel Pacaud <emmanuel.pacaud@lapp.in2p3.fr>
 */

#include "cairo-test.h"
#include <math.h>
#include <stdio.h>

#define STEPS 16
#define START_OPERATOR	CAIRO_OPERATOR_MULTIPLY
#define STOP_OPERATOR	CAIRO_OPERATOR_HSL_LUMINOSITY

#define SIZE 5
#define COUNT 4
#define FULL_WIDTH  ((STEPS + 1) * COUNT - 1)
#define FULL_HEIGHT ((COUNT + STOP_OPERATOR - START_OPERATOR) / COUNT) * (STEPS + 1)

static void
set_solid_pattern (cairo_t *cr,
		   int step,
		   cairo_bool_t bg,
		   cairo_bool_t alpha)
{
    double c, a;

    a = ((double) step) / (STEPS - 1);
    if (alpha) {
	c = 1;
    } else {
	c = a;
	a = 1;
    }

    if (bg) /* draw a yellow background fading in using discrete steps */
	cairo_set_source_rgba (cr, c, c, 0, a);
    else /* draw a teal foreground pattern fading in using discrete steps */
	cairo_set_source_rgba (cr, 0, c, c, a);
}

/* expects a STEP*STEP pixel rectangle */
static void
do_blend_solid (cairo_t *cr, cairo_operator_t op, cairo_bool_t alpha)
{
    int x;

    cairo_save (cr);
    cairo_scale (cr, SIZE, SIZE);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    for (x = 0; x < STEPS; x++) {
	/* draw the background using discrete steps */
	set_solid_pattern (cr, x, TRUE, alpha);
	cairo_rectangle (cr, x, 0, 1, STEPS);
	cairo_fill (cr);
    }

    cairo_set_operator (cr, op);
    for (x = 0; x < STEPS; x++) {
	/* draw an orthogonal foreground pattern using discrete steps */
	set_solid_pattern (cr, x, FALSE, alpha);
	cairo_rectangle (cr, 0, x, STEPS, 1);
	cairo_fill (cr);
    }

    cairo_restore (cr);
}

static void
create_patterns (cairo_t *cr,
		 cairo_surface_t **bg,
		 cairo_surface_t **fg,
		 cairo_bool_t alpha)
{
    cairo_t *bgcr, *fgcr;

    *bg = cairo_surface_create_similar (cairo_get_target (cr),
					CAIRO_CONTENT_COLOR_ALPHA,
					SIZE * STEPS,
					SIZE * STEPS);
    *fg = cairo_surface_create_similar (cairo_get_target (cr),
					CAIRO_CONTENT_COLOR_ALPHA,
					SIZE * STEPS,
					SIZE * STEPS);

    bgcr = cairo_create (*bg);
    fgcr = cairo_create (*fg);

    do_blend_solid (bgcr, CAIRO_OPERATOR_DEST, alpha);
    do_blend_solid (fgcr, CAIRO_OPERATOR_SOURCE, alpha);

    cairo_destroy (bgcr);
    cairo_destroy (fgcr);
}

/* expects a STEP*STEP pixel rectangle */
static void
do_blend (cairo_t *cr, cairo_operator_t op, cairo_bool_t alpha)
{
    cairo_surface_t *bg, *fg;

    create_patterns (cr, &bg, &fg, alpha);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_surface (cr, bg, 0, 0);
    cairo_paint (cr);

    cairo_set_operator (cr, op);
    cairo_set_source_surface (cr, fg, 0, 0);
    cairo_paint (cr);

    cairo_surface_destroy (fg);
    cairo_surface_destroy (bg);
}

static void
do_blend_mask (cairo_t *cr, cairo_operator_t op, cairo_bool_t alpha)
{
    cairo_surface_t *bg, *fg;

    create_patterns (cr, &bg, &fg, alpha);

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_surface (cr, bg, 0, 0);
    cairo_paint (cr);

    cairo_set_operator (cr, op);
    cairo_set_source_surface (cr, fg, 0, 0);
    cairo_paint_with_alpha (cr, .5);

    cairo_surface_destroy (fg);
    cairo_surface_destroy (bg);
}

static cairo_test_status_t
draw (cairo_t *cr, cairo_bool_t alpha,
      void (*blend)(cairo_t *, cairo_operator_t, cairo_bool_t))
{
    size_t i = 0;
    cairo_operator_t op;

    for (op = START_OPERATOR; op <= STOP_OPERATOR; op++, i++) {
	cairo_save (cr);
	cairo_translate (cr,
		SIZE * (STEPS + 1) * (i % COUNT),
		SIZE * (STEPS + 1) * (i / COUNT));
	blend (cr, op, alpha);
	cairo_restore (cr);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_extended_blend (cairo_t *cr, int width, int height)
{
    return draw (cr, FALSE, do_blend);
}

static cairo_test_status_t
draw_extended_blend_alpha (cairo_t *cr, int width, int height)
{
    return draw (cr, TRUE, do_blend);
}

static cairo_test_status_t
draw_extended_blend_solid (cairo_t *cr, int width, int height)
{
    return draw (cr, FALSE, do_blend_solid);
}

static cairo_test_status_t
draw_extended_blend_solid_alpha (cairo_t *cr, int width, int height)
{
    return draw (cr, TRUE, do_blend_solid);
}

static cairo_test_status_t
draw_extended_blend_mask (cairo_t *cr, int width, int height)
{
    return draw (cr, FALSE, do_blend_mask);
}
static cairo_test_status_t
draw_extended_blend_alpha_mask (cairo_t *cr, int width, int height)
{
    return draw (cr, TRUE, do_blend_mask);
}

CAIRO_TEST (extended_blend,
	    "Tests extended blend modes without alpha",
	    "operator", /* keywords */
	    NULL, /* requirements */
	    FULL_WIDTH * SIZE, FULL_HEIGHT * SIZE,
	    NULL, draw_extended_blend)

CAIRO_TEST (extended_blend_alpha,
	    "Tests extended blend modes with alpha",
	    "operator", /* keywords */
	    NULL, /* requirements */
	    FULL_WIDTH * SIZE, FULL_HEIGHT * SIZE,
	    NULL, draw_extended_blend_alpha)

CAIRO_TEST (extended_blend_mask,
	    "Tests extended blend modes with an alpha mask",
	    "operator,mask", /* keywords */
	    NULL, /* requirements */
	    FULL_WIDTH * SIZE, FULL_HEIGHT * SIZE,
	    NULL, draw_extended_blend_mask)
CAIRO_TEST (extended_blend_alpha_mask,
	    "Tests extended blend modes with an alpha mask",
	    "operator,mask", /* keywords */
	    NULL, /* requirements */
	    FULL_WIDTH * SIZE, FULL_HEIGHT * SIZE,
	    NULL, draw_extended_blend_alpha_mask)


CAIRO_TEST (extended_blend_solid,
	    "Tests extended blend modes on solid patterns without alpha",
	    "operator", /* keywords */
	    NULL, /* requirements */
	    FULL_WIDTH * SIZE, FULL_HEIGHT * SIZE,
	    NULL, draw_extended_blend_solid)

CAIRO_TEST (extended_blend_solid_alpha,
	    "Tests extended blend modes on solid patterns with alpha",
	    "operator", /* keywords */
	    NULL, /* requirements */
	    FULL_WIDTH * SIZE, FULL_HEIGHT * SIZE,
	    NULL, draw_extended_blend_solid_alpha)
