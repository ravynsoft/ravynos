/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2006 Red Hat, Inc.
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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define PAD 2
#define SIZE 10

typedef void (*path_func_t) (cairo_t *cr);

static void
rectangle (cairo_t *cr)
{
    cairo_rectangle (cr, PAD, PAD, SIZE, SIZE);
}

static void
circle (cairo_t *cr)
{
    cairo_arc (cr,
	       PAD + SIZE / 2, PAD + SIZE / 2,
	       SIZE / 2,
	       0, 2 * M_PI);
}

/* Given a path-generating function and two opaque patterns, fill and
 * stroke the path with the patterns (to an offscreen group), then
 * blend the result into the destination with the given alpha
 * value.
 */
static void
fill_and_stroke_alpha (cairo_t		*cr,
		       path_func_t	 path_func,
		       cairo_pattern_t	*fill_pattern,
		       cairo_pattern_t	*stroke_pattern,
		       double		 alpha)
{
    cairo_push_group (cr);
    {
	(path_func) (cr);
	cairo_set_source (cr, fill_pattern);
	cairo_fill_preserve (cr);
	cairo_set_source (cr, stroke_pattern);
	cairo_stroke (cr);
    }
    cairo_pop_group_to_source (cr);
    cairo_paint_with_alpha (cr, alpha);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *blue;
    cairo_pattern_t *red;

    blue = cairo_pattern_create_rgb (0.0, 0.0, 1.0);
    red = cairo_pattern_create_rgb (1.0, 0.0, 0.0);

    cairo_test_paint_checkered (cr);

    fill_and_stroke_alpha (cr, rectangle, blue, red, 0.5);

    cairo_translate (cr, SIZE + 2 * PAD, 0);

    fill_and_stroke_alpha (cr, circle, red, blue, 0.5);

    cairo_pattern_destroy (blue);
    cairo_pattern_destroy (red);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (fill_and_stroke_alpha,
	    "Use a group to fill/stroke a path then blend the result with alpha onto the destination",
	    "fill-and-stroke, fill, stroke", /* keywords */
	    NULL, /* requirements */
	    2 * SIZE + 4 * PAD, SIZE + 2 * PAD,
	    NULL, draw)
