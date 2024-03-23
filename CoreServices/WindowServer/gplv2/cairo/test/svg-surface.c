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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#include <stdio.h>

#include <cairo-svg.h>

/* Pretty boring test just to make sure things aren't crashing ---
 * no verification that we're getting good results yet.
 * But you can manually view the image to make sure it looks happy.
 */

#define WIDTH_IN_INCHES  3
#define HEIGHT_IN_INCHES 3
#define WIDTH_IN_POINTS  (WIDTH_IN_INCHES  * 72)
#define HEIGHT_IN_POINTS (HEIGHT_IN_INCHES * 72)
#define BASENAME "svg-surface.out"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
#define STROKE_WIDTH .04

    double size;

    if (width > height)
	size = height;
    else
	size = width;

    cairo_translate (cr, (width - size) / 2.0, (height - size) / 2.0);
    cairo_scale (cr, size, size);

    /* Fill face */
    cairo_arc (cr, 0.5, 0.5, 0.5 - STROKE_WIDTH, 0, 2 * M_PI);
    cairo_set_source_rgb (cr, 1, 1, 0);
    cairo_save (cr);
    {
	cairo_fill (cr);
    }
    cairo_restore (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);

    /* Stroke face */
    cairo_set_line_width (cr, STROKE_WIDTH / 2.0);
    cairo_stroke (cr);

    /* Eyes */
    cairo_set_line_width (cr, STROKE_WIDTH);
    cairo_arc (cr, 0.3, 0.4, STROKE_WIDTH, 0, 2 * M_PI);
    cairo_fill (cr);
    cairo_arc (cr, 0.7, 0.4, STROKE_WIDTH, 0, 2 * M_PI);
    cairo_fill (cr);

    /* Mouth */
    cairo_move_to (cr, 0.3, 0.7);
    cairo_curve_to (cr,
		    0.4, 0.8,
		    0.6, 0.8,
		    0.7, 0.7);
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
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
			filename,
			cairo_status_to_string (cairo_surface_status (surface)));
	free (filename);
	return CAIRO_TEST_FAILURE;
    }

    cr = cairo_create (surface);

    draw (cr, WIDTH_IN_POINTS, HEIGHT_IN_POINTS);

    cairo_show_page (cr);

    cairo_destroy (cr);
    cairo_surface_destroy (surface);

    printf ("svg-surface: Please check %s to make sure it looks happy.\n", filename);
    free (filename);
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (svg_surface,
	    "Check creation of a SVG surface",
	    "svg", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
