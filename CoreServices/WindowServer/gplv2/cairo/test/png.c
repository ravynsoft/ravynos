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

#include <assert.h>

/* Test the idempotency of write_png->read_png */

#define RGB_MASK 0x00ffffff
#define BASENAME "png.out"

static cairo_bool_t
image_surface_equals (cairo_surface_t *A, cairo_surface_t *B)
{
    if (cairo_image_surface_get_format (A) !=
	cairo_image_surface_get_format (B))
	return 0;

    if (cairo_image_surface_get_width (A) !=
	cairo_image_surface_get_width (B))
	return 0;

    if (cairo_image_surface_get_height (A) !=
	cairo_image_surface_get_height (B))
	return 0;

    return 1;
}

static const char *
format_to_string (cairo_format_t format)
{
    switch (format) {
    case CAIRO_FORMAT_A1:     return "a1";
    case CAIRO_FORMAT_A8:     return "a8";
    case CAIRO_FORMAT_RGB16_565:  return "rgb16";
    case CAIRO_FORMAT_RGB24:  return "rgb24";
    case CAIRO_FORMAT_RGB30:  return "rgb30";
    case CAIRO_FORMAT_ARGB32: return "argb32";
    case CAIRO_FORMAT_RGB96F: return "rgb96f";
    case CAIRO_FORMAT_RGBA128F: return "rgba128f";
    case CAIRO_FORMAT_INVALID:
    default: return "???";
    }
}

static void
print_surface (const cairo_test_context_t *ctx, cairo_surface_t *surface)
{
    cairo_test_log (ctx,
		    "%s (%dx%d)\n",
		    format_to_string (cairo_image_surface_get_format (surface)),
		    cairo_image_surface_get_width (surface),
		    cairo_image_surface_get_height (surface));
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t *surface0, *surface1;
    cairo_status_t status;
    uint32_t argb32 = 0xdeadbede;
    char *filename;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

    xasprintf (&filename, "%s/%s.png", path, BASENAME);
    surface0 = cairo_image_surface_create_for_data ((unsigned char *) &argb32,
						    CAIRO_FORMAT_ARGB32,
						    1, 1, 4);
    status = cairo_surface_write_to_png (surface0, filename);
    if (status) {
	cairo_test_log (ctx, "Error writing '%s': %s\n",
			filename, cairo_status_to_string (status));

	cairo_surface_destroy (surface0);
	free (filename);
	return cairo_test_status_from_status (ctx, status);
    }
    surface1 = cairo_image_surface_create_from_png (filename);
    status = cairo_surface_status (surface1);
    if (status) {
	cairo_test_log (ctx, "Error reading '%s': %s\n",
			filename, cairo_status_to_string (status));

	cairo_surface_destroy (surface1);
	cairo_surface_destroy (surface0);
	free (filename);
	return cairo_test_status_from_status (ctx, status);
    }

    if (! image_surface_equals (surface0, surface1)) {
	cairo_test_log (ctx, "Error surface mismatch.\n");
	cairo_test_log (ctx, "to png: "); print_surface (ctx, surface0);
	cairo_test_log (ctx, "from png: "); print_surface (ctx, surface1);

	cairo_surface_destroy (surface0);
	cairo_surface_destroy (surface1);
	free (filename);
	return CAIRO_TEST_FAILURE;
    }
    assert (*(uint32_t *) cairo_image_surface_get_data (surface1) == argb32);

    cairo_surface_destroy (surface0);
    cairo_surface_destroy (surface1);

    surface0 = cairo_image_surface_create_for_data ((unsigned char *) &argb32,
						    CAIRO_FORMAT_RGB24,
						    1, 1, 4);
    status = cairo_surface_write_to_png (surface0, filename);
    if (status) {
	cairo_test_log (ctx, "Error writing '%s': %s\n",
			filename, cairo_status_to_string (status));
	cairo_surface_destroy (surface0);
	return cairo_test_status_from_status (ctx, status);
    }
    surface1 = cairo_image_surface_create_from_png (filename);
    status = cairo_surface_status (surface1);
    if (status) {
	cairo_test_log (ctx, "Error reading '%s': %s\n",
			filename, cairo_status_to_string (status));
	free (filename);

	cairo_surface_destroy (surface1);
	cairo_surface_destroy (surface0);
	return cairo_test_status_from_status (ctx, status);
    }
    free (filename);

    if (! image_surface_equals (surface0, surface1)) {
	cairo_test_log (ctx, "Error surface mismatch.\n");
	cairo_test_log (ctx, "to png: "); print_surface (ctx, surface0);
	cairo_test_log (ctx, "from png: "); print_surface (ctx, surface1);

	cairo_surface_destroy (surface0);
	cairo_surface_destroy (surface1);
	return CAIRO_TEST_FAILURE;
    }
    assert ((*(uint32_t *) cairo_image_surface_get_data (surface1) & RGB_MASK)
	    == (argb32 & RGB_MASK));

    cairo_surface_destroy (surface0);
    cairo_surface_destroy (surface1);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (png,
	    "Check that the png export/import is idempotent.",
	    "png, api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
