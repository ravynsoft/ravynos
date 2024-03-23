/*
 * Copyright © 2005 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "cairo-test.h"

#define SIZE 10

static cairo_status_t
invalid_rel_move_to (cairo_surface_t *target)
{
    cairo_t *cr;
    cairo_status_t status;

    cr = cairo_create (target);
    cairo_rel_move_to (cr, SIZE, SIZE/2);
    status = cairo_status (cr);
    cairo_destroy (cr);

    return status;
}

static cairo_status_t
invalid_rel_line_to (cairo_surface_t *target)
{
    cairo_t *cr;
    cairo_status_t status;

    cr = cairo_create (target);
    cairo_rel_line_to (cr, -SIZE, SIZE/2);
    status = cairo_status (cr);
    cairo_destroy (cr);

    return status;
}

static cairo_status_t
invalid_rel_curve_to (cairo_surface_t *target)
{
    cairo_t *cr;
    cairo_status_t status;

    cr = cairo_create (target);
    cairo_rel_curve_to (cr,
			SIZE/2, -SIZE/2,
			SIZE*2/3, -SIZE/3,
			SIZE/2, -SIZE);
    status = cairo_status (cr);
    cairo_destroy (cr);

    return status;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_status_t status;
    cairo_test_status_t result;

    /* first test that a relative move without a current point fails... */
    status = invalid_rel_move_to (cairo_get_target (cr));
    if (status != CAIRO_STATUS_NO_CURRENT_POINT) {
	result = cairo_test_status_from_status (ctx, status);
	if (result == CAIRO_TEST_NO_MEMORY)
	    return result;

	cairo_test_log (ctx, "Error: invalid cairo_rel_move_to() did not raise NO_CURRENT_POINT\n");
	return result;
    }

    status = invalid_rel_line_to (cairo_get_target (cr));
    if (status != CAIRO_STATUS_NO_CURRENT_POINT) {
	result = cairo_test_status_from_status (ctx, status);
	if (result == CAIRO_TEST_NO_MEMORY)
	    return result;

	cairo_test_log (ctx, "Error: invalid cairo_rel_line_to() did not raise NO_CURRENT_POINT\n");
	return result;
    }

    status = invalid_rel_curve_to (cairo_get_target (cr));
    if (status != CAIRO_STATUS_NO_CURRENT_POINT) {
	result = cairo_test_status_from_status (ctx, status);
	if (result == CAIRO_TEST_NO_MEMORY)
	    return result;

	cairo_test_log (ctx, "Error: invalid cairo_rel_curve_to() did not raise NO_CURRENT_POINT\n");
	return result;
    }

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_move_to (cr, 0, 0);
    cairo_rel_move_to (cr, SIZE, SIZE/2);
    cairo_rel_line_to (cr, -SIZE, SIZE/2);
    cairo_rel_curve_to (cr,
			SIZE/2, -SIZE/2,
			SIZE*2/3, -SIZE/3,
			SIZE/2, -SIZE);

    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (rel_path,
	    "Tests calls to various relative path functions",
	    "path", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
