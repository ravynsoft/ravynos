/*
 * Copyright © 2008 Carlos Garcia Campos
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
 * Author: Carlos Garcia Campos <carlosgc@gnome.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>
#include <libspectre/spectre.h>

#define FAIL(msg)                                                        \
    do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

static const char *
_spectre_render_page (const char *filename,
		      const char *page_label,
		      cairo_surface_t **surface_out)
{
    static const cairo_user_data_key_t key;

    SpectreDocument *document;
    SpectreStatus status;
    int width, height, stride;
    unsigned char *pixels;
    cairo_surface_t *surface;

    document = spectre_document_new ();
    spectre_document_load (document, filename);
    status = spectre_document_status (document);
    if (status) {
	spectre_document_free (document);
	return spectre_status_to_string (status);
    }

    if (page_label) {
	SpectrePage *page;
	SpectreRenderContext *rc;

	page = spectre_document_get_page_by_label (document, page_label);
	spectre_document_free (document);
	if (page == NULL)
	    return "page not found";

	spectre_page_get_size (page, &width, &height);
	rc = spectre_render_context_new ();
	spectre_render_context_set_page_size (rc, width, height);
	spectre_page_render (page, rc, &pixels, &stride);
	spectre_render_context_free (rc);
	status = spectre_page_status (page);
	spectre_page_free (page);
	if (status) {
	    free (pixels);
	    return spectre_status_to_string (status);
	}
    } else {
	spectre_document_get_page_size (document, &width, &height);
	spectre_document_render (document, &pixels, &stride);
	spectre_document_free (document);
    }

    surface = cairo_image_surface_create_for_data (pixels,
						   CAIRO_FORMAT_RGB24,
						   width, height,
						   stride);
    cairo_surface_set_user_data (surface, &key,
				 pixels, (cairo_destroy_func_t) free);
    *surface_out = surface;
    return NULL;
}

int main
(int argc, char *argv[])
{
    const char *err;
    cairo_surface_t *surface = NULL; /* silence compiler warning */
    cairo_status_t status;

    if (argc < 3 || argc > 4)
        FAIL ("usage: ps2png input_file.ps output_file.png [page]");

    err = _spectre_render_page (argv[1], argv[3], &surface);
    if (err != NULL)
        FAIL (err);

    status = cairo_surface_write_to_png (surface, argv[2]);
    cairo_surface_destroy (surface);

    if (status != CAIRO_STATUS_SUCCESS)
        FAIL (cairo_status_to_string (status));

    return 0;
}
