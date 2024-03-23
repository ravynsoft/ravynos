/*
 * Copyright © 2008 Chris Wilson <chris@chris-wilson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Chris Wilson.
 *
 * Contributor(s):
 *      Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "config.h"

#include "cairo.h"
#include "cairo-script-interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(CAIRO_HAS_XLIB_SURFACE) || defined(CAIRO_HAS_XLIB_XRENDER_SURFACE)
static const cairo_user_data_key_t _key;
#endif

#define SINGLE_SURFACE 1

#if SINGLE_SURFACE
static cairo_surface_t *
_similar_surface_create (void *closure,
			 cairo_content_t content,
			 double width, double height,
			 long uid)
{
    return cairo_surface_create_similar (closure, content, width, height);
}

static struct list {
    struct list *next;
    cairo_t *context;
    cairo_surface_t *surface;
} *list;

static cairo_t *
_context_create (void *closure, cairo_surface_t *surface)
{
    cairo_t *cr = cairo_create (surface);
    struct list *l = malloc (sizeof (*l));
    l->next = list;
    l->context = cr;
    l->surface = cairo_surface_reference (surface);
    list = l;
    return cr;
}

static void
_context_destroy (void *closure, void *ptr)
{
    struct list *l, **prev = &list;
    while ((l = *prev) != NULL) {
	if (l->context == ptr) {
	    if (cairo_surface_status (l->surface) == CAIRO_STATUS_SUCCESS) {
		cairo_t *cr = cairo_create (closure);
		cairo_set_source_surface (cr, l->surface, 0, 0);
		cairo_paint (cr);
		cairo_destroy (cr);
	    }

	    cairo_surface_destroy (l->surface);
	    *prev = l->next;
	    free (l);
	    return;
	}
	prev = &l->next;
    }
}
#endif

#if CAIRO_HAS_XLIB_SURFACE
#include <cairo-xlib.h>
static Display *
_get_display (void)
{
    static Display *dpy;

    if (dpy != NULL)
	return dpy;

    dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
	fprintf (stderr, "Failed to open display.\n");
	exit (1);
    }

    return dpy;
}

static void
_destroy_window (void *closure)
{
    XFlush (_get_display ());
    XDestroyWindow (_get_display(), (Window) closure);
}

static cairo_surface_t *
_xlib_surface_create (void *closure,
		      cairo_content_t content,
		      double width, double height,
		      long uid)
{
    Display *dpy;
    XSetWindowAttributes attr;
    Visual *visual;
    int depth;
    Window w;
    cairo_surface_t *surface;

    dpy = _get_display ();

    visual = DefaultVisual (dpy, DefaultScreen (dpy));
    depth = DefaultDepth (dpy, DefaultScreen (dpy));
    attr.override_redirect = True;
    w = XCreateWindow (dpy, DefaultRootWindow (dpy), 0, 0,
		       width <= 0 ? 1 : width,
		       height <= 0 ? 1 : height,
		       0, depth,
		       InputOutput, visual, CWOverrideRedirect, &attr);
    XMapWindow (dpy, w);

    surface = cairo_xlib_surface_create (dpy, w, visual, width, height);
    cairo_surface_set_user_data (surface, &_key, (void *) w, _destroy_window);

    return surface;
}

#if CAIRO_HAS_XLIB_XRENDER_SURFACE
#include <cairo-xlib-xrender.h>

static void
_destroy_pixmap (void *closure)
{
    XFreePixmap (_get_display(), (Pixmap) closure);
}

static cairo_surface_t *
_xrender_surface_create (void *closure,
			 cairo_content_t content,
			 double width, double height,
			 long uid)
{
    Display *dpy;
    Pixmap pixmap;
    XRenderPictFormat *xrender_format;
    cairo_surface_t *surface;

    dpy = _get_display ();

    content = CAIRO_CONTENT_COLOR_ALPHA;

    switch (content) {
    case CAIRO_CONTENT_COLOR_ALPHA:
	xrender_format = XRenderFindStandardFormat (dpy, PictStandardARGB32);
	break;
    case CAIRO_CONTENT_COLOR:
	xrender_format = XRenderFindStandardFormat (dpy, PictStandardRGB24);
	break;
    case CAIRO_CONTENT_ALPHA:
    default:
	xrender_format = XRenderFindStandardFormat (dpy, PictStandardA8);
    }

    pixmap = XCreatePixmap (dpy, DefaultRootWindow (dpy),
		       width, height, xrender_format->depth);

    surface = cairo_xlib_surface_create_with_xrender_format (dpy, pixmap,
							     DefaultScreenOfDisplay (dpy),
							     xrender_format,
							     width, height);
    cairo_surface_set_user_data (surface, &_key,
				 (void *) pixmap, _destroy_pixmap);

    return surface;
}
#endif
#endif

#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
static cairo_surface_t *
_pdf_surface_create (void *closure,
		     cairo_content_t content,
		     double width, double height,
		     long uid)
{
    return cairo_pdf_surface_create_for_stream (NULL, NULL, width, height);
}
#endif

#if CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
static cairo_surface_t *
_ps_surface_create (void *closure,
		    cairo_content_t content,
		    double width, double height,
		    long uid)
{
    return cairo_ps_surface_create_for_stream (NULL, NULL, width, height);
}
#endif

#if CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
static cairo_surface_t *
_svg_surface_create (void *closure,
		     cairo_content_t content,
		     double width, double height,
		     long uid)
{
    return cairo_svg_surface_create_for_stream (NULL, NULL, width, height);
}
#endif

static cairo_surface_t *
_image_surface_create (void *closure,
		       cairo_content_t content,
		       double width, double height,
		       long uid)
{
    return cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
}

int
main (int argc, char **argv)
{
    cairo_script_interpreter_t *csi;
    cairo_script_interpreter_hooks_t hooks = {
#if SINGLE_SURFACE
	.surface_create = _similar_surface_create,
	.context_create = _context_create,
	.context_destroy = _context_destroy
#elif CAIRO_HAS_XLIB_XRENDER_SURFACE
	.surface_create = _xrender_surface_create
#elif CAIRO_HAS_XLIB_SURFACE
	.surface_create = _xlib_surface_create
#elif CAIRO_PDF_SURFACE
	.surface_create = _pdf_surface_create
#elif CAIRO_PS_SURFACE
	.surface_create = _ps_surface_create
#elif CAIRO_SVG_SURFACE
	.surface_create = _svg_surface_create
#else
	.surface_create = _image_surface_create
#endif
    };
    int i;
    const struct backends {
	const char *name;
	csi_surface_create_func_t create;
    } backends[] = {
	{ "--image", _image_surface_create },
#if CAIRO_HAS_XLIB_XRENDER_SURFACE
	{ "--xrender", _xrender_surface_create },
#endif
#if CAIRO_HAS_XLIB_SURFACE
	{ "--xlib", _xlib_surface_create },
#endif
#if CAIRO_HAS_PDF_SURFACE
	{ "--pdf", _pdf_surface_create },
#endif
#if CAIRO_HAS_PS_SURFACE
	{ "--ps", _ps_surface_create },
#endif
#if CAIRO_HAS_SVG_SURFACE
	{ "--svg", _svg_surface_create },
#endif
	{ NULL, NULL }
    };

#if SINGLE_SURFACE
    hooks.closure = backends[0].create (NULL,
					CAIRO_CONTENT_COLOR_ALPHA,
					512, 512,
					0);
#endif


    csi = cairo_script_interpreter_create ();
    cairo_script_interpreter_install_hooks (csi, &hooks);

    for (i = 1; i < argc; i++) {
	const struct backends *b;

	for (b = backends; b->name != NULL; b++) {
	    if (strcmp (b->name, argv[i]) == 0) {
#if SINGLE_SURFACE
		cairo_surface_destroy (hooks.closure);
		hooks.closure = b->create (NULL,
					   CAIRO_CONTENT_COLOR_ALPHA,
					   512, 512,
					   0);
#else
		hooks.surface_create = b->create;
#endif
		cairo_script_interpreter_install_hooks (csi, &hooks);
		break;
	    }
	}

	if (b->name == NULL)
	    cairo_script_interpreter_run (csi, argv[i]);
    }
    cairo_surface_destroy (hooks.closure);

    return cairo_script_interpreter_destroy (csi);
}
