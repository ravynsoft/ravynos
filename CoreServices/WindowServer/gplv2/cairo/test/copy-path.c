/*
 * Copyright © 2005 Red Hat, Inc.
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

#include "cairo-test.h"
#include <stdlib.h>

static void
scale_by_two (double *x, double *y)
{
    *x = *x * 2.0;
    *y = *y * 2.0;
}

typedef void (*munge_func_t) (double *x, double *y);

static void
munge_and_set_path (cairo_t	 *cr,
		    cairo_path_t *path,
		    munge_func_t  munge)
{
    int i;
    cairo_path_data_t *p;
    double x1, y1, x2, y2, x3, y3;

    if (path->status) {
	cairo_append_path (cr, path);
	return;
    }

    for (i=0; i < path->num_data; i += path->data[i].header.length) {
	p = &path->data[i];
	switch (p->header.type) {
	case CAIRO_PATH_MOVE_TO:
	    x1 = p[1].point.x; y1 = p[1].point.y;
	    (munge) (&x1, &y1);
	    cairo_move_to (cr, x1, y1);
	    break;
	case CAIRO_PATH_LINE_TO:
	    x1 = p[1].point.x; y1 = p[1].point.y;
	    (munge) (&x1, &y1);
	    cairo_line_to (cr, x1, y1);
	    break;
	case CAIRO_PATH_CURVE_TO:
	    x1 = p[1].point.x; y1 = p[1].point.y;
	    x2 = p[2].point.x; y2 = p[2].point.y;
	    x3 = p[3].point.x; y3 = p[3].point.y;
	    (munge) (&x1, &y1);
	    (munge) (&x2, &y2);
	    (munge) (&x3, &y3);
	    cairo_curve_to (cr,
			    x1, y1,
			    x2, y2,
			    x3, y3);
	    break;
	case CAIRO_PATH_CLOSE_PATH:
	    cairo_close_path (cr);
	    break;
	}
    }
}

static void
make_path (cairo_t *cr)
{
    cairo_rectangle (cr, 0, 0, 5, 5);
    cairo_move_to (cr, 15, 2.5);
    cairo_arc (cr, 12.5, 2.5, 2.5, 0, 2 * M_PI);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_path_t *path;
    cairo_t *cr_error;

    /* Ensure that calling cairo_copy_path on an in-error cairo_t will
     * propagate the error. */
    cr_error = cairo_create (NULL);
    path = cairo_copy_path (cr_error);
    if (path->status != CAIRO_STATUS_NULL_POINTER) {
	cairo_test_log (ctx,
			"Error: cairo_copy_path returned status of %s rather than propagating %s\n",
			cairo_status_to_string (path->status),
			cairo_status_to_string (CAIRO_STATUS_NULL_POINTER));
	cairo_path_destroy (path);
	cairo_destroy (cr_error);
	return CAIRO_TEST_FAILURE;
    }
    cairo_path_destroy (path);

    path = cairo_copy_path_flat (cr_error);
    if (path->status != CAIRO_STATUS_NULL_POINTER) {
	cairo_test_log (ctx,
			"Error: cairo_copy_path_flat returned status of %s rather than propagating %s\n",
			cairo_status_to_string (path->status),
			cairo_status_to_string (CAIRO_STATUS_NULL_POINTER));
	cairo_path_destroy (path);
	cairo_destroy (cr_error);
	return CAIRO_TEST_FAILURE;
    }
    cairo_path_destroy (path);

    cairo_destroy (cr_error);

    /* first check that we can copy an empty path */
    cairo_new_path (cr);
    path = cairo_copy_path (cr);
    if (path->status != CAIRO_STATUS_SUCCESS) {
	cairo_status_t status = path->status;
	cairo_test_log (ctx,
			"Error: cairo_copy_path returned status of %s\n",
			cairo_status_to_string (status));
	cairo_path_destroy (path);
	return cairo_test_status_from_status (ctx, status);
    }
    if (path->num_data != 0) {
	cairo_test_log (ctx,
			"Error: cairo_copy_path did not copy an empty path, returned path contains %d elements\n",
		        path->num_data);
	cairo_path_destroy (path);
	return CAIRO_TEST_FAILURE;
    }
    cairo_append_path (cr, path);
    cairo_path_destroy (path);
    if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) {
	cairo_test_log (ctx,
			"Error: cairo_append_path failed with a copy of an empty path, returned status of %s\n",
			cairo_status_to_string (cairo_status (cr)));
	return cairo_test_status_from_status (ctx, cairo_status (cr));
    }

    /* We draw in the default black, so paint white first. */
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_restore (cr);

    /* copy path, munge, and fill */
    cairo_translate (cr, 5, 5);
    make_path (cr);
    path = cairo_copy_path (cr);

    cairo_new_path (cr);
    munge_and_set_path (cr, path, scale_by_two);
    cairo_path_destroy (path);
    cairo_fill (cr);

    /* copy flattened path, munge, and fill */
    cairo_translate (cr, 0, 15);
    make_path (cr);
    path = cairo_copy_path_flat (cr);

    cairo_new_path (cr);
    munge_and_set_path (cr, path, scale_by_two);
    cairo_path_destroy (path);
    cairo_fill (cr);

    /* append two copies of path, and fill */
    cairo_translate (cr, 0, 15);
    cairo_scale (cr, 2.0, 2.0);
    make_path (cr);
    path = cairo_copy_path (cr);

    cairo_new_path (cr);
    cairo_append_path (cr, path);
    cairo_translate (cr, 2.5, 2.5);
    cairo_append_path (cr, path);

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_fill (cr);

    cairo_path_destroy (path);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_t *cr;
    cairo_path_data_t data;
    cairo_path_t path;
    cairo_surface_t *surface;
    cairo_status_t status;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
    status = cairo_surface_status (surface);
    if (status) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    /* Test a few error cases for cairo_append_path_data */
#define CAIRO_CREATE() do {\
    cr = cairo_create (surface); \
    status = cairo_status (cr); \
    if (status) { \
	cairo_destroy (cr); \
	cairo_surface_destroy (surface); \
	return cairo_test_status_from_status (ctx, status); \
    } \
} while (0)
    CAIRO_CREATE ();
    cairo_append_path (cr, NULL);
    status = cairo_status (cr);
    cairo_destroy (cr);
    if (status != CAIRO_STATUS_NULL_POINTER) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    CAIRO_CREATE ();
    path.status = -1;
    cairo_append_path (cr, &path);
    status = cairo_status (cr);
    cairo_destroy (cr);
    if (status != CAIRO_STATUS_INVALID_STATUS) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    CAIRO_CREATE ();
    path.status = CAIRO_STATUS_NO_MEMORY;
    cairo_append_path (cr, &path);
    status = cairo_status (cr);
    cairo_destroy (cr);
    if (status != CAIRO_STATUS_NO_MEMORY) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    CAIRO_CREATE ();
    path.data = NULL;
    path.num_data = 0;
    path.status = CAIRO_STATUS_SUCCESS;
    cairo_append_path (cr, &path);
    status = cairo_status (cr);
    cairo_destroy (cr);
    if (status != CAIRO_STATUS_SUCCESS) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    CAIRO_CREATE ();
    path.data = NULL;
    path.num_data = 1;
    path.status = CAIRO_STATUS_SUCCESS;
    cairo_append_path (cr, &path);
    status = cairo_status (cr);
    cairo_destroy (cr);
    if (status != CAIRO_STATUS_NULL_POINTER) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    CAIRO_CREATE ();
    /* Intentionally insert bogus header.length value (otherwise would be 2) */
    data.header.type = CAIRO_PATH_MOVE_TO;
    data.header.length = 1;
    path.data = &data;
    path.num_data = 1;
    cairo_append_path (cr, &path);
    status = cairo_status (cr);
    cairo_destroy (cr);
    if (status != CAIRO_STATUS_INVALID_PATH_DATA) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    /* And test the degenerate case */
    CAIRO_CREATE ();
    path.num_data = 0;
    cairo_append_path (cr, &path);
    status = cairo_status (cr);
    cairo_destroy (cr);
    if (status != CAIRO_STATUS_SUCCESS) {
	cairo_surface_destroy (surface);
	return cairo_test_status_from_status (ctx, status);
    }

    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (copy_path,
	    "Tests calls to path_data functions: cairo_copy_path, cairo_copy_path_flat, and cairo_append_path",
	    "path", /* keywords */
	    NULL, /* requirements */
	    45, 53,
	    preamble, draw)
