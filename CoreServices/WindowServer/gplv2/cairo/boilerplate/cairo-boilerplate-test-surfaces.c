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

#include <cairo-types-private.h>

#include <test-compositor-surface.h>
#include <test-null-compositor-surface.h>
#if CAIRO_HAS_TEST_PAGINATED_SURFACE
#include <test-paginated-surface.h>
#endif

static cairo_surface_t *
_cairo_boilerplate_test_base_compositor_create_surface (const char		   *name,
							cairo_content_t	    content,
							double		    width,
							double		    height,
							double		    max_width,
							double		    max_height,
							cairo_boilerplate_mode_t mode,
							void		  **closure)
{
    *closure = NULL;
    return _cairo_test_base_compositor_surface_create (content, ceil (width), ceil (height));
}


static cairo_surface_t *
_cairo_boilerplate_test_fallback_compositor_create_surface (const char		   *name,
							    cairo_content_t	    content,
							    double		    width,
							    double		    height,
							    double		    max_width,
							    double		    max_height,
							    cairo_boilerplate_mode_t mode,
							    void		  **closure)
{
    *closure = NULL;
    return _cairo_test_fallback_compositor_surface_create (content, ceil (width), ceil (height));
}

static cairo_surface_t *
_cairo_boilerplate_test_mask_compositor_create_surface (const char		   *name,
							 cairo_content_t	    content,
							 double			    width,
							 double			    height,
							 double			    max_width,
							 double			    max_height,
							 cairo_boilerplate_mode_t   mode,
							 void			  **closure)
{
    *closure = NULL;
    return _cairo_test_mask_compositor_surface_create (content, ceil (width), ceil (height));
}


static cairo_surface_t *
_cairo_boilerplate_test_traps_compositor_create_surface (const char		   *name,
							 cairo_content_t	    content,
							 double			    width,
							 double			    height,
							 double			    max_width,
							 double			    max_height,
							 cairo_boilerplate_mode_t   mode,
							 void			  **closure)
{
    *closure = NULL;
    return _cairo_test_traps_compositor_surface_create (content, ceil (width), ceil (height));
}

static cairo_surface_t *
_cairo_boilerplate_test_spans_compositor_create_surface (const char		   *name,
							 cairo_content_t	    content,
							 double			    width,
							 double			    height,
							 double			    max_width,
							 double			    max_height,
							 cairo_boilerplate_mode_t   mode,
							 void			  **closure)
{
    *closure = NULL;
    return _cairo_test_spans_compositor_surface_create (content, ceil (width), ceil (height));
}

static cairo_surface_t *
_cairo_boilerplate_test_no_fallback_compositor_create_surface (const char		   *name,
							       cairo_content_t	    content,
							       double			    width,
							       double			    height,
							       double			    max_width,
							       double			    max_height,
							       cairo_boilerplate_mode_t   mode,
							       void			  **closure)
{
    if (mode == CAIRO_BOILERPLATE_MODE_TEST)
	return NULL;

    *closure = NULL;
    return _cairo_test_no_fallback_compositor_surface_create (content, ceil (width), ceil (height));
}

static cairo_surface_t *
_cairo_boilerplate_test_no_traps_compositor_create_surface (const char		   *name,
							 cairo_content_t	    content,
							 double			    width,
							 double			    height,
							 double			    max_width,
							 double			    max_height,
							 cairo_boilerplate_mode_t   mode,
							 void			  **closure)
{
    if (mode == CAIRO_BOILERPLATE_MODE_TEST)
	return NULL;

    *closure = NULL;
    return _cairo_test_no_traps_compositor_surface_create (content, ceil (width), ceil (height));
}

static cairo_surface_t *
_cairo_boilerplate_test_no_spans_compositor_create_surface (const char		   *name,
							 cairo_content_t	    content,
							 double			    width,
							 double			    height,
							 double			    max_width,
							 double			    max_height,
							 cairo_boilerplate_mode_t   mode,
							 void			  **closure)
{
    if (mode == CAIRO_BOILERPLATE_MODE_TEST)
	return NULL;

    *closure = NULL;
    return _cairo_test_no_spans_compositor_surface_create (content, ceil (width), ceil (height));
}

#if CAIRO_HAS_TEST_PAGINATED_SURFACE
static const cairo_user_data_key_t test_paginated_closure_key;

typedef struct {
    cairo_surface_t *target;
} test_paginated_closure_t;

static cairo_surface_t *
_cairo_boilerplate_test_paginated_create_surface (const char		    *name,
						  cairo_content_t	     content,
						  double		     width,
						  double		     height,
						  double		     max_width,
						  double		     max_height,
						  cairo_boilerplate_mode_t   mode,
						  void			   **closure)
{
    test_paginated_closure_t *tpc;
    cairo_format_t format;
    cairo_surface_t *surface;
    cairo_status_t status;

    *closure = tpc = xmalloc (sizeof (test_paginated_closure_t));

    format = cairo_boilerplate_format_from_content (content);
    tpc->target = cairo_image_surface_create (format,
					      ceil (width), ceil (height));

    surface = _cairo_test_paginated_surface_create (tpc->target);
    if (cairo_surface_status (surface))
	goto CLEANUP;

    status = cairo_surface_set_user_data (surface,
					  &test_paginated_closure_key,
					  tpc, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return surface;

    cairo_surface_destroy (surface);
    surface = cairo_boilerplate_surface_create_in_error (status);

    cairo_surface_destroy (tpc->target);

  CLEANUP:
    free (tpc);
    return surface;
}

/* The only reason we go through all these machinations to write a PNG
 * image is to _really ensure_ that the data actually landed in our
 * buffer through the paginated surface to the test_paginated_surface.
 *
 * If we didn't implement this function then the default
 * cairo_surface_write_to_png would result in the paginated_surface's
 * acquire_source_image function replaying the recording-surface to an
 * intermediate image surface. And in that case the
 * test_paginated_surface would not be involved and wouldn't be
 * tested.
 */
static cairo_status_t
_cairo_boilerplate_test_paginated_surface_write_to_png (cairo_surface_t *surface,
							const char	*filename)
{
    test_paginated_closure_t *tpc;
    cairo_status_t status;

    /* show page first.  the automatic show_page is too late for us */
    cairo_surface_show_page (surface);
    status = cairo_surface_status (surface);
    if (status)
	return status;

    tpc = cairo_surface_get_user_data (surface, &test_paginated_closure_key);
    return cairo_surface_write_to_png (tpc->target, filename);
}

static cairo_surface_t *
_cairo_boilerplate_test_paginated_get_image_surface (cairo_surface_t *surface,
						     int	      page,
						     int	      width,
						     int	      height)
{
    test_paginated_closure_t *tpc;
    cairo_status_t status;

    /* XXX separate finish as per PDF */
    if (page != 0)
	return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);

    /* show page first.  the automatic show_page is too late for us */
    cairo_surface_show_page (surface);
    status = cairo_surface_status (surface);
    if (status)
	return cairo_boilerplate_surface_create_in_error (status);

    tpc = cairo_surface_get_user_data (surface, &test_paginated_closure_key);
    return _cairo_boilerplate_get_image_surface (tpc->target, 0, width, height);
}

static void
_cairo_boilerplate_test_paginated_cleanup (void *closure)
{
    test_paginated_closure_t *tpc = closure;

    cairo_surface_destroy (tpc->target);
    free (tpc);
}
#endif

static const cairo_boilerplate_target_t targets[] = {
    {
	"test-base", "base", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_base_compositor_surface_create",
	_cairo_boilerplate_test_base_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, TRUE, FALSE, FALSE
    },
    {
	"test-base", "base", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR, 0,
	"_cairo_test_base_compositor_surface_create",
	_cairo_boilerplate_test_base_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, FALSE, FALSE, FALSE
    },

    {
	"test-fallback", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_fallback_compositor_surface_create",
	_cairo_boilerplate_test_fallback_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, FALSE, FALSE, FALSE
    },
    {
	"test-fallback", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR, 0,
	"_cairo_test_fallback_compositor_surface_create",
	_cairo_boilerplate_test_fallback_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, FALSE, FALSE, FALSE
    },

    {
	"test-mask", "mask", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_mask_compositor_surface_create",
	_cairo_boilerplate_test_mask_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, TRUE, FALSE, FALSE
    },
    {
	"test-mask", "mask", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR, 0,
	"_cairo_test_mask_compositor_surface_create",
	_cairo_boilerplate_test_mask_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, FALSE, FALSE, FALSE
    },

    {
	"test-traps", "traps", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_traps_compositor_surface_create",
	_cairo_boilerplate_test_traps_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, TRUE, FALSE, FALSE
    },
    {
	"test-traps", "traps", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR, 0,
	"_cairo_test_traps_compositor_surface_create",
	_cairo_boilerplate_test_traps_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, FALSE, FALSE, FALSE
    },

    {
	"test-spans", "spans", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_spans_compositor_surface_create",
	_cairo_boilerplate_test_spans_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, TRUE, FALSE, FALSE
    },
    {
	"test-spans", "spans", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR, 0,
	"_cairo_test_spans_compositor_surface_create",
	_cairo_boilerplate_test_spans_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, FALSE, FALSE, FALSE
    },

    {
	"no-fallback", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_no_fallback_compositor_surface_create",
	_cairo_boilerplate_test_no_fallback_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, FALSE, FALSE, FALSE
    },
    {
	"no-traps", "traps", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_no_traps_compositor_surface_create",
	_cairo_boilerplate_test_no_traps_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, TRUE, FALSE, FALSE
    },
    {
	"no-spans", "spans", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_no_spans_compositor_surface_create",
	_cairo_boilerplate_test_no_spans_compositor_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL, NULL, TRUE, FALSE, FALSE
    },
#if CAIRO_HAS_TEST_PAGINATED_SURFACE
    {
	"test-paginated", "image", NULL, NULL,
	CAIRO_INTERNAL_SURFACE_TYPE_TEST_PAGINATED,
	CAIRO_CONTENT_COLOR_ALPHA, 0,
	"_cairo_test_paginated_surface_create",
	_cairo_boilerplate_test_paginated_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_test_paginated_get_image_surface,
	_cairo_boilerplate_test_paginated_surface_write_to_png,
	_cairo_boilerplate_test_paginated_cleanup,
	NULL, NULL, FALSE, TRUE, FALSE
    },
    {
	"test-paginated", "image", NULL, NULL,
	CAIRO_INTERNAL_SURFACE_TYPE_TEST_PAGINATED,
	CAIRO_CONTENT_COLOR, 0,
	"_cairo_test_paginated_surface_create",
	_cairo_boilerplate_test_paginated_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_test_paginated_get_image_surface,
	_cairo_boilerplate_test_paginated_surface_write_to_png,
	_cairo_boilerplate_test_paginated_cleanup,
	NULL, NULL, FALSE, TRUE, FALSE
    },
#endif
};
CAIRO_BOILERPLATE (test, targets)
