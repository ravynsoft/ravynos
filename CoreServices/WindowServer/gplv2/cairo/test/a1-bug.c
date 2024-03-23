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

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    static const struct point {
	double x;
	double y;
    } xy[] = {
	{ 627.016212, 221.749777 },
	{ 756.120787, 221.749777 },
	{ 756.120787, 557.602766 },
	{ 626.952721, 557.602766 },
	{ 626.548456, 493.315729 },
    };
    unsigned int i;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    for (i = 0; i < ARRAY_LENGTH (xy); i++)
	cairo_line_to (cr, xy[i].x, xy[i].y);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill_preserve (cr);

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (a1_bug,
	    "Check the fidelity of the rasterisation.",
	    "a1, raster", /* keywords */
	    "target=raster", /* requirements */
	    1000, 800,
	    NULL, draw)
