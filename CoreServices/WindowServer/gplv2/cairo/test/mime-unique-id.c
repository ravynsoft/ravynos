/*
 * Copyright © 2017 Adrian Johnson
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


/* Check that source surfaces with same CAIRO_MIME_TYPE_UNIQUE_ID are
 * embedded only once in PDF/PS.
 *
 * To exercise all the surface embedding code in PS/PDF, four types of
 * source surfaces are painted on each page, each with its own UNIQUE_ID:
 * - an image surface
 * - a recording surface with a jpeg mime attached
 * - a bounded recording surface
 * - an unbounded recording surface.
 *
 * Four pages are generated. Each source is clipped starting with the
 * smallest area on the first page increasing to the unclipped size on
 * the last page. This is to ensure the output does not embed the
 * source clipped to a smaller size than used on subsequent pages.
 *
 * The test verifies the use of UNIQUE_ID by comparing the file size
 * with the expected size.
 */

#include "cairo-test.h"

#include <math.h>
#include <stdio.h>
#include <errno.h>

#include <cairo.h>

#if CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif

#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif

#define NUM_PAGES 4

#define WIDTH  275
#define HEIGHT 275

#define BASENAME "mime-unique-id"


/* Expected file size to check that surfaces are embedded only once.
 * SIZE_TOLERANCE should be large enough to allow some variation in
 * file size due to changes to the PS/PDF surfaces while being small
 * enough to catch any attempt to embed the surface more than
 * once. The compressed size of each surface embedded in PDF is:
 * - image:    108,774
 * - jpeg:      11,400
 * - recording: 17,518
 *
 * If the size check fails, manually check the output and if the
 * surfaces are still embedded only once, update the expected sizes.
 */
#define PS2_EXPECTED_SIZE 417510
#define PS3_EXPECTED_SIZE 381554
#define PDF_EXPECTED_SIZE 162923
#define SIZE_TOLERANCE      5000

static const char *png_filename = "romedalen.png";
static const char *jpeg_filename = "romedalen.jpg";

static FILE *
my_fopen (cairo_test_context_t *ctx, const char *pathname, const char *mode)
{
    FILE *f = fopen (pathname, mode);
    if (f == NULL && errno == ENOENT && ctx->srcdir) {
	char *srcdir_pathname;
	xasprintf (&srcdir_pathname, "%s/%s", ctx->srcdir, pathname);
	f = fopen (srcdir_pathname, mode);
	free (srcdir_pathname);
    }
    return f;
}

static cairo_test_status_t
create_image_surface (cairo_test_context_t *ctx, cairo_surface_t **surface)
{
    cairo_status_t status;
    const char *unique_id = "image";

    *surface = cairo_test_create_surface_from_png (ctx, png_filename);
    status = cairo_surface_set_mime_data (*surface, CAIRO_MIME_TYPE_UNIQUE_ID,
					  (unsigned char *)unique_id,
					  strlen (unique_id),
					  NULL, NULL);
    if (status) {
	cairo_surface_destroy (*surface);
	return cairo_test_status_from_status (ctx, status);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
create_recording_surface_with_mime_jpg (cairo_test_context_t *ctx, cairo_surface_t **surface)
{
    cairo_status_t status;
    FILE *f;
    unsigned char *data;
    long len;
    const char *unique_id = "jpeg";
    cairo_rectangle_t extents = { 0, 0, 1, 1 };

    *surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &extents);
    f = my_fopen (ctx, jpeg_filename, "rb");
    if (f == NULL) {
	cairo_test_log (ctx, "Unable to open file %s\n", jpeg_filename);
	return CAIRO_TEST_FAILURE;
    }

    fseek (f, 0, SEEK_END);
    len = ftell(f);
    fseek (f, 0, SEEK_SET);
    data = malloc (len);
    if (fread(data, len, 1, f) != 1) {
	cairo_test_log (ctx, "Unable to read file %s\n", jpeg_filename);
	return CAIRO_TEST_FAILURE;
    }

    fclose(f);
    status = cairo_surface_set_mime_data (*surface,
					  CAIRO_MIME_TYPE_JPEG,
					  data, len,
					  free, data);
    if (status) {
	cairo_surface_destroy (*surface);
	return cairo_test_status_from_status (ctx, status);
    }

    status = cairo_surface_set_mime_data (*surface, CAIRO_MIME_TYPE_UNIQUE_ID,
					  (unsigned char *)unique_id,
					  strlen (unique_id),
					  NULL, NULL);
    if (status) {
	cairo_surface_destroy (*surface);
	return cairo_test_status_from_status (ctx, status);
    }

    return CAIRO_TEST_SUCCESS;
}

static void
draw_tile (cairo_t *cr)
{
    cairo_move_to (cr, 10 + 5, 10);
    cairo_arc (cr, 10, 10, 5, 0, 2*M_PI);
    cairo_close_path (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill (cr);

    cairo_move_to (cr, 30, 10-10*0.43);
    cairo_line_to (cr, 25, 10+10*0.43);
    cairo_line_to (cr, 35, 10+10*0.43);
    cairo_close_path (cr);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_fill (cr);

    cairo_rectangle (cr, 5, 25, 10, 10);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_fill (cr);

    cairo_save (cr);
    cairo_translate (cr, 30, 30);
    cairo_rotate (cr, M_PI/4.0);
    cairo_rectangle (cr, -5, -5, 10, 10);
    cairo_set_source_rgb (cr, 1, 0, 1);
    cairo_fill (cr);
    cairo_restore (cr);
}

#define RECORDING_SIZE 800
#define TILE_SIZE 40

static cairo_test_status_t
create_recording_surface (cairo_test_context_t *ctx, cairo_surface_t **surface, cairo_bool_t bounded)
{
    cairo_status_t status;
    int x, y;
    cairo_t *cr;
    cairo_matrix_t ctm;
    int start, size;
    const char *bounded_id = "recording bounded";
    const char *unbounded_id = "recording unbounded";
    cairo_rectangle_t extents = { 0, 0, RECORDING_SIZE, RECORDING_SIZE };

    if (bounded) {
	*surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &extents);
	start = 0;
	size = RECORDING_SIZE;
    } else {
	*surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
	start = RECORDING_SIZE / 2;
	size = RECORDING_SIZE * 2;
    }

    /* Draw each tile instead of creating a cairo pattern to make size
     * of the emitted recording as large as possible.
     */
    cr = cairo_create (*surface);
    cairo_set_source_rgb (cr, 1, 1, 0);
    cairo_paint (cr);
    cairo_get_matrix (cr, &ctm);
    for (y = start; y < size; y += TILE_SIZE) {
	for (x = start; x < size; x += TILE_SIZE) {
	    draw_tile (cr);
	    cairo_translate (cr, TILE_SIZE, 0);
	}
	cairo_matrix_translate (&ctm, 0, TILE_SIZE);
	cairo_set_matrix (cr, &ctm);
    }
    cairo_destroy (cr);

    status = cairo_surface_set_mime_data (*surface, CAIRO_MIME_TYPE_UNIQUE_ID,
					  (unsigned char *)(bounded ? bounded_id : unbounded_id),
					  strlen (bounded ? bounded_id : unbounded_id),
					  NULL, NULL);
    if (status) {
	cairo_surface_destroy (*surface);
	return cairo_test_status_from_status (ctx, status);
    }

    return CAIRO_TEST_SUCCESS;
}

/* Draw @source scaled to fit @rect and clipped to a rectangle
 * @clip_margin units smaller on each side.  @rect will be stroked
 * with a solid line and the clip rect stroked with a dashed line.
 */
static void
draw_surface (cairo_t *cr, cairo_surface_t *source, cairo_rectangle_int_t *rect, int clip_margin)
{
    cairo_surface_type_t type;
    int width, height;
    cairo_rectangle_t extents;
    const double dashes[2] = { 2, 2 };

    type = cairo_surface_get_type (source);
    if (type == CAIRO_SURFACE_TYPE_IMAGE) {
	width = cairo_image_surface_get_width (source);
	height = cairo_image_surface_get_height (source);
    } else {
	if (cairo_recording_surface_get_extents (source, &extents)) {
	    width = extents.width;
	    height = extents.height;
	} else {
	    width = RECORDING_SIZE;
	    height = RECORDING_SIZE;
	}
    }

    cairo_save (cr);
    cairo_rectangle (cr, rect->x, rect->y, rect->width, rect->height);
    cairo_stroke (cr);
    cairo_rectangle (cr,
		     rect->x + clip_margin,
		     rect->y + clip_margin,
		     rect->width - clip_margin*2,
		     rect->height - clip_margin*2);
    cairo_set_dash (cr, dashes, 2, 0);
    cairo_stroke_preserve (cr);
    cairo_clip (cr);

    cairo_translate (cr, rect->x, rect->y);
    cairo_scale (cr, (double)rect->width/width, (double)rect->height/height);
    cairo_set_source_surface (cr, source, 0, 0);
    cairo_paint (cr);

    cairo_restore (cr);
}

static cairo_test_status_t
draw_pages (cairo_test_context_t *ctx, cairo_surface_t *surface)
{
    cairo_t *cr;
    int i;
    cairo_rectangle_int_t img_rect;
    cairo_rectangle_int_t jpg_rect;
    cairo_rectangle_int_t bounded_rect;
    cairo_rectangle_int_t unbounded_rect;
    int clip_margin;
    cairo_surface_t *source;
    cairo_test_status_t status;

    cr = cairo_create (surface);

    /* target area to fill with the image source */
    img_rect.x = 25;
    img_rect.y = 25;
    img_rect.width = 100;
    img_rect.height = 100;

    /* target area to fill with the recording with jpeg mime source */
    jpg_rect.x = 150;
    jpg_rect.y = 25;
    jpg_rect.width = 100;
    jpg_rect.height = 100;

    /* target area to fill with the bounded recording source */
    bounded_rect.x = 25;
    bounded_rect.y = 150;
    bounded_rect.width = 100;
    bounded_rect.height = 100;

    /* target area to fill with the unbounded recording source */
    unbounded_rect.x = 150;
    unbounded_rect.y = 150;
    unbounded_rect.width = 100;
    unbounded_rect.height = 100;

    /* Draw the image and recording surface on each page. The sources
     * are clipped starting with a small clip area on the first page
     * and increasing to the source size on last page to ensure the
     * embedded source is not clipped to the area used on the first
     * page.
     *
     * The sources are created each time they are used to ensure
     * CAIRO_MIME_TYPE_UNIQUE_ID is tested.
     */
    for (i = 0; i < NUM_PAGES; i++) {
	clip_margin = (NUM_PAGES - i - 1) * 5;

	status = create_image_surface (ctx, &source);
	if (status)
	    return status;
	draw_surface (cr, source, &img_rect, clip_margin);
	cairo_surface_destroy (source);

	status = create_recording_surface_with_mime_jpg (ctx, &source);
	if (status)
	    return status;
	draw_surface (cr, source, &jpg_rect, clip_margin);
	cairo_surface_destroy (source);

	status = create_recording_surface (ctx, &source, TRUE);
	if (status)
	    return status;
	draw_surface (cr, source, &bounded_rect, clip_margin);
	cairo_surface_destroy (source);

	status = create_recording_surface (ctx, &source, FALSE);
	if (status)
	    return status;
	draw_surface (cr, source, &unbounded_rect, clip_margin);
	cairo_surface_destroy (source);

	cairo_show_page (cr);
    }

    cairo_destroy (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
check_file_size (cairo_test_context_t *ctx, const char *filename, long expected_size)
{
    FILE *f;
    long size;

    f = my_fopen (ctx, filename, "rb");
    if (f == NULL) {
	cairo_test_log (ctx, "Unable to open file %s\n", filename);
	return CAIRO_TEST_FAILURE;
    }

    fseek (f, 0, SEEK_END);
    size = ftell (f);
    fclose(f);

    if (labs(size - expected_size) > SIZE_TOLERANCE) {
	cairo_test_log (ctx,
			"mime-unique-id: File %s has size %ld. Expected size %ld +/- %ld."
			" Check if surfaces are embedded once.\n",
			filename, size, expected_size, (long)SIZE_TOLERANCE);
	return CAIRO_TEST_FAILURE;
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t *surface;
    cairo_status_t status;
    char *filename;
    cairo_test_status_t result = CAIRO_TEST_UNTESTED;
    cairo_test_status_t test_status;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

#if CAIRO_HAS_PS_SURFACE
    if (cairo_test_is_target_enabled (ctx, "ps2"))
    {
	xasprintf (&filename, "%s/%s.ps2.out.ps", path, BASENAME);
	surface = cairo_ps_surface_create (filename, WIDTH, HEIGHT);
	status = cairo_surface_status (surface);
	if (status) {
	    cairo_test_log (ctx, "Failed to create ps surface for file %s: %s\n",
			    filename, cairo_status_to_string (status));
	    test_status = CAIRO_TEST_FAILURE;
	    goto ps2_finish;
	}

	cairo_ps_surface_restrict_to_level (surface, CAIRO_PS_LEVEL_2);

	test_status = draw_pages (ctx, surface);
	cairo_surface_destroy (surface);

	if (test_status == CAIRO_TEST_SUCCESS)
	    test_status = check_file_size (ctx, filename, PS2_EXPECTED_SIZE);

      ps2_finish:
	cairo_test_log (ctx, "TEST: %s TARGET: %s RESULT: %s\n",
			ctx->test->name,
			"ps2",
			test_status ? "FAIL" : "PASS");

	if (result == CAIRO_TEST_UNTESTED || test_status == CAIRO_TEST_FAILURE)
	    result = test_status;

	free (filename);
    }

    if (cairo_test_is_target_enabled (ctx, "ps3"))
    {
	xasprintf (&filename, "%s/%s.ps3.out.ps", path, BASENAME);
	surface = cairo_ps_surface_create (filename, WIDTH, HEIGHT);
	status = cairo_surface_status (surface);
	if (status) {
	    cairo_test_log (ctx, "Failed to create ps surface for file %s: %s\n",
			    filename, cairo_status_to_string (status));
	    test_status = CAIRO_TEST_FAILURE;
	    goto ps3_finish;
	}

	test_status = draw_pages (ctx, surface);
	cairo_surface_destroy (surface);

	if (test_status == CAIRO_TEST_SUCCESS)
	    test_status = check_file_size (ctx, filename, PS3_EXPECTED_SIZE);

      ps3_finish:
	cairo_test_log (ctx, "TEST: %s TARGET: %s RESULT: %s\n",
			ctx->test->name,
			"ps3",
			test_status ? "FAIL" : "PASS");

	if (result == CAIRO_TEST_UNTESTED || test_status == CAIRO_TEST_FAILURE)
	    result = test_status;

	free (filename);
    }
#endif

#if CAIRO_HAS_PDF_SURFACE
    if (cairo_test_is_target_enabled (ctx, "pdf"))
    {
	xasprintf (&filename, "%s/%s.pdf.out.pdf", path, BASENAME);
	surface = cairo_pdf_surface_create (filename, WIDTH, HEIGHT);
	status = cairo_surface_status (surface);
	if (status) {
	    cairo_test_log (ctx, "Failed to create pdf surface for file %s: %s\n",
			    filename, cairo_status_to_string (status));
	    test_status = CAIRO_TEST_FAILURE;
	    goto pdf_finish;
	}

	test_status = draw_pages (ctx, surface);
	cairo_surface_destroy (surface);

	if (test_status == CAIRO_TEST_SUCCESS)
	    test_status = check_file_size (ctx, filename, PDF_EXPECTED_SIZE);


      pdf_finish:
	cairo_test_log (ctx, "TEST: %s TARGET: %s RESULT: %s\n",
			ctx->test->name,
			"pdf",
			test_status ? "FAIL" : "PASS");

	if (result == CAIRO_TEST_UNTESTED || test_status == CAIRO_TEST_FAILURE)
	    result = test_status;

	free (filename);
    }
#endif

    return result;
}

CAIRO_TEST (mime_unique_id,
	    "Check that paginated surfaces embed only one copy of surfaces with the same CAIRO_MIME_TYPE_UNIQUE_ID.",
	    "paginated", /* keywords */
	    "target=vector", /* requirements */
	    0, 0,
	    preamble, NULL)
