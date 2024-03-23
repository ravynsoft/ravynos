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

#include <cairo.h>

#if CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif

#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif

/* The PostScript and PDF backends are now integrated into the main
 * test suite, so we are getting good verification of most things
 * there.
 *
 * One thing that isn't supported there yet is multi-page output. So,
 * for now we have this one-off test. There's no automatic
 * verififcation here yet, but you can manually view the output to
 * make sure it looks happy.
 */

#define WIDTH_IN_INCHES  3
#define HEIGHT_IN_INCHES 3
#define WIDTH_IN_POINTS  (WIDTH_IN_INCHES  * 72.0)
#define HEIGHT_IN_POINTS (HEIGHT_IN_INCHES * 72.0)
#define BASENAME         "multi-page.out"

static void
draw_smiley (cairo_t *cr, double width, double height, double smile_ratio)
{
#define STROKE_WIDTH .04
    double size;

    double theta = M_PI / 4 * smile_ratio;
    double dx = sqrt (0.005) * cos (theta);
    double dy = sqrt (0.005) * sin (theta);

    cairo_save (cr);

    if (width > height)
	size = height;
    else
	size = width;

    cairo_translate (cr, (width - size) / 2.0, (height - size) / 2.0);
    cairo_scale (cr, size, size);

    /* Fill face */
    cairo_arc (cr, 0.5, 0.5, 0.5 - STROKE_WIDTH, 0, 2 * M_PI);
    cairo_set_source_rgb (cr, 1, 1, 0);
    cairo_fill_preserve (cr);

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
    cairo_move_to (cr,
		   0.35 - dx, 0.75 - dy);
    cairo_curve_to (cr,
		    0.35 + dx, 0.75 + dy,
		    0.65 - dx, 0.75 + dy,
		    0.65 + dx, 0.75 - dy);
    cairo_stroke (cr);

    cairo_restore (cr);
}

static void
draw_some_pages (cairo_surface_t *surface)
{
    cairo_t *cr;
    int i;

    cr = cairo_create (surface);

#define NUM_FRAMES 5
    for (i=0; i < NUM_FRAMES; i++) {
	draw_smiley (cr, WIDTH_IN_POINTS, HEIGHT_IN_POINTS,
	             (double) i / (NUM_FRAMES - 1));

	/* Duplicate the last frame onto another page. (This is just a
	 * way to sneak cairo_copy_page into the test).
	 */
	if (i == (NUM_FRAMES - 1))
	    cairo_copy_page (cr);

	cairo_show_page (cr);
    }

    cairo_destroy (cr);
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t *surface;
    cairo_status_t status;
    char *filename;
    cairo_test_status_t result = CAIRO_TEST_UNTESTED;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

#if CAIRO_HAS_PS_SURFACE
    if (cairo_test_is_target_enabled (ctx, "ps2") ||
        cairo_test_is_target_enabled (ctx, "ps3"))
    {
	if (result == CAIRO_TEST_UNTESTED)
	    result = CAIRO_TEST_SUCCESS;

	xasprintf (&filename, "%s/%s", path, BASENAME ".ps");
	surface = cairo_ps_surface_create (filename,
					   WIDTH_IN_POINTS, HEIGHT_IN_POINTS);
	status = cairo_surface_status (surface);
	if (status) {
	    cairo_test_log (ctx, "Failed to create ps surface for file %s: %s\n",
			    filename, cairo_status_to_string (status));
	    result = CAIRO_TEST_FAILURE;
	}

	draw_some_pages (surface);

	cairo_surface_destroy (surface);

	printf ("multi-page: Please check %s to ensure it looks happy.\n", filename);
	free (filename);
    }
#endif

#if CAIRO_HAS_PDF_SURFACE
    if (cairo_test_is_target_enabled (ctx, "pdf")) {
	if (result == CAIRO_TEST_UNTESTED)
	    result = CAIRO_TEST_SUCCESS;

	xasprintf (&filename, "%s/%s", path, BASENAME ".pdf");
	surface = cairo_pdf_surface_create (filename,
					    WIDTH_IN_POINTS, HEIGHT_IN_POINTS);
	status = cairo_surface_status (surface);
	if (status) {
	    cairo_test_log (ctx, "Failed to create pdf surface for file %s: %s\n",
			    filename, cairo_status_to_string (status));
	    result = CAIRO_TEST_FAILURE;
	}

	draw_some_pages (surface);

	cairo_surface_destroy (surface);

	printf ("multi-page: Please check %s to ensure it looks happy.\n", filename);
	free (filename);
    }
#endif

    return result;
}

CAIRO_TEST (multi_page,
	    "Check the paginated surfaces handle multiple pages.",
	    "paginated", /* keywords */
	    "target=vector", /* requirements */
	    0, 0,
	    preamble, NULL)
