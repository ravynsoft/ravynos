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
#include <cairo-xlib.h>
#if CAIRO_HAS_XLIB_XRENDER_SURFACE
#include <cairo-xlib-xrender.h>
#endif

#include "surface-source.c"

static cairo_user_data_key_t closure_key;

struct closure {
    cairo_device_t *device;
    Display *dpy;
    Pixmap pix;
};

static void
cleanup (void *data)
{
    struct closure *arg = data;

    cairo_device_finish (arg->device);
    cairo_device_destroy (arg->device);

    XFreePixmap (arg->dpy, arg->pix);
    XCloseDisplay (arg->dpy);

    free (arg);
}

static cairo_surface_t *
create_source_surface (int size)
{
#if CAIRO_HAS_XLIB_XRENDER_SURFACE
    XRenderPictFormat *xrender_format;
    struct closure *data;
    cairo_surface_t *surface;

    data = xmalloc (sizeof (struct closure));

    data->dpy = XOpenDisplay (NULL);
    if (!data->dpy) {
	return NULL;
    }

    xrender_format = XRenderFindStandardFormat (data->dpy, PictStandardARGB32);

    data->pix = XCreatePixmap (data->dpy, DefaultRootWindow (data->dpy),
			       size, size, xrender_format->depth);

    surface = cairo_xlib_surface_create_with_xrender_format (data->dpy,
	                                                     data->pix,
							     DefaultScreenOfDisplay (data->dpy),
							     xrender_format,
							     size, size);
    data->device = cairo_device_reference (cairo_surface_get_device (surface));
    if (cairo_surface_set_user_data (surface, &closure_key, data, cleanup)) {
	cairo_surface_finish (surface);
	cairo_surface_destroy (surface);
	cleanup (data);
	return NULL;
    }

    return surface;
#else
    return NULL;
#endif
}

CAIRO_TEST (xlib_surface_source,
	    "Test using a Xlib surface as the source",
	    "source", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    preamble, draw)
