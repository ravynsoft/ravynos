/*
 * Copyright © 2006 M Joonas Pihlaja
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
 * Author: M Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
 */

/* Bug history
 *
 * 2006-12-05  M Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
 *
 *  The cairo_in_fill () function can sometimes produce false
 *  positives when the tessellator produces empty trapezoids
 *  and the query point lands exactly on a trapezoid edge.
 */

#include "cairo-test.h"

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    int x,y;
    int width = 10;
    int height = 10;
    cairo_surface_t *surf;
    cairo_t *cr;
    int false_positive_count = 0;
    cairo_status_t status;
    cairo_test_status_t ret;

    surf = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surf);
    cairo_surface_destroy (surf);

    /* Empty horizontal trapezoid. */
    cairo_move_to (cr, 0, height/3);
    cairo_line_to (cr, width, height/3);
    cairo_close_path (cr);

    /* Empty non-horizontal trapezoid #1. */
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, width, height/2);
    cairo_close_path (cr);

    /* Empty non-horizontal trapezoid #2 intersecting #1. */
    cairo_move_to (cr, 0, height/2);
    cairo_line_to (cr, width, 0);
    cairo_close_path (cr);

    status = cairo_status (cr);

    /* Point sample the tessellated path. */
    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    if (cairo_in_fill (cr, x, y)) {
		false_positive_count++;
	    }
	}
    }
    cairo_destroy (cr);

    /* Check that everything went well. */
    ret = CAIRO_TEST_SUCCESS;
    if (CAIRO_STATUS_SUCCESS != status) {
	cairo_test_log (ctx, "Failed to create a test surface and path: %s\n",
			cairo_status_to_string (status));
	ret = CAIRO_TEST_XFAILURE;
    }

    if (0 != false_positive_count) {
	cairo_test_log (ctx, "Point sampling found %d false positives "
			"from cairo_in_fill()\n",
			false_positive_count);
	ret = CAIRO_TEST_XFAILURE;
    }

    return ret;
}

/*
 * XFAIL: The cairo_in_fill () function can sometimes produce false positives
 * when the tessellator produces empty trapezoids and the query point lands
 * exactly on a trapezoid edge.
 */
CAIRO_TEST (in_fill_empty_trapezoid,
	    "Test that the tessellator isn't producing obviously empty trapezoids",
	    "in, trap", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
