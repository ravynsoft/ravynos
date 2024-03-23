/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2004,2007 Red Hat, Inc.
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
 */

#include "cairo-boilerplate-private.h"

#include <cairo-quartz-private.h>

#include <dlfcn.h>

#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT ((void *) 0)
#endif

/*
 * macOS Private functions
 */
typedef enum {
    kCGContextTypeUnknown,
    kCGContextTypePDF,
    kCGContextTypePostScript,
    kCGContextTypeWindow,
    kCGContextTypeBitmap,
    kCGContextTypeGL,
    kCGContextTypeDisplayList,
    kCGContextTypeKSeparation,
    kCGContextTypeIOSurface,
    kCGContextTypeCount
} CGContextType;


static unsigned int (*CGContextGetTypePtr) (CGContextRef) = NULL;
static void
quartz_ensure_symbols (void)
{
    static cairo_bool_t symbol_lookup_done = FALSE;
    if (!symbol_lookup_done) {
	CGContextGetTypePtr = dlsym (RTLD_DEFAULT, "CGContextGetType");
	symbol_lookup_done = TRUE;
    }
}

static cairo_surface_t *
_cairo_boilerplate_quartz_create_surface (const char		    *name,
					  cairo_content_t	     content,
					  double		     width,
					  double		     height,
					  double		     max_width,
					  double		     max_height,
					  cairo_boilerplate_mode_t   mode,
					  void			   **closure)
{
    cairo_format_t format;

    format = cairo_boilerplate_format_from_content (content);

    *closure = NULL;

    return cairo_quartz_surface_create (format, width, height);
}

static bool
cg_context_is_bitmap (CGContextRef context)
{
    quartz_ensure_symbols ();

    if (likely (CGContextGetTypePtr)) {
	return CGContextGetTypePtr (context) == kCGContextTypeBitmap;
    }

    return CGBitmapContextGetBitsPerPixel (context) != 0;
}

static cairo_status_t
_cairo_boilerplate_quartz_surface_to_png (cairo_surface_t *surface,
                                          const char      *dest)
{
    CGContextRef context = cairo_quartz_surface_get_cg_context (surface);
    if (!context || !cg_context_is_bitmap (context)) {
        return CAIRO_STATUS_SURFACE_TYPE_MISMATCH;
    }

    CGImageRef image = CGBitmapContextCreateImage (context);
    CFStringRef png_utti = CFSTR("public.png");
    CFStringRef path;
    CFURLRef url;

    CGImageDestinationRef image_dest;

    path = CFStringCreateWithCString (NULL, dest, kCFStringEncodingUTF8);
    url = CFURLCreateWithFileSystemPath (NULL, path, kCFURLPOSIXPathStyle, FALSE);
    image_dest = CGImageDestinationCreateWithURL (url, png_utti, 1, NULL);

    CGImageDestinationAddImage (image_dest, image, NULL);
    CGImageDestinationFinalize (image_dest);

    CFRelease (url);
    CFRelease (path);

    CGImageRelease (image);
    return CAIRO_STATUS_SUCCESS;
}

static const cairo_boilerplate_target_t targets[] = {
    {
	"quartz", "quartz", NULL, NULL,
	CAIRO_SURFACE_TYPE_QUARTZ, CAIRO_CONTENT_COLOR_ALPHA, 0,
	"cairo_quartz_surface_create",
	_cairo_boilerplate_quartz_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	_cairo_boilerplate_quartz_surface_to_png,
	NULL, NULL, NULL,
	TRUE, FALSE, FALSE
    },
    {
	"quartz", "quartz", NULL, NULL,
	CAIRO_SURFACE_TYPE_QUARTZ, CAIRO_CONTENT_COLOR, 0,
	"cairo_quartz_surface_create",
	_cairo_boilerplate_quartz_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	_cairo_boilerplate_quartz_surface_to_png,
	NULL, NULL, NULL,
        FALSE, FALSE, FALSE
    },
};
CAIRO_BOILERPLATE (quartz, targets)
