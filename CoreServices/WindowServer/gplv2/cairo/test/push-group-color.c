/*
 * Copyright © 2005 Mozilla Corporation
 * Copyright © 2009 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Mozilla Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Mozilla Corporation makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * MOZILLA CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL MOZILLA CORPORATION BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Vladimir Vukicevic <vladimir@pobox.com>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define UNIT_SIZE 100
#define PAD 5
#define INNER_PAD 10

#define WIDTH (UNIT_SIZE + PAD) + PAD
#define HEIGHT (UNIT_SIZE + PAD) + PAD

static cairo_pattern_t *
argb32_source (void)
{
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    cairo_t *cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 16, 16);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_rectangle (cr, 8, 0, 8, 8);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_rectangle (cr, 0, 8, 8, 8);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_rectangle (cr, 8, 8, 8, 8);
    cairo_fill (cr);

    pattern = cairo_pattern_create_for_surface (cairo_get_target (cr));
    cairo_destroy (cr);

    return pattern;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *gradient, *image;

    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);

    cairo_translate (cr, PAD, PAD);

    /* clip to the unit size */
    cairo_rectangle (cr, 0, 0,
		     UNIT_SIZE, UNIT_SIZE);
    cairo_clip (cr);

    cairo_rectangle (cr, 0, 0,
		     UNIT_SIZE, UNIT_SIZE);
    cairo_set_source_rgba (cr, 0, 0, 0, 1);
    cairo_set_line_width (cr, 2);
    cairo_stroke (cr);

    /* start a group */
    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR);

    /* draw a gradient background */
    cairo_save (cr);
    cairo_translate (cr, INNER_PAD, INNER_PAD);
    cairo_new_path (cr);
    cairo_rectangle (cr, 0, 0,
		     UNIT_SIZE - (INNER_PAD*2), UNIT_SIZE - (INNER_PAD*2));
    gradient = cairo_pattern_create_linear (UNIT_SIZE - (INNER_PAD*2), 0,
                                            UNIT_SIZE - (INNER_PAD*2), UNIT_SIZE - (INNER_PAD*2));
    cairo_pattern_add_color_stop_rgba (gradient, 0.0, 0.3, 0.3, 0.3, 1.0);
    cairo_pattern_add_color_stop_rgba (gradient, 1.0, 1.0, 1.0, 1.0, 1.0);
    cairo_set_source (cr, gradient);
    cairo_pattern_destroy (gradient);
    cairo_fill (cr);
    cairo_restore (cr);

    /* draw diamond */
    cairo_move_to (cr, UNIT_SIZE / 2, 0);
    cairo_line_to (cr, UNIT_SIZE    , UNIT_SIZE / 2);
    cairo_line_to (cr, UNIT_SIZE / 2, UNIT_SIZE);
    cairo_line_to (cr, 0            , UNIT_SIZE / 2);
    cairo_close_path (cr);
    cairo_set_source_rgba (cr, 0, 0, 1, 1);
    cairo_fill (cr);

    /* draw circle */
    cairo_arc (cr,
	       UNIT_SIZE / 2, UNIT_SIZE / 2,
	       UNIT_SIZE / 3.5,
	       0, M_PI * 2);
    cairo_set_source_rgba (cr, 1, 0, 0, 1);
    cairo_fill (cr);

    /* and put the image on top */
    cairo_translate (cr, UNIT_SIZE/2 - 8, UNIT_SIZE/2 - 8);
    image = argb32_source ();
    cairo_set_source (cr, image);
    cairo_pattern_destroy (image);
    cairo_paint (cr);

    cairo_pop_group_to_source (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (push_group_color,
	    "Verify that cairo_push_group_with_content works.",
	    "group", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
