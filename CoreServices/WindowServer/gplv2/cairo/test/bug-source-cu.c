/*
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
 */

#include "cairo-test.h"

static cairo_pattern_t *
create_pattern (cairo_surface_t *target)
{
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    cairo_t *cr;
    cairo_matrix_t m;

    surface = cairo_surface_create_similar(target,
					   cairo_surface_get_content (target),
					   1000, 600);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_paint(cr);

    pattern = cairo_pattern_create_for_surface (cairo_get_target (cr));
    cairo_destroy(cr);

    cairo_matrix_init_translate (&m, 0, 0.1); // y offset must be non-integer
    cairo_pattern_set_matrix (pattern, &m);
    return pattern;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    cairo_new_path (cr);
    cairo_move_to (cr, 10, 400.1);
    cairo_line_to (cr, 990, 400.1);
    cairo_line_to (cr, 990, 600);
    cairo_line_to (cr, 10,  600);
    cairo_close_path (cr);

    pattern = create_pattern (cairo_get_target (cr));
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);

    cairo_fill(cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_source_cu,
	    "Exercises a bug discovered in the tracking of unbounded source extents",
	    "fill", /* keywords */
	    NULL, /* requirements */
	    1000, 600,
	    NULL, draw)
