/*
 * Copyright © 2023 Adrian Johnson
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
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

#include "config.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>

#if CAIRO_HAS_PDF_SURFACE && CAIRO_HAS_PS_SURFACE && CAIRO_HAS_SVG_SURFACE

#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <errno.h>
#endif

#include "cairo-test.h"
#include "buffer-diff.h"

/* This test is to ensure that _cairo_recording_surface_replay_and_create_regions()
 * works with a recording surface source re-used for multiple paginated surfaces.
 * The prior use of the source with a paginated surface should not affect the region
 * analysis on subsequent surfaces.
 *
 * If test output should only contain fallback images for unsupported
 * operations.  If the recording surface is incorrectly re-using the
 * analysis from a different target, some operations may be missing
 * from the ouput (recording surface marked the operation as supported
 * when it is not) or some operations may be have fallbacks for
 * natively suported opetions (recording surface marked a supported
 * operation as unsupprted).
 *
 * To create ref images, run the test for one target at a time to
 * prevent re-use of the recording surface for different targets.
 */


#define SIZE 40
#define PAD 25
#define PAGE_SIZE (SIZE*3 + PAD*4)

/* Apply a slight rotation and use a very low fallback resolution to
 * ensure fallback images are apparent in the output. */
#define ROTATE 5
#define FALLBACK_PPI 18

static void
create_recordings (cairo_operator_t    op,
                   cairo_surface_t   **recording,
                   cairo_surface_t   **recording_group)
{
    *recording = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
    cairo_t *cr = cairo_create (*recording);

    cairo_rotate (cr, ROTATE*M_PI/180.0);

    cairo_rectangle (cr, 0, 0, SIZE*3.0/4.0, SIZE*3.0/4.0);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill (cr);

    cairo_set_operator (cr, op);

    cairo_rectangle (cr, SIZE/4.0, SIZE/4.0, SIZE*3.0/4.0, SIZE*3.0/4.0);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_fill (cr);

    cairo_destroy (cr);

    *recording_group = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
    cr = cairo_create (*recording_group);

    cairo_set_source_surface (cr, *recording, 0, 0);
    cairo_paint (cr);

    cairo_destroy (cr);
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

    test_image = target->get_image_surface (surface, 0, PAGE_SIZE, PAGE_SIZE);
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
	    PAGE_SIZE, PAGE_SIZE);

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

#define TEST_ROWS 2
#define TEST_COLS 3

static void
draw (cairo_t *cr, cairo_surface_t *recordings[TEST_ROWS][TEST_COLS])
{
    cairo_translate (cr, PAD, PAD);
    for (int row = 0; row < TEST_ROWS; row++) {
        cairo_save (cr);
        for (int col = 0; col < TEST_COLS; col++) {
            cairo_save (cr);
            cairo_set_source_surface (cr, recordings[row][col], 0, 0);
            cairo_paint (cr);
            cairo_restore (cr);
            cairo_translate (cr, SIZE + PAD, 0);
        }
        cairo_restore (cr);
        cairo_translate (cr, 0, SIZE + PAD);
    }
}

#define NUM_TEST_SURFACES 3

typedef struct _test_surface {
    const cairo_boilerplate_target_t *target;
    cairo_surface_t *surface;
    char *base_name;
    void *closure;
} test_surface_t;

static test_surface_t test_surfaces[NUM_TEST_SURFACES];

static void
init_test_surfaces()
{
    memset (test_surfaces, 0, sizeof (*test_surfaces));
    test_surfaces[0].surface = cairo_pdf_surface_create_for_stream (NULL, NULL, PAGE_SIZE, PAGE_SIZE);
    test_surfaces[1].surface = cairo_ps_surface_create_for_stream (NULL, NULL, PAGE_SIZE, PAGE_SIZE);
    test_surfaces[2].surface = cairo_svg_surface_create_for_stream (NULL, NULL, PAGE_SIZE, PAGE_SIZE);
}

static void
add_test_surface (const cairo_boilerplate_target_t *target,
                  cairo_surface_t *surface,
                  char *base_name,
                  void *closure)
{
    for (int i = 0; i < NUM_TEST_SURFACES; i++) {
        if (cairo_surface_get_type (test_surfaces[i].surface) == cairo_surface_get_type (surface)) {
            cairo_surface_destroy (test_surfaces[i].surface);
            test_surfaces[i].target = target;
            test_surfaces[i].surface = surface;
            test_surfaces[i].base_name = base_name;
            test_surfaces[i].closure = closure;
            break;
        }
    }
}

static void
destroy_test_surfaces()
{
    for (int i = 0; i < NUM_TEST_SURFACES; i++) {
        cairo_surface_destroy (test_surfaces[i].surface);
        if (test_surfaces[i].target && test_surfaces[i].target->cleanup)
            test_surfaces[i].target->cleanup (test_surfaces[i].closure);
        if (test_surfaces[i].base_name)
            free (test_surfaces[i].base_name);
    }
}


static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_t *cr;
    cairo_test_status_t ret = CAIRO_TEST_UNTESTED;
    unsigned int i;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";
    cairo_surface_t *recordings[TEST_ROWS][TEST_COLS];
    const char *test_name = "create-regions";
    char *base_name;

    /* Each row displays three recordings. One with operations
     * supported by all paginated surfaces (OVER), one with operations
     * supported by PDF but not PS or SVG surfaces (DIFFERENCE), and
     * one with operations supported by SVG but not PDF or PS
     * (DEST_XOR).
     *
     * The recordings for the first row is a single recording. The
     * recordings for the second row contains the first row recording
     * inside another recording to test the use of cloned recording
     * surfaces.
     *
     * We are looking to see that fallback images are only used for
     * unsupported operations.
     */
    create_recordings (CAIRO_OPERATOR_OVER,       &recordings[0][0], &recordings[1][0]);
    create_recordings (CAIRO_OPERATOR_DIFFERENCE, &recordings[0][1], &recordings[1][1]);
    create_recordings (CAIRO_OPERATOR_XOR,        &recordings[0][2], &recordings[1][2]);

    init_test_surfaces();
    for (i = 0; i < ctx->num_targets; i++) {
	const cairo_boilerplate_target_t *target = ctx->targets_to_test[i];
	cairo_surface_t *surface = NULL;
        void *closure;
	const char *format;

        /* This test only works on surfaces that support fine grained fallbacks */
        if (! (target->expected_type == CAIRO_SURFACE_TYPE_PDF ||
               target->expected_type == CAIRO_SURFACE_TYPE_PS ||
               target->expected_type ==CAIRO_SURFACE_TYPE_SVG))
	    continue;

	if (! cairo_test_is_target_enabled (ctx, target->name))
	    continue;

	if (ret == CAIRO_TEST_UNTESTED)
	    ret = CAIRO_TEST_SUCCESS;

	format = cairo_boilerplate_content_name (target->content);
        base_name = NULL;
        xasprintf (&base_name, "%s/%s.%s.%s",
                   path, test_name,
                   target->name,
                   format);

        surface = (target->create_surface) (base_name,
                                            target->content,
                                            PAGE_SIZE, PAGE_SIZE,
                                            PAGE_SIZE, PAGE_SIZE,
                                            CAIRO_BOILERPLATE_MODE_TEST,
                                            &closure);
        if (surface == NULL || cairo_surface_status (surface)) {
            cairo_test_log (ctx, "Failed to generate surface: %s.%s\n",
                            target->name,
                            format);
            ret = CAIRO_TEST_FAILURE;
            break;
        }

        cairo_surface_set_fallback_resolution (surface, FALLBACK_PPI, FALLBACK_PPI);
        add_test_surface (target, surface, base_name, closure);
    }
 
    for (int i = 0; i < NUM_TEST_SURFACES; i++) {
	cairo_status_t status;

        if (test_surfaces[i].target != NULL) {
            cairo_test_log (ctx,
                            "Testing create-regions with %s target\n",
                            test_surfaces[i].target->name);
            printf ("%s:\t", test_surfaces[i].base_name);
            fflush (stdout);
        }

        cr = cairo_create (test_surfaces[i].surface);

        draw (cr, recordings);

        status = cairo_status (cr);
        cairo_destroy (cr);
        cairo_surface_finish (test_surfaces[i].surface);

        if (test_surfaces[i].target) {
            cairo_bool_t pass = FALSE;
            if (status) {
                cairo_test_log (ctx, "Error: Failed to create target surface: %s\n",
                                cairo_status_to_string (status));
                ret = CAIRO_TEST_FAILURE;
            } else {
                /* extract the image and compare it to our reference */
                if (! check_result (ctx, test_surfaces[i].target, test_name, test_surfaces[i].base_name, test_surfaces[i].surface))
                    ret = CAIRO_TEST_FAILURE;
                else
                    pass = TRUE;
            }

            if (pass) {
                printf ("PASS\n");
            } else {
                printf ("FAIL\n");
            }
            fflush (stdout);
        }
    }

    destroy_test_surfaces();

    for (int row = 0; row < TEST_ROWS; row++) {
        for (int col = 0; col < TEST_COLS; col++) {
            cairo_surface_destroy (recordings[row][col]);
        }
    }

    return ret;
}

CAIRO_TEST (create_regions,
	    "Check region analysis when re-used with different surfaces",
	    "fallback", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)

#endif /* CAIRO_HAS_PDF_SURFACE && CAIRO_HAS_PS_SURFACE && CAIRO_HAS_SVG_SURFACE */
