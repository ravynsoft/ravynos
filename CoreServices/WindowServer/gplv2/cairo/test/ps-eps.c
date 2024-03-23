/*
 * Copyright © 2006 Red Hat, Inc.
 * Copyright © 2009 Adrian Johnson
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
 *         Adrian Johnson <ajohnson@redneon.com>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cairo.h>
#include <cairo-ps.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <errno.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "cairo-test.h"
#include "buffer-diff.h"

/* Test EPS output.
 */

#define WIDTH 595
#define HEIGHT 842

/* Reference Bounding Box */
#define LLX  95
#define LLY 687
#define URX 155
#define URY 747

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

    test_image = target->get_image_surface (surface, 0, WIDTH, HEIGHT);
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
	    WIDTH, HEIGHT);

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


#define DOCUMENT_BBOX  "%%BoundingBox:"
#define PAGE_BBOX      "%%PageBoundingBox:"

static cairo_bool_t
check_bbox (cairo_test_context_t *ctx,
	    const char *base_name)
{
    char *filename;
    FILE *f;
    char buf[256];
    cairo_bool_t bbox_pass, page_bbox_pass;
    int llx, lly, urx, ury;
    int ret;

    xasprintf (&filename,  "%s.out.ps", base_name);
    f = fopen (filename, "r");
    if (!f) {
	cairo_test_log (ctx, "Error: Cannot open EPS output: %s\n",
		        base_name);
	free (filename);
	return FALSE;
    }

    bbox_pass = FALSE;
    page_bbox_pass = FALSE;
    while (!feof(f)) {
	if (fgets (buf, sizeof(buf), f) == (char *)EOF) {
	    cairo_test_log (ctx, "Error: Unexpected EOF in %s\n",
			    filename);
	    break;
	}

	if (strncmp (buf, DOCUMENT_BBOX, strlen (DOCUMENT_BBOX)) == 0) {
	    ret = sscanf (buf+strlen (DOCUMENT_BBOX), "%d %d %d %d", &llx, &lly, &urx, &ury);
	    if (ret == 4 && llx == LLX && lly == LLY && urx == URX && ury == URY)
		bbox_pass = TRUE;
	}

	if (strncmp (buf, PAGE_BBOX, strlen (PAGE_BBOX)) == 0) {
	    ret = sscanf (buf+strlen (PAGE_BBOX), "%d %d %d %d", &llx, &lly, &urx, &ury);
	    if (ret == 4 && llx == LLX && lly == LLY && urx == URX && ury == URY)
		page_bbox_pass = TRUE;
	}
    }
    fclose (f);

    if (!bbox_pass || !page_bbox_pass) {
	cairo_test_log (ctx, "Error: EPS Bounding Box does not match reference Bounding Box\n");
	return FALSE;
    }

    free (filename);

    return TRUE;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_t *cr;
    cairo_test_status_t ret = CAIRO_TEST_UNTESTED;
    unsigned int i;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

    for (i = 0; i < ctx->num_targets; i++) {
	const cairo_boilerplate_target_t *target = ctx->targets_to_test[i];
	cairo_surface_t *surface = NULL;
	char *base_name;
	void *closure;
	const char *format;
	cairo_status_t status;
	cairo_bool_t pass;
	char *test_name;

	if (! cairo_test_is_target_enabled (ctx, target->name))
	    continue;

	format = cairo_boilerplate_content_name (target->content);
	xasprintf (&test_name, "ps-eps");
	xasprintf (&base_name, "%s/ps-eps.%s.%s",
		   path, target->name, format);

	surface = (target->create_surface) (base_name,
					    target->content,
					    WIDTH, HEIGHT,
					    WIDTH, HEIGHT,
					    CAIRO_BOILERPLATE_MODE_TEST,
					    &closure);

	if (surface == NULL) {
	    free (base_name);
	    free (test_name);
	    continue;
	}

	cairo_ps_surface_set_eps (surface, TRUE);
	if (!cairo_ps_surface_get_eps (surface)) {
	    cairo_surface_destroy (surface);
	    if (target->cleanup)
		target->cleanup (closure);

	    free (base_name);
	    free (test_name);
	    continue;
	}
	
	cairo_test_log (ctx,
			"Testing ps-eps with %s target\n",
			target->name);
	printf ("%s:\t", base_name);
	fflush (stdout);

	cairo_surface_set_device_offset (surface, 25, 25);
	cr = cairo_create (surface);

	cairo_new_sub_path (cr);
	cairo_arc (cr, 100, 100, 25, 0, 2*M_PI);
	cairo_set_line_width (cr, 10);
	cairo_stroke (cr);

	cairo_show_page (cr);

	status = cairo_status (cr);
	cairo_destroy (cr);

	if (status) {
	    cairo_test_log (ctx, "Error: Failed to create target surface: %s\n",
			    cairo_status_to_string (status));
	    pass = FALSE;
	} else {
	    pass = TRUE;
	    /* extract the image and compare it to our reference */
	    if (! check_result (ctx, target, test_name, base_name, surface))
		pass = FALSE;

	    /* check the bounding box of the EPS file and compare it to our reference */
	    if (! check_bbox (ctx, base_name))
		pass = FALSE;
	}
	cairo_surface_destroy (surface);
	if (target->cleanup)
	    target->cleanup (closure);

	free (base_name);
	free (test_name);

	if (pass) {
	    printf ("PASS\n");
	    ret = CAIRO_TEST_SUCCESS;
	} else {
	    printf ("FAIL\n");
	    ret = CAIRO_TEST_FAILURE;
	}
	fflush (stdout);
    }

    return ret;
}

CAIRO_TEST (ps_eps,
	    "Check EPS output from PS surface",
	    "ps, api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
