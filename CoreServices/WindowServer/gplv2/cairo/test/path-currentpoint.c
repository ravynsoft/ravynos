/*
 * Copyright © 2014 Google, Inc.
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
 */

#include "cairo-test.h"

#include <assert.h>

static void
assert_point (cairo_t *cr, double expected_x, double expected_y) {
  double x, y;
  assert (cairo_has_current_point (cr));
  cairo_get_current_point (cr, &x, &y);
  assert (x == expected_x);
  assert (y == expected_y);
}

static void
assert_point_maintained (cairo_t *cr, double expected_x, double expected_y) {
  cairo_path_t *path;

  assert_point (cr, expected_x, expected_y);

  path = cairo_copy_path (cr);

  cairo_new_path (cr);
  cairo_rectangle (cr, 5, 5, 10, 20);
  cairo_stroke (cr);

  cairo_new_path (cr);
  cairo_append_path (cr, path);
  cairo_path_destroy (path);

  assert_point (cr, expected_x, expected_y);
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 20, 20);
    cr = cairo_create (surface);

    cairo_new_path (cr);
    cairo_move_to (cr, 1., 2.);
    assert_point_maintained (cr, 1., 2.);

    cairo_line_to (cr, 4., 5.);
    cairo_move_to (cr, 2., 1.);
    assert_point_maintained (cr, 2., 1.);

    cairo_move_to (cr, 5, 5);
    cairo_arc (cr, 5, 5, 10, 0, M_PI / 3);
    cairo_close_path (cr);
    assert_point_maintained (cr, 5, 5);

    cairo_destroy (cr);
    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (path_currentpoint,
	    "Test save/restore path maintains current point",
	    "api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
