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
 *
 * Author: Krzysztof Kosi\u0144ski <tweenk.pl@gmail.com>
 */

/* Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=40918 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *radial;
    double angle;
    int i, j;

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_paint (cr);

    angle = 0.0;

    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++) {
	    cairo_save (cr);
	    cairo_rectangle (cr, 100*i, 100*j, 100, 100);
	    cairo_clip (cr);

	    radial = cairo_pattern_create_radial (cos (angle), sin (angle), 0,
						  0, 0, 1);
	    cairo_pattern_add_color_stop_rgb (radial, 0.0, 1, 0, 0);
	    cairo_pattern_add_color_stop_rgb (radial, 1.0, 0, 1, 0);

	    cairo_translate (cr, 100*i+50, 100*j+50);
	    cairo_scale (cr, 50, -50);
	    cairo_set_source (cr, radial);
	    cairo_pattern_destroy (radial);

	    cairo_paint(cr);
	    cairo_restore (cr);

	    angle += M_PI/17;
	}
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (radial_outer_focus,
	    "Exercises the condition of rendering a radial gradial on its outer focus",
	    "radial", /* keywords */
	    NULL, /* requirements */
	    400, 400,
	    NULL, draw)
