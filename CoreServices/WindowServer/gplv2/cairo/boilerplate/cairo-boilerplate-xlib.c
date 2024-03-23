/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2004,2006 Red Hat, Inc.
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
#include "cairo-boilerplate-xlib.h"
#include "cairo-malloc-private.h"

#include <cairo-xlib.h>
#if CAIRO_HAS_XLIB_XRENDER_SURFACE
#include <cairo-xlib-xrender.h>
#endif

#include <X11/Xutil.h> /* for XDestroyImage */

#if !CAIRO_HAS_XLIB_XRENDER_SURFACE
#define PolyModePrecise			    0
#endif

static const cairo_user_data_key_t key;

typedef struct _xlib_target_closure {
    Display *dpy;
    Drawable drawable;
    cairo_bool_t drawable_is_pixmap;
} xlib_target_closure_t;

static void
_cairo_boilerplate_xlib_cleanup (void *closure)
{
    xlib_target_closure_t *xtc = closure;

    if (xtc->drawable) {
	if (xtc->drawable_is_pixmap)
	    XFreePixmap (xtc->dpy, xtc->drawable);
	else
	    XDestroyWindow (xtc->dpy, xtc->drawable);
    }
    XCloseDisplay (xtc->dpy);
    free (xtc);
}

static void
_cairo_boilerplate_xlib_synchronize (void *closure)
{
    xlib_target_closure_t *xtc = closure;
    XImage *ximage;

    ximage = XGetImage (xtc->dpy, xtc->drawable,
			0, 0, 1, 1, AllPlanes, ZPixmap);
    if (ximage != NULL)
	XDestroyImage (ximage);
}

static cairo_bool_t
_cairo_boilerplate_xlib_check_screen_size (Display *dpy,
					   int	    screen,
					   int	    width,
					   int	    height)
{
    Screen *scr = XScreenOfDisplay (dpy, screen);
    return width <= WidthOfScreen (scr) && height <= HeightOfScreen (scr);
}

static void
_cairo_boilerplate_xlib_setup_test_surface (cairo_surface_t *surface)
{

    /* For testing purposes, tell the X server to strictly adhere to the
     * Render specification.
     */
    cairo_xlib_device_debug_set_precision(cairo_surface_get_device(surface),
					  PolyModePrecise);
}


#if CAIRO_HAS_XLIB_XRENDER_SURFACE
/* For the xlib backend we distinguish between TEST and PERF mode in a
 * couple of ways.
 *
 * For TEST, we always test against pixmaps of depth 32 (for
 * COLOR_ALPHA) or 24 (for COLOR) and we use XSynchronize to make it
 * easier to debug problems.
 *
 * For PERF, we test against 32-bit pixmaps for COLOR_ALPHA, but for
 * COLOR we test against _windows_ at the depth of the default visual.
 * For obvious reasons, we don't use XSynchronize.
 */
static cairo_surface_t *
_cairo_boilerplate_xlib_test_create_surface (Display		   *dpy,
					     cairo_content_t	    content,
					     int		    width,
					     int		    height,
					     xlib_target_closure_t *xtc)
{
    XRenderPictFormat *xrender_format;
    cairo_surface_t *surface;

    /* This kills performance, but it makes debugging much
     * easier. That's why we have it here when in TEST mode, but not
     * over in PERF mode. */
    XSynchronize (xtc->dpy, 1);

    /* XXX: Currently we don't do any xlib testing when the X server
     * doesn't have the Render extension. We could do better here,
     * (perhaps by converting the tests from ARGB32 to RGB24). One
     * step better would be to always test the non-Render fallbacks
     * for each test even if the server does have the Render
     * extension. That would probably be through another
     * cairo_boilerplate_target which would use an extended version of
     * cairo_test_xlib_disable_render.	*/
    switch (content) {
    case CAIRO_CONTENT_COLOR_ALPHA:
	xrender_format = XRenderFindStandardFormat (dpy, PictStandardARGB32);
	break;
    case CAIRO_CONTENT_COLOR:
	xrender_format = XRenderFindStandardFormat (dpy, PictStandardRGB24);
	break;
    case CAIRO_CONTENT_ALPHA:
    default:
	CAIRO_BOILERPLATE_DEBUG (("Invalid content for xlib test: %d\n", content));
	return NULL;
    }
    if (xrender_format == NULL) {
	CAIRO_BOILERPLATE_DEBUG (("X server does not have the Render extension.\n"));
	return NULL;
    }

    xtc->drawable = XCreatePixmap (dpy, DefaultRootWindow (dpy),
				   width, height, xrender_format->depth);
    xtc->drawable_is_pixmap = TRUE;

    surface = cairo_xlib_surface_create_with_xrender_format (dpy, xtc->drawable,
							  DefaultScreenOfDisplay (dpy),
							  xrender_format,
							  width, height);

    _cairo_boilerplate_xlib_setup_test_surface(surface);

    return surface;
}

static cairo_surface_t *
_cairo_boilerplate_xlib_perf_create_surface (Display		   *dpy,
					     cairo_content_t	    content,
					     int		    width,
					     int		    height,
					     xlib_target_closure_t *xtc)
{
    XSetWindowAttributes attr;
    XRenderPictFormat *xrender_format;
    Visual *visual;

    switch (content) {
    case CAIRO_CONTENT_COLOR_ALPHA:
	xrender_format = XRenderFindStandardFormat (dpy, PictStandardARGB32);
	if (xrender_format == NULL) {
	    CAIRO_BOILERPLATE_DEBUG (("X server does not have the Render extension.\n"));
	    return NULL;
	}

	xtc->drawable = XCreatePixmap (dpy, DefaultRootWindow (dpy),
				       width, height, xrender_format->depth);
	xtc->drawable_is_pixmap = TRUE;
	break;

    case CAIRO_CONTENT_COLOR:
	if (! _cairo_boilerplate_xlib_check_screen_size (dpy,
							 DefaultScreen (dpy),
							 width, height)) {
	    CAIRO_BOILERPLATE_DEBUG (("Surface is larger than the Screen.\n"));
	    return NULL;
	}

	visual = DefaultVisual (dpy, DefaultScreen (dpy));
	xrender_format = XRenderFindVisualFormat (dpy, visual);
	if (xrender_format == NULL) {
	    CAIRO_BOILERPLATE_DEBUG (("X server does not have the Render extension.\n"));
	    return NULL;
	}

	attr.override_redirect = True;
	xtc->drawable = XCreateWindow (dpy, DefaultRootWindow (dpy), 0, 0,
				       width, height, 0, xrender_format->depth,
				       InputOutput, visual, CWOverrideRedirect, &attr);
	XMapWindow (dpy, xtc->drawable);
	xtc->drawable_is_pixmap = FALSE;
	break;

    case CAIRO_CONTENT_ALPHA:
    default:
	CAIRO_BOILERPLATE_DEBUG (("Invalid content for xlib test: %d\n", content));
	return NULL;
    }

    return cairo_xlib_surface_create_with_xrender_format (dpy, xtc->drawable,
							  DefaultScreenOfDisplay (dpy),
							  xrender_format,
							  width, height);
}

struct similar {
	Display *dpy;
	Pixmap pixmap;
};

static void _destroy_similar (void *closure)
{
    struct similar *similar = closure;

    XFreePixmap (similar->dpy, similar->pixmap);
    free (similar);
}

static cairo_surface_t *
_cairo_boilerplate_xlib_create_similar (cairo_surface_t		*other,
					cairo_content_t		 content,
					int			 width,
					int			 height)
{
    XRenderPictFormat *xrender_format;
    uint32_t format;
    struct similar *similar;
    cairo_surface_t *surface;

    similar = _cairo_malloc (sizeof (*similar));
    similar->dpy = cairo_xlib_surface_get_display (other);

    switch (content) {
    case CAIRO_CONTENT_COLOR:
        format = PictStandardRGB24;
        break;
    case CAIRO_CONTENT_ALPHA:
        format = PictStandardA8;
        break;
    case CAIRO_CONTENT_COLOR_ALPHA:
    default:
        format = PictStandardARGB32;
        break;
    }

    xrender_format = XRenderFindStandardFormat (similar->dpy, format);
    similar->pixmap = XCreatePixmap (similar->dpy,
				     DefaultRootWindow (similar->dpy),
				     width, height,
				     xrender_format->depth);

    surface =
	    cairo_xlib_surface_create_with_xrender_format (similar->dpy,
							   similar->pixmap,
							   DefaultScreenOfDisplay (similar->dpy),
							   xrender_format,
							   width, height);

    cairo_surface_set_user_data (surface, &key, similar, _destroy_similar);

    return surface;
}

static cairo_surface_t *
_cairo_boilerplate_xlib_create_surface (const char		  *name,
					cairo_content_t		   content,
					double			   width,
					double			   height,
					double			   max_width,
					double			   max_height,
					cairo_boilerplate_mode_t   mode,
					void			 **closure)
{
    xlib_target_closure_t *xtc;
    Display *dpy;
    cairo_surface_t *surface;

    *closure = xtc = xcalloc (1, sizeof (xlib_target_closure_t));

    width = ceil (width);
    if (width < 1)
	width = 1;

    height = ceil (height);
    if (height < 1)
	height = 1;

    xtc->dpy = dpy = XOpenDisplay (NULL);
    if (xtc->dpy == NULL) {
	free (xtc);
	CAIRO_BOILERPLATE_DEBUG (("Failed to open display: %s\n", XDisplayName(0)));
	return NULL;
    }

    if (mode == CAIRO_BOILERPLATE_MODE_TEST)
	surface = _cairo_boilerplate_xlib_test_create_surface (dpy, content, width, height, xtc);
    else /* mode == CAIRO_BOILERPLATE_MODE_PERF */
	surface = _cairo_boilerplate_xlib_perf_create_surface (dpy, content, width, height, xtc);

    if (surface == NULL || cairo_surface_status (surface))
	_cairo_boilerplate_xlib_cleanup (xtc);

    return surface;
}

static cairo_surface_t *
_cairo_boilerplate_xlib_render_0_0_create_surface (const char		  *name,
						   cairo_content_t		   content,
						   double			   width,
						   double			   height,
						   double			   max_width,
						   double			   max_height,
						   cairo_boilerplate_mode_t   mode,
						   void			 **closure)
{
    xlib_target_closure_t *xtc;
    Display *dpy;
    int screen;
    Pixmap pixmap;
    cairo_surface_t *surface, *dummy;

    *closure = xtc = xcalloc (1, sizeof (xlib_target_closure_t));

    width = ceil (width);
    if (width < 1)
	width = 1;

    height = ceil (height);
    if (height < 1)
	height = 1;

    xtc->dpy = dpy = XOpenDisplay (NULL);
    if (xtc->dpy == NULL) {
	free (xtc);
	CAIRO_BOILERPLATE_DEBUG (("Failed to open display: %s\n", XDisplayName(0)));
	return NULL;
    }


    screen = DefaultScreen (dpy);
    pixmap = XCreatePixmap (dpy, DefaultRootWindow (dpy), 1, 1,
			    DefaultDepth (dpy, screen));
    dummy = cairo_xlib_surface_create (dpy, pixmap,
				       DefaultVisual (dpy, screen),
				       1, 1);
    cairo_xlib_device_debug_cap_xrender_version (cairo_surface_get_device (dummy),
						 0, 0);

    if (mode == CAIRO_BOILERPLATE_MODE_TEST)
	surface = _cairo_boilerplate_xlib_test_create_surface (dpy, content, width, height, xtc);
    else /* mode == CAIRO_BOILERPLATE_MODE_PERF */
	surface = _cairo_boilerplate_xlib_perf_create_surface (dpy, content, width, height, xtc);

    cairo_surface_destroy (dummy);
    XFreePixmap (dpy, pixmap);

    if (surface == NULL || cairo_surface_status (surface))
	_cairo_boilerplate_xlib_cleanup (xtc);

    return surface;
}

static cairo_surface_t *
_cairo_boilerplate_xlib_window_create_surface (const char		 *name,
					       cairo_content_t		  content,
					       double			  width,
					       double			  height,
					       double			  max_width,
					       double			  max_height,
					       cairo_boilerplate_mode_t   mode,
					       void			**closure)
{
    xlib_target_closure_t *xtc;
    Display *dpy;
    int screen;
    XSetWindowAttributes attr;
    cairo_surface_t *surface;

    /* We're not yet bothering to support perf mode for the
     * xlib-fallback surface. */
    if (mode == CAIRO_BOILERPLATE_MODE_PERF)
	return NULL;

    /* We also don't support drawing with destination-alpha in the
     * xlib-fallback surface. */
    if (content == CAIRO_CONTENT_COLOR_ALPHA)
	return NULL;

    *closure = xtc = xmalloc (sizeof (xlib_target_closure_t));

    width = ceil (width);
    if (width < 1)
	width = 1;

    height = ceil (height);
    if (height < 1)
	height = 1;

    xtc->dpy = dpy = XOpenDisplay (NULL);
    if (xtc->dpy == NULL) {
	CAIRO_BOILERPLATE_DEBUG (("Failed to open display: %s\n", XDisplayName(0)));
	free (xtc);
	return NULL;
    }

    /* This kills performance, but it makes debugging much
     * easier. That's why we have it here only after explicitly not
     * supporting PERF mode.*/
    XSynchronize (dpy, 1);

    screen = DefaultScreen (dpy);
    if (! _cairo_boilerplate_xlib_check_screen_size (dpy, screen,
						     width, height)) {
	CAIRO_BOILERPLATE_DEBUG (("Surface is larger than the Screen.\n"));
	XCloseDisplay (dpy);
	free (xtc);
	return NULL;
    }

    attr.override_redirect = True;
    xtc->drawable = XCreateWindow (dpy, DefaultRootWindow (dpy),
				   0, 0,
				   width, height, 0,
				   DefaultDepth (dpy, screen),
				   InputOutput,
				   DefaultVisual (dpy, screen),
				   CWOverrideRedirect, &attr);
    XMapWindow (dpy, xtc->drawable);
    xtc->drawable_is_pixmap = FALSE;

    surface = cairo_xlib_surface_create (dpy, xtc->drawable,
					 DefaultVisual (dpy, screen),
					 width, height);
    if (cairo_surface_status (surface))
	_cairo_boilerplate_xlib_cleanup (xtc);

    _cairo_boilerplate_xlib_setup_test_surface(surface);

    return surface;
}
#endif


#if CAIRO_HAS_XLIB_SURFACE
/* The xlib-fallback target differs from the xlib target in two ways:
 *
 * 1. It creates its surfaces without relying on the Render extension
 *
 * 2. It disables use of the Render extension for its surfaces
 *
 * This provides testing of the non-Render fallback paths we have in
 * cairo-xlib-surface.c
 */
static cairo_surface_t *
_cairo_boilerplate_xlib_fallback_create_surface (const char		   *name,
						 cairo_content_t	    content,
						 double 		    width,
						 double 		    height,
						 double 		    max_width,
						 double 		    max_height,
						 cairo_boilerplate_mode_t   mode,
						 void			  **closure)
{
    xlib_target_closure_t *xtc;
    Display *dpy;
    int screen;
    XSetWindowAttributes attr;
    cairo_surface_t *surface, *dummy;

    /* We're not yet bothering to support perf mode for the
     * xlib-fallback surface. */
    if (mode == CAIRO_BOILERPLATE_MODE_PERF)
	return NULL;

    /* We also don't support drawing with destination-alpha in the
     * xlib-fallback surface. */
    if (content == CAIRO_CONTENT_COLOR_ALPHA)
	return NULL;

    *closure = xtc = xmalloc (sizeof (xlib_target_closure_t));

    width = ceil (width);
    if (width < 1)
	width = 1;

    height = ceil (height);
    if (height < 1)
	height = 1;

    xtc->dpy = dpy = XOpenDisplay (NULL);
    if (xtc->dpy == NULL) {
	CAIRO_BOILERPLATE_DEBUG (("Failed to open display: %s\n", XDisplayName(0)));
	free (xtc);
	return NULL;
    }

    /* This kills performance, but it makes debugging much
     * easier. That's why we have it here only after explicitly not
     * supporting PERF mode.*/
    XSynchronize (dpy, 1);

    screen = DefaultScreen (dpy);
    if (! _cairo_boilerplate_xlib_check_screen_size (dpy, screen,
						     width, height)) {
	CAIRO_BOILERPLATE_DEBUG (("Surface is larger than the Screen.\n"));
	XCloseDisplay (dpy);
	free (xtc);
	return NULL;
    }

    attr.override_redirect = True;
    xtc->drawable = XCreateWindow (dpy, DefaultRootWindow (dpy),
				   0, 0,
				   width, height, 0,
				   DefaultDepth (dpy, screen),
				   InputOutput,
				   DefaultVisual (dpy, screen),
				   CWOverrideRedirect, &attr);
    XMapWindow (dpy, xtc->drawable);
    xtc->drawable_is_pixmap = FALSE;

    dummy = cairo_xlib_surface_create (dpy, xtc->drawable,
				       DefaultVisual (dpy, screen),
				       width, height);
    cairo_xlib_device_debug_cap_xrender_version (cairo_surface_get_device (dummy),
						 -1, -1);

    surface = cairo_xlib_surface_create (dpy, xtc->drawable,
					 DefaultVisual (dpy, screen),
					 width, height);
    cairo_surface_destroy (dummy);
    if (cairo_surface_status (surface))
	_cairo_boilerplate_xlib_cleanup (xtc);

    _cairo_boilerplate_xlib_setup_test_surface(surface);

    return surface;
}
#endif

static const cairo_boilerplate_target_t targets[] = {
#if CAIRO_HAS_XLIB_XRENDER_SURFACE
    /* Acceleration architectures may make the results differ by a
     * bit, so we set the error tolerance to 1. */
    {
	"xlib", "traps", NULL, "xlib-fallback",
	CAIRO_SURFACE_TYPE_XLIB, CAIRO_CONTENT_COLOR_ALPHA, 1,
	"cairo_xlib_surface_create_with_xrender_format",
	_cairo_boilerplate_xlib_create_surface,
	_cairo_boilerplate_xlib_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_xlib_cleanup,
	_cairo_boilerplate_xlib_synchronize,
        NULL,
	TRUE, FALSE, FALSE
    },
    {
	"xlib", "traps", NULL, "xlib-fallback",
	CAIRO_SURFACE_TYPE_XLIB, CAIRO_CONTENT_COLOR, 1,
	"cairo_xlib_surface_create_with_xrender_format",
	_cairo_boilerplate_xlib_create_surface,
	_cairo_boilerplate_xlib_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_xlib_cleanup,
	_cairo_boilerplate_xlib_synchronize,
        NULL,
	FALSE, FALSE, FALSE
    },
    {
	"xlib-window", "traps", NULL, NULL,
	CAIRO_SURFACE_TYPE_XLIB, CAIRO_CONTENT_COLOR, 1,
	"cairo_xlib_surface_create",
	_cairo_boilerplate_xlib_window_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_xlib_cleanup,
	_cairo_boilerplate_xlib_synchronize,
        NULL,
	FALSE, FALSE, FALSE
    },
    {
	"xlib-render-0_0", "mask", NULL, NULL,
	CAIRO_SURFACE_TYPE_XLIB, CAIRO_CONTENT_COLOR, 1,
	"cairo_xlib_surface_create",
	_cairo_boilerplate_xlib_render_0_0_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_xlib_cleanup,
	_cairo_boilerplate_xlib_synchronize,
        NULL,
	FALSE, FALSE, FALSE
    },
#endif
#if CAIRO_HAS_XLIB_SURFACE
    /* This is a fallback surface which uses xlib fallbacks instead of
     * the Render extension. */
    {
	"xlib-fallback", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_XLIB, CAIRO_CONTENT_COLOR, 1,
	"cairo_xlib_surface_create",
	_cairo_boilerplate_xlib_fallback_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_xlib_cleanup,
	_cairo_boilerplate_xlib_synchronize,
        NULL,
	FALSE, FALSE, FALSE
    },
#endif
};
CAIRO_BOILERPLATE (xlib, targets)
