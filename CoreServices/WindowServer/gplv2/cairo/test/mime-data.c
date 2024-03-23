/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#include <stdio.h>
#include <errno.h>

/* Basic test to exercise the new mime-data embedding. */

static cairo_status_t
read_file (const cairo_test_context_t *ctx,
	   const char *filename,
	   unsigned char **data_out,
	   unsigned int *length_out)
{
    FILE *file;
    unsigned char *buf;
    unsigned int len;

    file = fopen (filename, "rb");
    if (file == NULL) {
	char path[4096];

	if (errno == ENOMEM)
	    return CAIRO_STATUS_NO_MEMORY;

	/* try again with srcdir */
	snprintf (path, sizeof (path),
		  "%s/%s", ctx->srcdir, filename);
	file = fopen (path, "rb");
    }
    if (file == NULL) {
	switch (errno) {
	case ENOMEM:
	    return CAIRO_STATUS_NO_MEMORY;
	default:
	    return CAIRO_STATUS_FILE_NOT_FOUND;
	}
    }

    fseek (file, 0, SEEK_END);
    len = ftell (file);
    fseek (file, 0, SEEK_SET);

    buf = xmalloc (len);
    *length_out = fread (buf, 1, len, file);
    fclose (file);
    if (*length_out != len) {
	free (buf);
	return CAIRO_STATUS_READ_ERROR;
    }

    *data_out = buf;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_test_status_t
paint_file (cairo_t *cr,
	    const char *filename, const char *mime_type,
	    int x, int y)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *image;
    unsigned char *mime_data;
    unsigned int mime_length;
    cairo_status_t status;

    /* Deliberately use a non-matching MIME images, so that we can identify
     * when the MIME representation is used in preference to the plain image
     * surface.
     */
    status = read_file (ctx, filename, &mime_data, &mime_length);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 200, 50);

    status = cairo_surface_set_mime_data (image, mime_type,
					  mime_data, mime_length,
					  free, mime_data);
    if (status) {
	cairo_surface_destroy (image);
	free (mime_data);
	return cairo_test_status_from_status (ctx, status);
    }

    cairo_set_source_surface (cr, image, x, y);
    cairo_surface_destroy (image);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
paint_jbig2_file (cairo_t *cr, int x, int y)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *image;
    unsigned char *mime_data;
    unsigned int mime_length;
    cairo_status_t status;
    const char jbig2_image1_filename[] = "image1.jb2";
    const char jbig2_image2_filename[] = "image2.jb2";
    const char jbig2_global_filename[] = "global.jb2";

    /* Deliberately use a non-matching MIME images, so that we can identify
     * when the MIME representation is used in preference to the plain image
     * surface.
     */

    /* Image 1 */

    status = read_file (ctx, jbig2_image1_filename, &mime_data, &mime_length);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 200, 50);

    status = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_JBIG2_GLOBAL_ID,
					  (unsigned char *)"global", 6, NULL, NULL);
    if (status) {
	cairo_surface_destroy (image);
	return cairo_test_status_from_status (ctx, status);
    }

    status = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_JBIG2,
					  mime_data, mime_length,
					  free, mime_data);
    if (status) {
	cairo_surface_destroy (image);
	free (mime_data);
	return cairo_test_status_from_status (ctx, status);
    }

    cairo_set_source_surface (cr, image, x, y);
    cairo_surface_destroy (image);

    cairo_paint (cr);

    /* Image 2 */

    status = read_file (ctx, jbig2_image2_filename, &mime_data, &mime_length);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 200, 50);

    status = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_JBIG2_GLOBAL_ID,
					  (unsigned char *)"global", 6, NULL, NULL);
    if (status) {
	cairo_surface_destroy (image);
	return cairo_test_status_from_status (ctx, status);
    }

    status = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_JBIG2,
					  mime_data, mime_length,
					  free, mime_data);
    if (status) {
	cairo_surface_destroy (image);
	free (mime_data);
	return cairo_test_status_from_status (ctx, status);
    }

    /* Set the global data */
    status = read_file (ctx, jbig2_global_filename, &mime_data, &mime_length);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    status = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_JBIG2_GLOBAL,
					  mime_data, mime_length,
					  free, mime_data);
    if (status) {
	cairo_surface_destroy (image);
	free (mime_data);
	return cairo_test_status_from_status (ctx, status);
    }

    cairo_set_source_surface (cr, image, x, y + 50);
    cairo_surface_destroy (image);

    cairo_paint (cr);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
paint_ccitt_file (cairo_t *cr, int x, int y)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *image;
    unsigned char *mime_data;
    unsigned int mime_length;
    cairo_status_t status;
    const char *ccitt_image_filename = "ccitt.g3";
    const char *ccitt_image_params = "Columns=200 Rows=50 K=-1";

    /* Deliberately use a non-matching MIME images, so that we can identify
     * when the MIME representation is used in preference to the plain image
     * surface.
     */

    status = read_file (ctx, ccitt_image_filename, &mime_data, &mime_length);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 200, 50);

    /* Set the CCITT image data */
    status = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_CCITT_FAX,
					  mime_data, mime_length,
					  free, mime_data);
    if (status) {
	cairo_surface_destroy (image);
	free (mime_data);
	return cairo_test_status_from_status (ctx, status);
    }

    /* Set the CCITT image parameters */
    status = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_CCITT_FAX_PARAMS,
					  (unsigned char *)ccitt_image_params,
					  strlen (ccitt_image_params),
					  NULL, NULL);
    if (status) {
	cairo_surface_destroy (image);
	return cairo_test_status_from_status (ctx, status);
    }

    cairo_set_source_surface (cr, image, x, y);
    cairo_surface_destroy (image);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const char jpg_filename[] = "jpeg.jpg";
    const char png_filename[] = "png.png";
    const char jp2_filename[] = "jp2.jp2";
    cairo_test_status_t status;

    status = paint_file (cr, jpg_filename, CAIRO_MIME_TYPE_JPEG, 0, 0);
    if (status)
	return status;

    status = paint_file (cr, png_filename, CAIRO_MIME_TYPE_PNG, 0, 50);
    if (status)
	return status;

    status = paint_file (cr, jp2_filename, CAIRO_MIME_TYPE_JP2, 0, 100);
    if (status)
	return status;

    status = paint_jbig2_file (cr, 0, 150);
    if (status)
	return status;

    status = paint_ccitt_file (cr, 0, 250);
    if (status)
	return status;

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (mime_data,
	    "Check that the mime-data embedding works",
	    "jpeg, api", /* keywords */
	    NULL, /* requirements */
	    200, 300,
	    NULL, draw)
