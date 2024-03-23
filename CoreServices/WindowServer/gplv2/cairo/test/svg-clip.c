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

#include <stdio.h>

#include <cairo-svg.h>

/* Test SVG clipping */

#define WIDTH_IN_POINTS 600
#define HEIGHT_IN_POINTS 600
#define BASENAME "svg-clip.out"

static void
test_clip (cairo_t *cr, double width, double height)
{
    cairo_t *cr2;

    /* Basic test; set a square clip and draw a circle to be clipped
     * against it.*/

    cairo_rectangle (cr, 100, 100, 400, 400);
    cairo_clip (cr);
    cairo_arc (cr, 300, 300, 210, 0, 2 * M_PI);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill (cr);

    /* Add a plus shaped clip path to the square clip and draw a big
     * green square to test the new clip path. */

    cairo_save (cr);

    cairo_rectangle (cr, 250, 100, 100, 400);
    cairo_rectangle (cr, 100, 250, 400, 100);
    cairo_clip (cr);

    cairo_rectangle (cr, 0, 0, 600, 600);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_fill (cr);

    cairo_restore (cr);

    /* Set a bezier shape in addition to the rectangle clip set before
     * the cairo_save() to verify that we successfully removed the
     * plus shaped clip path and can set a new clip.*/

    cairo_move_to (cr, 600, 0);
    cairo_curve_to (cr, 300, 600, 0, 300, 600, 0);
    cairo_clip (cr);

    cairo_rectangle (cr, 0, 0, 600, 600);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_fill (cr);

    /* Create a new context for this surface to test overlapped
     * drawing from two contexts */
    cr2 = cairo_create (cairo_get_group_target (cr));

    /* Using the new context, draw a black vertical line, which should
     * appear unclipped on top of everything drawn so far. */
    cairo_move_to (cr2, 110, 0);
    cairo_line_to (cr2, 110, 600);
    cairo_stroke (cr2);

    /* Using the first context, draw another black vertical line.
     * This line should be clipped against the bezier clipping path set
     * earlier. */
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_move_to (cr, 400, 0);
    cairo_line_to (cr, 400, 600);
    cairo_stroke (cr);

    cairo_destroy (cr2);

    /* Test reset clip.  Draw a transparent black circle over
     * everything.  Specifically, make sure the circle extends outside
     * the square clip set at the top of this function.  */
    cairo_reset_clip (cr);
    cairo_arc (cr, 300, 300, 220, 0, 2 * M_PI);
    cairo_set_source_rgba (cr, 0, 0, 0, 0.2);
    cairo_fill (cr);
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_t *cr;
    cairo_surface_t *surface;
    char *filename;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

    if (! cairo_test_is_target_enabled (ctx, "svg11") &&
	! cairo_test_is_target_enabled (ctx, "svg12"))
    {
	return CAIRO_TEST_UNTESTED;
    }

    xasprintf (&filename, "%s/%s.svg", path, BASENAME);
    surface = cairo_svg_surface_create (filename,
					WIDTH_IN_POINTS, HEIGHT_IN_POINTS);
    if (cairo_surface_status (surface)) {
	cairo_test_log (ctx,
			"Failed to create svg surface for file %s: %s\n",
			filename, cairo_status_to_string (cairo_surface_status (surface)));
	free (filename);
	return CAIRO_TEST_FAILURE;
    }

    cr = cairo_create (surface);

    test_clip (cr, WIDTH_IN_POINTS, HEIGHT_IN_POINTS);
    cairo_show_page (cr);

    cairo_destroy (cr);
    cairo_surface_destroy (surface);

    printf ("svg-clip: Please check %s to make sure it looks happy.\n",
	    filename);
    free (filename);
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (svg_clip,
	    "Test SVG clipping",
	    "svg, clip", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
