/*
 * Copyright © 2010 Red Hat Inc.
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
 * Author: Benjamin Otte <otte@redhat.com>
 */

#include "config.h"

#include <limits.h>

#include "cairo-test.h"

#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#if CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif
#if CAIRO_HAS_XCB_SURFACE
#include <cairo-xcb.h>
#endif
#if CAIRO_HAS_XLIB_SURFACE
#include <cairo-xlib.h>
#endif

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t *surface;

    /* get the error surface */
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, INT_MAX, INT_MAX);

#if CAIRO_HAS_PDF_SURFACE
    cairo_pdf_surface_restrict_to_version (surface, CAIRO_PDF_VERSION_1_4);
    cairo_pdf_surface_set_size (surface, 0, 0);
#endif

#if CAIRO_HAS_PS_SURFACE
    cairo_ps_surface_set_eps (surface, FALSE);
    cairo_ps_surface_set_size (surface, 0, 0);
    cairo_ps_surface_restrict_to_level (surface, CAIRO_PS_LEVEL_2);
    cairo_ps_surface_dsc_comment (surface, NULL);
    cairo_ps_surface_dsc_begin_setup (surface);
    cairo_ps_surface_dsc_begin_page_setup (surface);
#endif

#if CAIRO_HAS_XCB_SURFACE
    cairo_xcb_surface_set_size (surface, 0, 0);
#endif

#if CAIRO_HAS_XLIB_SURFACE
    cairo_xlib_surface_set_size (surface, 0, 0);
    cairo_xlib_surface_set_drawable (surface, 0, 0, 0);
#endif

    cairo_surface_set_mime_data (surface, NULL, NULL, 0, NULL, 0);
    cairo_surface_set_device_offset (surface, 0, 0);
    cairo_surface_set_fallback_resolution (surface, 0, 0);

    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (error_setters,
	    "Check setters properly error out on read-only error surfaces",
	    NULL, /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
