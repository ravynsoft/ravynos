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

#if CAIRO_CAN_TEST_SVG_SURFACE

#include <cairo-svg.h>
#include <cairo-svg-surface-private.h>
#include <cairo-paginated-surface-private.h>

#if HAVE_SIGNAL_H
#include <stdlib.h>
#include <signal.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if ! CAIRO_HAS_RECORDING_SURFACE
#define CAIRO_SURFACE_TYPE_RECORDING CAIRO_INTERNAL_SURFACE_TYPE_RECORDING
#endif

static const cairo_user_data_key_t svg_closure_key;

typedef struct _svg_target_closure {
    char    *filename;
    int     width, height;
    cairo_surface_t	*target;
} svg_target_closure_t;

static cairo_surface_t *
_cairo_boilerplate_svg_create_surface (const char		 *name,
				       cairo_content_t		  content,
				       cairo_svg_version_t	  version,
				       double			  width,
				       double			  height,
				       double			  max_width,
				       double			  max_height,
				       cairo_boilerplate_mode_t   mode,
				       void			**closure)
{
    svg_target_closure_t *ptc;
    cairo_surface_t *surface;
    cairo_status_t status;

    *closure = ptc = xmalloc (sizeof (svg_target_closure_t));

    ptc->width = ceil (width);
    ptc->height = ceil (height);

    xasprintf (&ptc->filename, "%s.out.svg", name);
    xunlink (ptc->filename);

    surface = cairo_svg_surface_create (ptc->filename, width, height);
    if (cairo_surface_status (surface))
	goto CLEANUP_FILENAME;

    cairo_svg_surface_restrict_to_version (surface, version);
    cairo_surface_set_fallback_resolution (surface, 72., 72.);

    if (content == CAIRO_CONTENT_COLOR) {
	ptc->target = surface;
	surface = cairo_surface_create_similar (ptc->target,
						CAIRO_CONTENT_COLOR,
						ptc->width, ptc->height);
	if (cairo_surface_status (surface))
	    goto CLEANUP_TARGET;
    } else
	ptc->target = NULL;

    status = cairo_surface_set_user_data (surface, &svg_closure_key, ptc, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return surface;

    cairo_surface_destroy (surface);
    surface = cairo_boilerplate_surface_create_in_error (status);

  CLEANUP_TARGET:
    cairo_surface_destroy (ptc->target);
  CLEANUP_FILENAME:
    free (ptc->filename);
    free (ptc);
    return surface;
}

static cairo_surface_t *
_cairo_boilerplate_svg11_create_surface (const char		   *name,
					 cairo_content_t	    content,
					 double 		    width,
					 double 		    height,
					 double 		    max_width,
					 double 		    max_height,
					 cairo_boilerplate_mode_t   mode,
					 void			  **closure)
{
    /* current default, but be explicit in case the default changes */
    return _cairo_boilerplate_svg_create_surface (name, content,
						  CAIRO_SVG_VERSION_1_1,
						  width, height,
						  max_width, max_height,
						  mode,
						  closure);
}

static cairo_status_t
_cairo_boilerplate_svg_finish_surface (cairo_surface_t *surface)
{
    svg_target_closure_t *ptc = cairo_surface_get_user_data (surface,
							     &svg_closure_key);
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
	status = cairo_status (cr);
	cairo_destroy (cr);
	cairo_surface_set_device_offset (surface, x_offset, y_offset);
	cairo_surface_set_device_scale (surface, x_scale, y_scale);

	if (status)
	    return status;

	cairo_surface_finish (surface);
	status = cairo_surface_status (surface);
	if (status)
	    return status;

	surface = ptc->target;
    }

    cairo_surface_finish (surface);
    status = cairo_surface_status (surface);
    if (status)
	return status;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_boilerplate_svg_surface_write_to_png (cairo_surface_t *surface,
					     const char      *filename)
{
    svg_target_closure_t *ptc = cairo_surface_get_user_data (surface,
							     &svg_closure_key);
    char    command[4096];
    int exitstatus;

    sprintf (command, "./svg2png %s %s",
	     ptc->filename, filename);

    exitstatus = system (command);
#if _XOPEN_SOURCE && HAVE_SIGNAL_H
    if (WIFSIGNALED (exitstatus))
	raise (WTERMSIG (exitstatus));
#endif
    if (exitstatus)
	return CAIRO_STATUS_WRITE_ERROR;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t *
_cairo_boilerplate_svg_convert_to_image (cairo_surface_t *surface)
{
    svg_target_closure_t *ptc = cairo_surface_get_user_data (surface,
							     &svg_closure_key);

    return cairo_boilerplate_convert_to_image (ptc->filename, 0);
}

static cairo_surface_t *
_cairo_boilerplate_svg_get_image_surface (cairo_surface_t *surface,
					  int		   page,
					  int		   width,
					  int		   height)
{
    cairo_surface_t *image;
    double x_offset, y_offset;
    double x_scale, y_scale;

    if (page != 0)
	return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);

    image = _cairo_boilerplate_svg_convert_to_image (surface);
    cairo_surface_get_device_offset (surface, &x_offset, &y_offset);
    cairo_surface_get_device_scale (surface, &x_scale, &y_scale);
    cairo_surface_set_device_offset (image, x_offset, y_offset);
    cairo_surface_set_device_scale (image, x_scale, y_scale);
    surface = _cairo_boilerplate_get_image_surface (image, 0, width, height);
    cairo_surface_destroy (image);

    return surface;
}

static void
_cairo_boilerplate_svg_cleanup (void *closure)
{
    svg_target_closure_t *ptc = closure;
    if (ptc->target != NULL) {
	cairo_surface_finish (ptc->target);
	cairo_surface_destroy (ptc->target);
    }
    free (ptc->filename);
    free (ptc);
}

static void
_cairo_boilerplate_svg_force_fallbacks (cairo_surface_t *abstract_surface,
				       double		 x_pixels_per_inch,
				       double		 y_pixels_per_inch)
{
    svg_target_closure_t *ptc = cairo_surface_get_user_data (abstract_surface,
							     &svg_closure_key);

    if (ptc->target)
	abstract_surface = ptc->target;

    cairo_paginated_surface_t *paginated = (cairo_paginated_surface_t*) abstract_surface;
    _cairo_svg_surface_set_force_fallbacks (paginated->target, TRUE);
    cairo_surface_set_fallback_resolution (&paginated->base,
					   x_pixels_per_inch,
					   y_pixels_per_inch);
}

static const cairo_boilerplate_target_t targets[] = {
    /* It seems we should be able to round-trip SVG content perfectly
     * through librsvg and cairo, but for some mysterious reason, some
     * systems get an error of 1 for some pixels on some of the text
     * tests. XXX: I'd still like to chase these down at some point.
     * For now just set the svg error tolerance to 1. */
    {
	"svg11", "svg", ".svg", NULL,
	CAIRO_SURFACE_TYPE_SVG, CAIRO_CONTENT_COLOR_ALPHA, 1,
	"cairo_svg_surface_create",
	_cairo_boilerplate_svg11_create_surface,
	cairo_surface_create_similar,
	_cairo_boilerplate_svg_force_fallbacks,
	_cairo_boilerplate_svg_finish_surface,
	_cairo_boilerplate_svg_get_image_surface,
	_cairo_boilerplate_svg_surface_write_to_png,
	_cairo_boilerplate_svg_cleanup,
	NULL, NULL, FALSE, TRUE, TRUE
    },
    {
	"svg11", "svg", ".svg", NULL,
	CAIRO_SURFACE_TYPE_RECORDING, CAIRO_CONTENT_COLOR, 1,
	"cairo_svg_surface_create",
	_cairo_boilerplate_svg11_create_surface,
	cairo_surface_create_similar,
	_cairo_boilerplate_svg_force_fallbacks,
	_cairo_boilerplate_svg_finish_surface,
	_cairo_boilerplate_svg_get_image_surface,
	_cairo_boilerplate_svg_surface_write_to_png,
	_cairo_boilerplate_svg_cleanup,
	NULL, NULL, FALSE, TRUE, TRUE
    },
};
CAIRO_BOILERPLATE (svg, targets)

#else

CAIRO_NO_BOILERPLATE (svg)

#endif
