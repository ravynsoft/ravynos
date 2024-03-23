/*
 * Copyright © 2006 Red Hat, Inc.
 * Copyright © 2008 Chris Wilson
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
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>

#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <errno.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "cairo-test.h"
#include "buffer-diff.h"

/* This test exists to test cairo_surface_set_fallback_resolution
 *
 * <behdad> one more thing.
 *          if you can somehow incorporate cairo_show_page stuff in the
 *          test suite.  such that fallback-resolution can actually be
 *          automated..
 *          if we could get a callback on surface when that function is
 *          called, we could do cool stuff like making other backends
 *          draw a long strip of images, one for each page...
 */

#define INCHES_TO_POINTS(in) ((in) * 72.0)
#define SIZE INCHES_TO_POINTS(2)

/* cairo_set_tolerance() is not respected by the PS/PDF backends currently */
#define SET_TOLERANCE 0

#define GENERATE_REFERENCE 0

static void
draw (cairo_t *cr, double width, double height)
{
    const char *text = "cairo";
    cairo_text_extents_t extents;
    const double dash[2] = { 8, 16 };
    cairo_pattern_t *pattern;

    cairo_save (cr);

    cairo_new_path (cr);

    cairo_set_line_width (cr, .05 * SIZE / 2.0);

    cairo_arc (cr, SIZE / 2.0, SIZE / 2.0,
	       0.875 * SIZE / 2.0,
	       0, 2.0 * M_PI);
    cairo_stroke (cr);

    /* use dashes to demonstrate bugs:
     *  https://bugs.freedesktop.org/show_bug.cgi?id=9189
     *  https://bugs.freedesktop.org/show_bug.cgi?id=17223
     */
    cairo_save (cr);
    cairo_set_dash (cr, dash, 2, 0);
    cairo_arc (cr, SIZE / 2.0, SIZE / 2.0,
	       0.75 * SIZE / 2.0,
	       0, 2.0 * M_PI);
    cairo_stroke (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_rectangle (cr, 0, 0, SIZE/2, SIZE);
    cairo_clip (cr);
    cairo_arc (cr, SIZE / 2.0, SIZE / 2.0,
	       0.6 * SIZE / 2.0,
	       0, 2.0 * M_PI);
    cairo_fill (cr);
    cairo_restore (cr);

    /* use a pattern to exercise bug:
     *   https://bugs.launchpad.net/inkscape/+bug/234546
     */
    cairo_save (cr);
    cairo_rectangle (cr, SIZE/2, 0, SIZE/2, SIZE);
    cairo_clip (cr);
    pattern = cairo_pattern_create_linear (SIZE/2, 0, SIZE, 0);
    cairo_pattern_add_color_stop_rgba (pattern, 0, 0, 0, 0, 1.);
    cairo_pattern_add_color_stop_rgba (pattern, 1, 0, 0, 0, 0.);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
    cairo_arc (cr, SIZE / 2.0, SIZE / 2.0,
	       0.6 * SIZE / 2.0,
	       0, 2.0 * M_PI);
    cairo_fill (cr);
    cairo_restore (cr);

    cairo_set_source_rgb (cr, 1, 1, 1); /* white */
    cairo_set_font_size (cr, .25 * SIZE / 2.0);
    cairo_text_extents (cr, text, &extents);
    cairo_move_to (cr, (SIZE-extents.width)/2.0-extents.x_bearing,
		       (SIZE-extents.height)/2.0-extents.y_bearing);
    cairo_show_text (cr, text);

    cairo_restore (cr);
}

static void
_xunlink (const cairo_test_context_t *ctx, const char *pathname)
{
    if (unlink (pathname) < 0 && errno != ENOENT) {
	cairo_test_log (ctx, "Error: Cannot remove %s: %s\n",
			pathname, strerror (errno));
	exit (1);
    }
}

static cairo_bool_t
check_result (cairo_test_context_t *ctx,
	      const cairo_boilerplate_target_t *target,
	      const char *test_name,
	      const char *base_name,
	      cairo_surface_t *surface)
{
    const char *format;
    char *ref_name;
    char *png_name;
    char *diff_name;
    cairo_surface_t *test_image, *ref_image, *diff_image;
    buffer_diff_result_t result;
    cairo_status_t status;
    cairo_bool_t ret;

    /* XXX log target, OUTPUT, REFERENCE, DIFFERENCE for index.html */

    if (target->finish_surface != NULL) {
	status = target->finish_surface (surface);
	if (status) {
	    cairo_test_log (ctx, "Error: Failed to finish surface: %s\n",
		    cairo_status_to_string (status));
	    cairo_surface_destroy (surface);
	    return FALSE;
	}
    }

    xasprintf (&png_name,  "%s.out.png", base_name);
    xasprintf (&diff_name, "%s.diff.png", base_name);

    test_image = target->get_image_surface (surface, 0, SIZE, SIZE);
    if (cairo_surface_status (test_image)) {
	cairo_test_log (ctx, "Error: Failed to extract page: %s\n",
		        cairo_status_to_string (cairo_surface_status (test_image)));
	cairo_surface_destroy (test_image);
	free (png_name);
	free (diff_name);
	return FALSE;
    }

    _xunlink (ctx, png_name);
    status = cairo_surface_write_to_png (test_image, png_name);
    if (status) {
	cairo_test_log (ctx, "Error: Failed to write output image: %s\n",
		cairo_status_to_string (status));
	cairo_surface_destroy (test_image);
	free (png_name);
	free (diff_name);
	return FALSE;
    }

    format = cairo_boilerplate_content_name (target->content);
    ref_name = cairo_test_reference_filename (ctx,
					      base_name,
					      test_name,
					      target->name,
					      target->basename,
					      format,
					      CAIRO_TEST_REF_SUFFIX,
					      CAIRO_TEST_PNG_EXTENSION);
    if (ref_name == NULL) {
	cairo_test_log (ctx, "Error: Cannot find reference image for %s\n",
		        base_name);
	cairo_surface_destroy (test_image);
	free (png_name);
	free (diff_name);
	return FALSE;
    }


    ref_image = cairo_test_get_reference_image (ctx, ref_name,
	    target->content == CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED);
    if (cairo_surface_status (ref_image)) {
	cairo_test_log (ctx, "Error: Cannot open reference image for %s: %s\n",
		        ref_name,
		cairo_status_to_string (cairo_surface_status (ref_image)));
	cairo_surface_destroy (ref_image);
	cairo_surface_destroy (test_image);
	free (png_name);
	free (diff_name);
	free (ref_name);
	return FALSE;
    }

    diff_image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
	    SIZE, SIZE);

    ret = TRUE;
    status = image_diff (ctx,
	    test_image, ref_image, diff_image,
	    &result);
    _xunlink (ctx, diff_name);
    if (status) {
	cairo_test_log (ctx, "Error: Failed to compare images: %s\n",
			cairo_status_to_string (status));
	ret = FALSE;
    } else if (image_diff_is_failure (&result, target->error_tolerance))
    {
	ret = FALSE;

	status = cairo_surface_write_to_png (diff_image, diff_name);
	if (status) {
	    cairo_test_log (ctx, "Error: Failed to write differences image: %s\n",
		    cairo_status_to_string (status));
	}
    }

    cairo_surface_destroy (test_image);
    cairo_surface_destroy (diff_image);
    free (png_name);
    free (diff_name);
    free (ref_name);

    return ret;
}

#if GENERATE_REFERENCE
static void
generate_reference (double ppi_x, double ppi_y, const char *filename)
{
    cairo_surface_t *surface, *target;
    cairo_t *cr;
    cairo_status_t status;

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
	                                  SIZE*ppi_x/72, SIZE*ppi_y/72);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    /* As we wish to mimic a PDF surface, copy across the default font options
     * from the PDF backend.
     */
    {
	cairo_surface_t *pdf;
	cairo_font_options_t *options;

	options = cairo_font_options_create ();

#if CAIRO_HAS_PDF_SURFACE
	pdf = cairo_pdf_surface_create ("tmp.pdf", 1, 1);
	cairo_surface_get_font_options (pdf, options);
	cairo_surface_destroy (pdf);
#endif

	cairo_set_font_options (cr, options);
	cairo_font_options_destroy (options);
    }

#if SET_TOLERANCE
    cairo_set_tolerance (cr, 3.0);
#endif

    cairo_save (cr); {
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_paint (cr);
    } cairo_restore (cr);

    cairo_scale (cr, ppi_x/72., ppi_y/72.);
    draw (cr, SIZE, SIZE);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    target = cairo_image_surface_create (CAIRO_FORMAT_RGB24, SIZE, SIZE);
    cr = cairo_create (target);
    cairo_scale (cr, 72./ppi_x, 72./ppi_y);
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);

    status = cairo_surface_write_to_png (cairo_get_target (cr), filename);
    cairo_destroy (cr);

    if (status) {
	fprintf (stderr, "Failed to generate reference image '%s': %s\n",
		 filename, cairo_status_to_string (status));
	exit (1);
    }
}
#endif

/* TODO: Split each ppi case out to its own CAIRO_TEST() test case */
static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_t *cr;
    cairo_test_status_t ret = CAIRO_TEST_UNTESTED;
    struct {
	double x, y;
    } ppi[] = {
	{ 576, 576 },
	{ 576, 72 },

	{ 288, 288 },
	{ 288, 72 },

	{ 144, 144 },
	{ 144, 72 },

	{ 72, 576 },
	{ 72, 288 },
	{ 72, 144 },
	{ 72, 72 },
    };
    unsigned int i;
    int n, num_ppi;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

    num_ppi = ARRAY_LENGTH (ppi);

#if GENERATE_REFERENCE
    for (n = 0; n < num_ppi; n++) {
	char *ref_name;
	xasprintf (&ref_name, "reference/fallback-resolution.ppi%gx%g.ref.png",
		   ppi[n].x, ppi[n].y);
	generate_reference (ppi[n].x, ppi[n].y, ref_name);
	free (ref_name);
    }
#endif

    for (i = 0; i < ctx->num_targets; i++) {
	const cairo_boilerplate_target_t *target = ctx->targets_to_test[i];
	cairo_surface_t *surface = NULL;
	char *base_name;
	void *closure;
	const char *format;
	cairo_status_t status;

	if (! target->is_vector)
	    continue;

	if (! cairo_test_is_target_enabled (ctx, target->name))
	    continue;

	format = cairo_boilerplate_content_name (target->content);
	xasprintf (&base_name, "%s/fallback-resolution.%s.%s",
		   path, target->name,
		   format);

	surface = (target->create_surface) (base_name,
					    target->content,
					    SIZE, SIZE,
					    SIZE, SIZE,
					    CAIRO_BOILERPLATE_MODE_TEST,
					    &closure);

	if (surface == NULL) {
	    free (base_name);
	    continue;
	}

	if (ret == CAIRO_TEST_UNTESTED)
	    ret = CAIRO_TEST_SUCCESS;

	cairo_surface_destroy (surface);
	if (target->cleanup)
	    target->cleanup (closure);
	free (base_name);

	/* we need to recreate the surface for each resolution as we include
	 * SVG in testing which does not support the paginated interface.
	 */
	for (n = 0; n < num_ppi; n++) {
	    char *test_name;
	    cairo_bool_t pass;

	    xasprintf (&test_name, "fallback-resolution.ppi%gx%g",
		       ppi[n].x, ppi[n].y);
	    xasprintf (&base_name, "%s/%s.%s.%s",
		       path, test_name,
		       target->name,
		       format);

	    surface = (target->create_surface) (base_name,
						target->content,
						SIZE + 25, SIZE + 25,
						SIZE + 25, SIZE + 25,
						CAIRO_BOILERPLATE_MODE_TEST,
						&closure);
	    if (surface == NULL || cairo_surface_status (surface)) {
		cairo_test_log (ctx, "Failed to generate surface: %s.%s\n",
				target->name,
				format);
		free (base_name);
		free (test_name);
		ret = CAIRO_TEST_FAILURE;
		continue;
	    }

	    cairo_test_log (ctx,
			    "Testing fallback-resolution %gx%g with %s target\n",
			    ppi[n].x, ppi[n].y, target->name);
	    printf ("%s:\t", base_name);
	    fflush (stdout);

	    if (target->force_fallbacks != NULL)
		target->force_fallbacks (surface, ppi[n].x, ppi[n].y);
	    cr = cairo_create (surface);
#if SET_TOLERANCE
	    cairo_set_tolerance (cr, 3.0);
#endif

	    cairo_surface_set_device_offset (surface, 25, 25);

	    cairo_save (cr); {
		cairo_set_source_rgb (cr, 1, 1, 1);
		cairo_paint (cr);
	    } cairo_restore (cr);

	    /* First draw the top half in a conventional way. */
	    cairo_save (cr); {
		cairo_rectangle (cr, 0, 0, SIZE, SIZE / 2.0);
		cairo_clip (cr);

		draw (cr, SIZE, SIZE);
	    } cairo_restore (cr);

	    /* Then draw the bottom half in a separate group,
	     * (exposing a bug in 1.6.4 with the group not being
	     * rendered with the correct fallback resolution). */
	    cairo_save (cr); {
		cairo_rectangle (cr, 0, SIZE / 2.0, SIZE, SIZE / 2.0);
		cairo_clip (cr);

		cairo_push_group (cr); {
		    draw (cr, SIZE, SIZE);
		} cairo_pop_group_to_source (cr);

		cairo_paint (cr);
	    } cairo_restore (cr);

	    status = cairo_status (cr);
	    cairo_destroy (cr);

	    pass = FALSE;
	    if (status) {
		cairo_test_log (ctx, "Error: Failed to create target surface: %s\n",
				cairo_status_to_string (status));
		ret = CAIRO_TEST_FAILURE;
	    } else {
		/* extract the image and compare it to our reference */
		if (! check_result (ctx, target, test_name, base_name, surface))
		    ret = CAIRO_TEST_FAILURE;
		else
		    pass = TRUE;
	    }
	    cairo_surface_destroy (surface);
	    if (target->cleanup)
		target->cleanup (closure);

	    free (base_name);
	    free (test_name);

	    if (pass) {
		printf ("PASS\n");
	    } else {
		printf ("FAIL\n");
	    }
	    fflush (stdout);
	}
    }

    return ret;
}

CAIRO_TEST (fallback_resolution,
	    "Check handling of fallback resolutions",
	    "fallback", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
