/*
 * Copyright © 2016 Adrian Johnson
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *	Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"

/* Check cairo_recording_surface_ink_extents() returns correct extents. */


static cairo_test_status_t
check_extents (cairo_test_context_t *cr,
	       cairo_surface_t *recording_surface,
	       const char * func_name,
	       double expected_x, double expected_y, double expected_w, double expected_h)
{
    double x, y, w, h;
    cairo_recording_surface_ink_extents (recording_surface, &x, &y, &w, &h);
    if (x != expected_x ||
	y != expected_y ||
	w != expected_w ||
	h != expected_h)
    {
	cairo_test_log (cr,
			"%s: x: %f, y: %f, w: %f, h: %f\n"
			"    expected: x: %f, y: %f, w: %f, h: %f\n",
			func_name,
			x, y, w, h,
			expected_x, expected_y,
			expected_w, expected_h);
       return CAIRO_TEST_ERROR;
    }
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
unbounded_fill (cairo_test_context_t *test_cr)
{
    cairo_test_status_t status;
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
    cr = cairo_create (surface);

    cairo_rectangle (cr, -300, -150, 900, 600);
    cairo_fill (cr);

    cairo_destroy(cr);

    status = check_extents (test_cr, surface,  __func__,
			    -300, -150, 900, 600);
    cairo_surface_destroy (surface);
    return status;
}

static cairo_test_status_t
bounded_fill (cairo_test_context_t *test_cr)
{
    cairo_test_status_t status;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_rectangle_t extents = { -150, -100, 300, 200 };

    surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &extents);
    cr = cairo_create (surface);

    cairo_rectangle (cr, -300, -300, 650, 600);
    cairo_fill (cr);

    cairo_destroy(cr);

    status = check_extents (test_cr, surface,  __func__,
			    -150, -100, 300, 200);
    cairo_surface_destroy (surface);
    return status;
}

static cairo_test_status_t
unbounded_paint (cairo_test_context_t *test_cr)
{
    cairo_test_status_t status;
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
    cr = cairo_create (surface);

    cairo_paint (cr);

    cairo_destroy(cr);

    status = check_extents (test_cr, surface,  __func__,
			    -(1 << 23), -(1 << 23), -1, -1);
    cairo_surface_destroy (surface);
    return status;
}

static cairo_test_status_t
bounded_paint (cairo_test_context_t *test_cr)
{
    cairo_test_status_t status;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_rectangle_t extents = { -150, -100, 300, 200 };

    surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &extents);
    cr = cairo_create (surface);

    cairo_paint (cr);

    cairo_destroy(cr);

    status = check_extents (test_cr, surface,  __func__,
			    -150, -100, 300, 200);
    cairo_surface_destroy (surface);
    return status;
}

static cairo_test_status_t
preamble (cairo_test_context_t *cr)
{
    cairo_test_status_t status;

    status = unbounded_fill (cr);
    if (status != CAIRO_TEST_SUCCESS)
	return status;

    status = bounded_fill (cr);
    if (status != CAIRO_TEST_SUCCESS)
	return status;

    status = unbounded_paint (cr);
    if (status != CAIRO_TEST_SUCCESS)
	return status;

    status = bounded_paint (cr);
    if (status != CAIRO_TEST_SUCCESS)
	return status;

    return CAIRO_TEST_SUCCESS;
}


CAIRO_TEST (recording_ink_extents,
	    "Test cairo_recording_surface_ink_extents()",
	    "api,recording,extents", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
