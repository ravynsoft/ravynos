/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2005 Emmanuel Pacaud <emmanuel.pacaud@free.fr>
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
 *	   Emmanuel Pacaud <emmanuel.pacaud@free.fr>
 */

#include <stdlib.h>
#include <stdio.h>

/* Disable deprecation warnings coming from librsvg */
#define RSVG_DISABLE_DEPRECATION_WARNINGS
#include <librsvg/rsvg.h>

#define FAIL(msg)							\
    do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

int main (int argc, char *argv[])
{
    GError *error = NULL;
    RsvgHandle *handle;
    RsvgDimensionData dimensions;
    const char *filename = argv[1];
    const char *output_filename = argv[2];
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    if (argc != 3)
	FAIL ("usage: svg2png input_file.svg output_file.png");

    #if GLIB_MAJOR_VERSION <= 2 && GLIB_MINOR_VERSION <= 34
    g_type_init ();
    #endif

    error = NULL;

    handle = rsvg_handle_new_from_file (filename, &error);
    if (!handle)
	FAIL (error->message);

    rsvg_handle_set_dpi (handle, 72.0);
    rsvg_handle_get_dimensions (handle, &dimensions);

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
					  dimensions.width, dimensions.height);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);
    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR_ALPHA);

    if (!rsvg_handle_render_cairo (handle, cr))
	FAIL (error->message);

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    status = cairo_surface_write_to_png (cairo_get_target (cr),
					 output_filename);
    cairo_destroy (cr);
    if (status)
	FAIL (cairo_status_to_string (status));

    g_object_unref (handle);
    return 0;
}
