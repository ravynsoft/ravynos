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
#include <stdio.h>
#include <errno.h>

#define WIDTH 2
#define HEIGHT 2

static cairo_status_t
read_png_from_file (void *closure, unsigned char *data, unsigned int length)
{
    FILE *file = closure;
    size_t bytes_read;

    bytes_read = fread (data, 1, length, file);
    if (bytes_read != length)
	return CAIRO_STATUS_READ_ERROR;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    char *filename;
    FILE *file;
    cairo_surface_t *surface;
    cairo_status_t status;

    xasprintf (&filename, "%s/reference/%s", ctx->srcdir,
	       "create-from-png-stream.ref.png");

    file = fopen (filename, "rb");
    if (file == NULL) {
	cairo_test_status_t ret;

	ret = CAIRO_TEST_FAILURE;
	if (errno == ENOMEM)
	    ret = cairo_test_status_from_status (ctx, CAIRO_STATUS_NO_MEMORY);

	if (ret != CAIRO_TEST_NO_MEMORY)
	    cairo_test_log (ctx, "Error: failed to open file: %s\n", filename);

	free (filename);
	return ret;
    }

    surface = cairo_image_surface_create_from_png_stream (read_png_from_file,
							  file);

    fclose (file);

    status = cairo_surface_status (surface);
    if (status) {
	cairo_test_status_t ret;

	cairo_surface_destroy (surface);

	ret = cairo_test_status_from_status (ctx, status);
	if (ret != CAIRO_TEST_NO_MEMORY) {
	    cairo_test_log (ctx,
			    "Error: failed to create surface from PNG: %s - %s\n",
			    filename,
			    cairo_status_to_string (status));
	}

	free (filename);

	return ret;
    }

    free (filename);

    /* Pretend we modify the surface data (which detaches the PNG mime data) */
    cairo_surface_flush (surface);
    cairo_surface_mark_dirty (surface);

    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
    cairo_paint (cr);

    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (create_from_png_stream,
	    "Tests the creation of an image surface from a PNG using a FILE *",
	    "png", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
