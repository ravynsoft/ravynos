/*
 * Copyright © 2011 Krzysztof Kosiński
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
 * Author: Krzysztof Kosiński <tweenk.pl@gmail.com>
 */

#include "cairo-test.h"

#define WIDTH	800
#define HEIGHT	800

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double lw = 800;
    int n = 0;

    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);

    cairo_set_source_rgba (cr, 0, 0, 0, .4);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
    do {
	cairo_set_line_width(cr, lw);
	cairo_arc(cr, WIDTH/2, HEIGHT/2, lw/2,
		  2*M_PI*(13*n + 1) / 130, 2*M_PI*(13*n + 12) / 130);
	cairo_stroke(cr);

	n++;
	lw /= 1.1;
    } while (lw > 0.5);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_84115,
	    "Exercises a bug found in stroke generation using trapezoids",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
