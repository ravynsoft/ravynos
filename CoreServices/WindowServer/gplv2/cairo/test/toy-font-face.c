/*
 * Copyright © 2005,2008 Red Hat, Inc.
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
 *         Behdad Esfahbod <behdad@behdad.org>
 */

#include "config.h"

#include "cairo-test.h"

#include <cairo.h>
#include <assert.h>
#include <string.h>

#if   CAIRO_HAS_WIN32_FONT
#define CAIRO_FONT_FAMILY_DEFAULT "Arial"
#elif CAIRO_HAS_QUARTZ_FONT
#define CAIRO_FONT_FAMILY_DEFAULT "Helvetica"
#elif CAIRO_HAS_FT_FONT
#define CAIRO_FONT_FAMILY_DEFAULT ""
#else
#define CAIRO_FONT_FAMILY_DEFAULT "@cairo:"
#endif


static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_t *cr;
    cairo_surface_t *surface;
    cairo_font_face_t *font_face;
    cairo_status_t status;

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 0, 0);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    font_face = cairo_font_face_reference (cairo_get_font_face (cr));
    assert (cairo_font_face_get_type (font_face) == CAIRO_FONT_TYPE_TOY);
    assert (cairo_toy_font_face_get_family (font_face) != NULL);
    assert (cairo_toy_font_face_get_slant (font_face) == CAIRO_FONT_SLANT_NORMAL);
    assert (cairo_toy_font_face_get_weight (font_face) == CAIRO_FONT_WEIGHT_NORMAL);
    status = cairo_font_face_status(font_face);
    cairo_font_face_destroy (font_face);

    if (status)
	return cairo_test_status_from_status (ctx, status);

    cairo_select_font_face (cr,
			    "bizarre",
			    CAIRO_FONT_SLANT_OBLIQUE,
			    CAIRO_FONT_WEIGHT_BOLD);
    font_face = cairo_font_face_reference (cairo_get_font_face (cr));
    assert (cairo_font_face_get_type (font_face) == CAIRO_FONT_TYPE_TOY);
    assert (0 == (strcmp) (cairo_toy_font_face_get_family (font_face), "bizarre"));
    assert (cairo_toy_font_face_get_slant (font_face) == CAIRO_FONT_SLANT_OBLIQUE);
    assert (cairo_toy_font_face_get_weight (font_face) == CAIRO_FONT_WEIGHT_BOLD);
    status = cairo_font_face_status(font_face);
    cairo_font_face_destroy (font_face);

    if (status)
	return cairo_test_status_from_status (ctx, status);

    font_face = cairo_toy_font_face_create ("bozarre",
					    CAIRO_FONT_SLANT_OBLIQUE,
					    CAIRO_FONT_WEIGHT_BOLD);
    assert (cairo_font_face_get_type (font_face) == CAIRO_FONT_TYPE_TOY);
    assert (0 == (strcmp) (cairo_toy_font_face_get_family (font_face), "bozarre"));
    assert (cairo_toy_font_face_get_slant (font_face) == CAIRO_FONT_SLANT_OBLIQUE);
    assert (cairo_toy_font_face_get_weight (font_face) == CAIRO_FONT_WEIGHT_BOLD);
    status = cairo_font_face_status(font_face);
    cairo_font_face_destroy (font_face);

    if (status)
	return cairo_test_status_from_status (ctx, status);

    font_face = cairo_toy_font_face_create (NULL,
					    CAIRO_FONT_SLANT_OBLIQUE,
					    CAIRO_FONT_WEIGHT_BOLD);
    assert (cairo_font_face_get_type (font_face) == CAIRO_FONT_TYPE_TOY);
    assert (0 == (strcmp) (cairo_toy_font_face_get_family (font_face), CAIRO_FONT_FAMILY_DEFAULT));
    assert (cairo_toy_font_face_get_slant (font_face) == CAIRO_FONT_SLANT_NORMAL);
    assert (cairo_toy_font_face_get_weight (font_face) == CAIRO_FONT_WEIGHT_NORMAL);
    assert (cairo_font_face_status(font_face) == CAIRO_STATUS_NULL_POINTER);
    cairo_font_face_destroy (font_face);

    font_face = cairo_toy_font_face_create ("\xff",
					    CAIRO_FONT_SLANT_OBLIQUE,
					    CAIRO_FONT_WEIGHT_BOLD);
    assert (cairo_font_face_get_type (font_face) == CAIRO_FONT_TYPE_TOY);
    assert (0 == (strcmp) (cairo_toy_font_face_get_family (font_face), CAIRO_FONT_FAMILY_DEFAULT));
    assert (cairo_toy_font_face_get_slant (font_face) == CAIRO_FONT_SLANT_NORMAL);
    assert (cairo_toy_font_face_get_weight (font_face) == CAIRO_FONT_WEIGHT_NORMAL);
    assert (cairo_font_face_status(font_face) == CAIRO_STATUS_INVALID_STRING);
    cairo_font_face_destroy (font_face);

    font_face = cairo_toy_font_face_create ("sans",
					    -1,
					    CAIRO_FONT_WEIGHT_BOLD);
    assert (cairo_font_face_get_type (font_face) == CAIRO_FONT_TYPE_TOY);
    assert (0 == (strcmp) (cairo_toy_font_face_get_family (font_face), CAIRO_FONT_FAMILY_DEFAULT));
    assert (cairo_toy_font_face_get_slant (font_face) == CAIRO_FONT_SLANT_NORMAL);
    assert (cairo_toy_font_face_get_weight (font_face) == CAIRO_FONT_WEIGHT_NORMAL);
    assert (cairo_font_face_status(font_face) == CAIRO_STATUS_INVALID_SLANT);
    cairo_font_face_destroy (font_face);

    font_face = cairo_toy_font_face_create ("sans",
					    CAIRO_FONT_SLANT_OBLIQUE,
					    -1);
    assert (cairo_font_face_get_type (font_face) == CAIRO_FONT_TYPE_TOY);
    assert (0 == (strcmp) (cairo_toy_font_face_get_family (font_face), CAIRO_FONT_FAMILY_DEFAULT));
    assert (cairo_toy_font_face_get_slant (font_face) == CAIRO_FONT_SLANT_NORMAL);
    assert (cairo_toy_font_face_get_weight (font_face) == CAIRO_FONT_WEIGHT_NORMAL);
    assert (cairo_font_face_status(font_face) == CAIRO_STATUS_INVALID_WEIGHT);
    cairo_font_face_destroy (font_face);

    cairo_destroy (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (toy_font_face,
	    "Check the construction of 'toy' font faces",
	    "font, api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
