/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2004,2006 Red Hat, Inc.
 * Copyright © 2007, Adrian Johnson
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
 * Authors: Carl D. Worth <cworth@cworth.org>
 *	    Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-boilerplate-private.h"
#include "cairo-malloc-private.h"

#if CAIRO_CAN_TEST_WIN32_PRINTING_SURFACE

#include <cairo-win32.h>
#include <cairo-paginated-surface-private.h>

#include <windows.h>

#if !defined(POSTSCRIPT_IDENTIFY)
# define POSTSCRIPT_IDENTIFY 0x1015
#endif

#if !defined(PSIDENT_GDICENTRIC)
# define PSIDENT_GDICENTRIC 0x0000
#endif

#if !defined(GET_PS_FEATURESETTING)
# define GET_PS_FEATURESETTING 0x1019
#endif

#if !defined(FEATURESETTING_PSLEVEL)
# define FEATURESETTING_PSLEVEL 0x0002
#endif

static cairo_status_t
_cairo_win32_print_gdi_error (const char *context)
{
    void *lpMsgBuf;
    DWORD last_error = GetLastError ();

    if (!FormatMessageW (FORMAT_MESSAGE_ALLOCATE_BUFFER |
			 FORMAT_MESSAGE_FROM_SYSTEM,
			 NULL,
			 last_error,
			 MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
			 (LPWSTR) &lpMsgBuf,
			 0, NULL)) {
	fprintf (stderr, "%s: Unknown GDI error", context);
    } else {
	fprintf (stderr, "%s: %S", context, (wchar_t *)lpMsgBuf);

	LocalFree (lpMsgBuf);
    }

    fflush (stderr);

    /* We should switch off of last_status, but we'd either return
     * CAIRO_STATUS_NO_MEMORY or CAIRO_STATUS_UNKNOWN_ERROR and there
     * is no CAIRO_STATUS_UNKNOWN_ERROR.
     */
    return CAIRO_STATUS_NO_MEMORY;
}

static cairo_user_data_key_t win32_closure_key;

typedef struct _win32_target_closure {
    char *filename;
    int width;
    int height;
    cairo_surface_t *target;
    HDC dc;
    int left_margin;
    int bottom_margin;
} win32_target_closure_t;

static cairo_bool_t
printer_is_postscript_level_3 (HDC dc)
{
    DWORD word;
    INT ps_feature, ps_level;

    word = PSIDENT_GDICENTRIC;
    if (ExtEscape (dc, POSTSCRIPT_IDENTIFY, sizeof(DWORD), (char *)&word, 0, (char *)NULL) <= 0)
	return FALSE;

    ps_feature = FEATURESETTING_PSLEVEL;
    if (ExtEscape (dc, GET_PS_FEATURESETTING, sizeof(INT),
		   (char *)&ps_feature, sizeof(INT), (char *)&ps_level) <= 0)
	return FALSE;

    if (ps_level >= 3)
	return TRUE;

    return FALSE;
}

static void
create_printer_dc (win32_target_closure_t *ptc)
{
    char *printer_name;
    DWORD size;
    int x_dpi, y_dpi, left_margin, top_margin, page_height, printable_height;
    XFORM xform;

    ptc->dc = NULL;
    GetDefaultPrinter (NULL, &size);
    printer_name = _cairo_malloc (size);

    if (printer_name == NULL)
	return;

    if (GetDefaultPrinter (printer_name, &size) == 0) {
	free (printer_name);
	return;
    }

    /* printf("\nPrinting to : %s\n", printer_name); */
    ptc->dc = CreateDC (NULL, printer_name, NULL, NULL);
    free (printer_name);

    if (!printer_is_postscript_level_3 (ptc->dc)) {
	printf("The default printer driver must be a color PostScript level 3 printer\n");
	ptc->dc = NULL;
	return;
    }

    /* The printer device units on win32 are 1 unit == 1 dot and the
     * origin is the start of the printable area. We transform the
     * coordinate space to 1 unit is 1 point as expected by the
     * tests. As the page size is larger than the test surface, the
     * origin is translated down so that the each test is drawn at the
     * bottom left corner of the page. This is because the bottom left
     * corner of the PNG image that ghostscript creates is positioned
     * at origin of the PS coordinates (ie the bottom left of the
     * page).  The left and bottom margins are stored in
     * win32_target_closure as size of the PNG image needs to be
     * increased because the test output is offset from the bottom
     * left by the non printable margins. After the PNG is created the
     * margins will be chopped off so the image matches the reference
     * image.
     */
    printable_height = GetDeviceCaps (ptc->dc, VERTRES);
    x_dpi = GetDeviceCaps (ptc->dc, LOGPIXELSX);
    y_dpi = GetDeviceCaps (ptc->dc, LOGPIXELSY);
    left_margin = GetDeviceCaps (ptc->dc, PHYSICALOFFSETX);
    top_margin = GetDeviceCaps (ptc->dc, PHYSICALOFFSETY);
    page_height = GetDeviceCaps (ptc->dc, PHYSICALHEIGHT);

    SetGraphicsMode (ptc->dc, GM_ADVANCED);
    xform.eM11 = x_dpi/72.0;
    xform.eM12 = 0;
    xform.eM21 = 0;
    xform.eM22 = y_dpi/72.0;
    xform.eDx = 0;
    xform.eDy = printable_height - ptc->height*y_dpi/72.0;
    if (!SetWorldTransform (ptc->dc, &xform)) {
	_cairo_win32_print_gdi_error ("cairo-boilerplate-win32-printing:SetWorldTransform");
	return;
    }

    ptc->left_margin = 72.0*left_margin/x_dpi;
    ptc->bottom_margin = 72.0*(page_height - printable_height - top_margin)/y_dpi;
}

static cairo_surface_t *
_cairo_boilerplate_win32_printing_create_surface (const char		    *name,
						  cairo_content_t	     content,
						  double		     width,
						  double		     height,
						  double		     max_width,
						  double		     max_height,
						  cairo_boilerplate_mode_t   mode,
						  void			   **closure)
{
    win32_target_closure_t *ptc;
    cairo_surface_t *surface;
    DOCINFO di;

    if (content == CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED)
	content = CAIRO_CONTENT_COLOR_ALPHA;

    *closure = ptc = xmalloc (sizeof (win32_target_closure_t));

    xasprintf (&ptc->filename, "%s.out.ps", name);
    xunlink (ptc->filename);

    memset (&di, 0, sizeof (DOCINFO));
    di.cbSize = sizeof (DOCINFO);
    di.lpszDocName = ptc->filename;
    di.lpszOutput = ptc->filename;

    ptc->width = width;
    ptc->height = height;

    create_printer_dc (ptc);
    if (ptc->dc == NULL) {
	printf("\nFailed to create printer\n");
	free (ptc->filename);
	free (ptc);
	return NULL;
    }
    StartDoc (ptc->dc, &di);
    StartPage (ptc->dc);
    surface = cairo_win32_printing_surface_create (ptc->dc);
    if (cairo_surface_status (surface)) {
	free (ptc->filename);
	free (ptc);
	return NULL;
    }
    cairo_surface_set_fallback_resolution (surface, 72., 72.);

    if (content == CAIRO_CONTENT_COLOR) {
	ptc->target = surface;
	surface = cairo_surface_create_similar (ptc->target,
						CAIRO_CONTENT_COLOR,
						width, height);
    } else {
	ptc->target = NULL;
    }

    if (cairo_surface_set_user_data (surface,
				     &win32_closure_key,
				     ptc,
				     NULL) != CAIRO_STATUS_SUCCESS) {
	cairo_surface_destroy (surface);
	if (ptc->target != NULL)
	    cairo_surface_destroy (ptc->target);
	free (ptc->filename);
	free (ptc);
	return NULL;
    }

    return surface;
}

static cairo_status_t
_cairo_boilerplate_win32_printing_surface_write_to_png (cairo_surface_t *surface,
							const char	*filename)
{
    win32_target_closure_t *ptc = cairo_surface_get_user_data (surface, &win32_closure_key);
    char command[4096];
    cairo_surface_t *src_image, *dst_image;
    cairo_t *cr;
    cairo_status_t status;

    if (ptc->target) {
	/* Both surface and ptc->target were originally created at the
	 * same dimensions. We want a 1:1 copy here, so we first clear any
	 * device offset and scale on surface.
	 *
	 * In a more realistic use case of device offsets, the target of
	 * this copying would be of a different size than the source, and
	 * the offset would be desirable during the copy operation. */
	double x_offset, y_offset;
	double x_scale, y_scale;
	cairo_surface_get_device_offset (surface, &x_offset, &y_offset);
	cairo_surface_get_device_scale (surface, &x_scale, &y_scale);
	cairo_surface_set_device_offset (surface, 0, 0);
	cairo_surface_set_device_scale (surface, 1, 1);
	cairo_t *cr;
	cr = cairo_create (ptc->target);
	cairo_set_source_surface (cr, surface, 0, 0);
	cairo_paint (cr);
	cairo_show_page (cr);
	cairo_destroy (cr);
	cairo_surface_set_device_offset (surface, x_offset, y_offset);
	cairo_surface_set_device_scale (surface, x_scale, y_scale);

	cairo_surface_finish (surface);
	surface = ptc->target;
    }

    cairo_surface_finish (surface);
    EndPage (ptc->dc);
    EndDoc (ptc->dc);
    sprintf (command, "gs -q -r72 -g%dx%d -dSAFER -dBATCH -dNOPAUSE -sDEVICE=pngalpha -sOutputFile=%s %s",
	     ptc->width + ptc->left_margin, ptc->height + ptc->bottom_margin, filename, ptc->filename);

    if (system (command) != 0)
	return CAIRO_STATUS_WRITE_ERROR;

    /* Create a new image from the ghostscript image that has the
     * left and bottom margins removed */

    src_image = cairo_image_surface_create_from_png (filename);
    status = cairo_surface_status (src_image);
    if (status)
	return status;

    dst_image = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
					    ptc->width,
					    ptc->height);
    cr = cairo_create (dst_image);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface (cr, src_image, -ptc->left_margin, 0);
    cairo_paint (cr);
    cairo_destroy (cr);

    cairo_surface_write_to_png (dst_image, filename);
    status = cairo_surface_status (dst_image);
    if (status)
	return status;

    cairo_surface_destroy (src_image);
    cairo_surface_destroy (dst_image);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t *
_cairo_boilerplate_win32_printing_get_image_surface (cairo_surface_t *surface,
						     int	      page,
						     int	      width,
						     int	      height)
{
    win32_target_closure_t *ptc = cairo_surface_get_user_data (surface,
							       &win32_closure_key);
    char *filename;
    cairo_status_t status;

    /* XXX test paginated interface */
    if (page != 0)
	return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);

    xasprintf (&filename, "%s.png", ptc->filename);
    status = _cairo_boilerplate_win32_printing_surface_write_to_png (surface, filename);
    if (status)
	return cairo_boilerplate_surface_create_in_error (status);

    surface = cairo_boilerplate_get_image_surface_from_png (filename,
							    width,
							    height,
							    ptc->target == NULL);

    remove (filename);
    free (filename);

    return surface;
}

static void
_cairo_boilerplate_win32_printing_cleanup (void *closure)
{
    win32_target_closure_t *ptc = closure;

    if (ptc->target)
	cairo_surface_destroy (ptc->target);
    free (ptc->filename);
    free (ptc);
    DeleteDC (ptc->dc);
}

static const cairo_boilerplate_target_t targets[] = {
    {
	"win32-printing", "win32", ".ps", NULL,
	CAIRO_SURFACE_TYPE_WIN32_PRINTING,
	CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED, 0,
	"cairo_win32_printing_surface_create",
	_cairo_boilerplate_win32_printing_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_win32_printing_get_image_surface,
	_cairo_boilerplate_win32_printing_surface_write_to_png,
	_cairo_boilerplate_win32_printing_cleanup,
	NULL, NULL, FALSE, TRUE, TRUE
    },
    {
	"win32-printing", "win32", ".ps", NULL,
	CAIRO_SURFACE_TYPE_RECORDING, CAIRO_CONTENT_COLOR, 0,
	"cairo_win32_printing_surface_create",
	_cairo_boilerplate_win32_printing_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_win32_printing_get_image_surface,
	_cairo_boilerplate_win32_printing_surface_write_to_png,
	_cairo_boilerplate_win32_printing_cleanup,
	NULL, NULL, FALSE, TRUE, TRUE
    },
};
CAIRO_BOILERPLATE (win32_printing, targets)

#else

CAIRO_NO_BOILERPLATE (win32_printing)

#endif
