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
 * Author: Carl Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#include <stdlib.h>

#define WIDTH 2
#define HEIGHT 2

static cairo_status_t
no_memory_error (void *closure, unsigned char *data, unsigned int size)
{
    return CAIRO_STATUS_NO_MEMORY;
}

static cairo_status_t
read_error (void *closure, unsigned char *data, unsigned int size)
{
    return CAIRO_STATUS_READ_ERROR;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    char *filename;
    cairo_surface_t *surface;

    xasprintf (&filename, "%s/reference/%s",
	       ctx->srcdir, "create-from-png.ref.png");

    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	cairo_test_status_t result;

	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}

	free (filename);
	return result;
    }

    /* Pretend we modify the surface data (which detaches the PNG mime data) */
    cairo_surface_flush (surface);
    cairo_surface_mark_dirty (surface);

    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
    cairo_paint (cr);

    cairo_surface_destroy (surface);

    free (filename);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    char *filename;
    char *path;
    cairo_surface_t *surface;
    cairo_status_t status;
    cairo_test_status_t result = CAIRO_TEST_SUCCESS;

    surface = cairo_image_surface_create_from_png ("___THIS_FILE_DOES_NOT_EXIST___");
    if (cairo_surface_status (surface) != CAIRO_STATUS_FILE_NOT_FOUND) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error: expected \"file not found\", but got: %s\n",
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    }
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    surface = cairo_image_surface_create_from_png_stream (no_memory_error, NULL);
    if (cairo_surface_status (surface) != CAIRO_STATUS_NO_MEMORY) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error: expected \"out of memory\", but got: %s\n",
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    }
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    surface = cairo_image_surface_create_from_png_stream (read_error, NULL);
    if (cairo_surface_status (surface) != CAIRO_STATUS_READ_ERROR) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error: expected \"read error\", but got: %s\n",
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    }
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    /* cheekily test error propagation from the user write funcs as well ... */
    xasprintf (&path, "%s/reference", ctx->srcdir);
    xasprintf (&filename, "%s/%s", path, "create-from-png.ref.png");

    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    } else {
	status = cairo_surface_write_to_png_stream (surface,
					       (cairo_write_func_t) no_memory_error,
					       NULL);
	if (status != CAIRO_STATUS_NO_MEMORY) {
	    result = cairo_test_status_from_status (ctx, status);
	    if (result == CAIRO_TEST_FAILURE) {
		cairo_test_log (ctx, "Error: expected \"out of memory\", but got: %s\n",
				cairo_status_to_string (status));
	    }
	}

	status = cairo_surface_write_to_png_stream (surface,
						    (cairo_write_func_t) read_error,
						    NULL);
	if (status != CAIRO_STATUS_READ_ERROR) {
	    result = cairo_test_status_from_status (ctx, status);
	    if (result == CAIRO_TEST_FAILURE) {
		cairo_test_log (ctx, "Error: expected \"read error\", but got: %s\n",
				cairo_status_to_string (status));
	    }
	}

	/* and check that error has not propagated to the surface */
	if (cairo_surface_status (surface)) {
	    result = cairo_test_status_from_status (ctx,
						    cairo_surface_status (surface));
	    if (result == CAIRO_TEST_FAILURE) {
		cairo_test_log (ctx, "Error: user write error propagated to surface: %s",
				cairo_status_to_string (cairo_surface_status (surface)));
	    }
	}
    }
    cairo_surface_destroy (surface);
    free (filename);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    /* check that loading alpha/opaque PNGs generate the correct surfaces */
    /* TODO: Avoid using target-specific references as sample images */
    xasprintf (&filename, "%s/%s", path, "create-from-png.alpha.ref.png");
    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    } else if (cairo_image_surface_get_format (surface) != CAIRO_FORMAT_ARGB32) {
	cairo_test_log (ctx, "Error reading PNG image %s: did not create an ARGB32 image\n",
			filename);
	result = CAIRO_TEST_FAILURE;
    }
    free (filename);
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    xasprintf (&filename, "%s/%s", path, "create-from-png.ref.png");
    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    } else if (cairo_image_surface_get_format (surface) != CAIRO_FORMAT_RGB24) {
	cairo_test_log (ctx, "Error reading PNG image %s: did not create an RGB24 image\n",
			filename);
	result = CAIRO_TEST_FAILURE;
    }
    free (filename);
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    /* check paletted PNGs */
    /* TODO: Avoid using target-specific references as sample images */
    xasprintf (&filename, "%s/%s", path, "create-from-png.indexed-alpha.ref.png");
    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    } else if (cairo_image_surface_get_format (surface) != CAIRO_FORMAT_ARGB32) {
	cairo_test_log (ctx, "Error reading PNG image %s: did not create an ARGB32 image\n",
			filename);
	result = CAIRO_TEST_FAILURE;
    }
    free (filename);
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    /* TODO: Avoid using target-specific references as sample images */
    xasprintf (&filename, "%s/%s", path, "create-from-png.indexed.ref.png");
    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    } else if (cairo_image_surface_get_format (surface) != CAIRO_FORMAT_RGB24) {
	cairo_test_log (ctx, "Error reading PNG image %s: did not create an RGB24 image\n",
			filename);
	result = CAIRO_TEST_FAILURE;
    }
    free (filename);
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    /* check grayscale PNGs */
    /* TODO: Avoid using target-specific references as sample images */
    xasprintf (&filename, "%s/%s", path, "create-from-png.gray-alpha.ref.png");
    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    } else if (cairo_image_surface_get_format (surface) != CAIRO_FORMAT_ARGB32) {
	cairo_test_log (ctx, "Error reading PNG image %s: did not create an ARGB32 image\n",
			filename);
	result = CAIRO_TEST_FAILURE;
    }
    free (filename);
    cairo_surface_destroy (surface);
    if (result != CAIRO_TEST_SUCCESS)
	return result;

    /* TODO: Avoid using target-specific references as sample images */
    xasprintf (&filename, "%s/%s", path, "create-from-png.gray.ref.png");
    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface)) {
	result = cairo_test_status_from_status (ctx,
						cairo_surface_status (surface));
	if (result == CAIRO_TEST_FAILURE) {
	    cairo_test_log (ctx, "Error reading PNG image %s: %s\n",
			    filename,
			    cairo_status_to_string (cairo_surface_status (surface)));
	}
    } else if (cairo_image_surface_get_format (surface) != CAIRO_FORMAT_RGB24) {
	cairo_test_log (ctx, "Error reading PNG image %s: did not create an RGB24 image\n",
			filename);
	result = CAIRO_TEST_FAILURE;
    }
    free (filename);
    cairo_surface_destroy (surface);

    free (path);

    return result;
}

CAIRO_TEST (create_from_png,
	    "Tests the creation of an image surface from a PNG file",
	    "png", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    preamble, draw)
