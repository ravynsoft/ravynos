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
 * Author: Kristian Høgsberg <krh@redhat.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <poppler.h>

#define FAIL(msg)							\
    do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

#define PIXELS_PER_POINT 1

int main (int argc, char *argv[])
{
    PopplerDocument *document;
    PopplerPage *page;
    double width, height;
    const char *filename = argv[1];
    const char *output_filename = argv[2];
    const char *page_label = argv[3];
    gchar *absolute, *uri;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;
    GError *error = NULL;

    if (argc != 4)
	FAIL ("usage: pdf2png input_file.pdf output_file.png page");

#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init ();
#endif

    if (g_path_is_absolute(filename)) {
	absolute = g_strdup (filename);
    } else {
	gchar *dir = g_get_current_dir ();
	absolute = g_build_filename (dir, filename, (gchar *) 0);
	g_free (dir);
    }

    uri = g_filename_to_uri (absolute, NULL, &error);
    g_free (absolute);
    if (uri == NULL)
	FAIL (error->message);

    document = poppler_document_new_from_file (uri, NULL, &error);
    g_free (uri);
    if (document == NULL)
	FAIL (error->message);

    page = poppler_document_get_page_by_label (document, page_label);
    g_object_unref (document);
    if (page == NULL)
	FAIL ("page not found");

    poppler_page_get_size (page, &width, &height);

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);
    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR_ALPHA);

    poppler_page_render (page, cr);
    g_object_unref (page);

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    status = cairo_surface_write_to_png (cairo_get_target (cr),
					 output_filename);
    cairo_destroy (cr);

    if (status)
	FAIL (cairo_status_to_string (status));

    return 0;
}
