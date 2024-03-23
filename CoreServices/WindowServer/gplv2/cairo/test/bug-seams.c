/*
 * Copyright 2010 Soeren Sandmann Pedersen
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
 * Author: Soeren Sandmann <sandmann@daimi.au.dk>
 */

/* Exercises a case of seam appearing between two polygons in the image
 * backend but not in xlib [using pixman].
 *
 * The test case draws two abutting quads both individually on the
 * leftm combining them with operator ADD, and in one go on the right.
 * Both methods should show no signs of seaming at the common edge
 * between the two quads, but the individually drawn ones have a
 * slight seam.
 *
 * The cause of the seam is that there are slight differences in the
 * output of the analytical coverage rasterization and the
 * supersampling rasterization methods, both employed by
 * cairo-tor-scan-converter.  When drawn individually, the scan
 * converter gets a partial view of the geometry at a time, so it ends
 * up making different decisions about which scanlines it rasterizes
 * with which method, compared to when the geometry is draw all at
 * once.  Though both methods produce seamless results individually
 * (where applicable), they don't produce bit-exact identical results,
 * and hence we get seaming where they meet.
 */

#include "cairo-test.h"

static void
draw_quad (cairo_t *cr,
	   double x1, double y1, double x2, double y2,
	   double x3, double y3, double x4, double y4)
{
    cairo_move_to (cr, x1, y1);
    cairo_line_to (cr, x2, y2);
    cairo_line_to (cr, x3, y3);
    cairo_line_to (cr, x4, y4);
    cairo_close_path (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_scale (cr, 20, 20);
    cairo_translate (cr, 5, 1);

    /* On the left side, we have two quads drawn one at a time and
     * combined with OPERATOR_ADD.  This should be seamless, but
     * isn't. */
    cairo_push_group (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);

    cairo_set_source_rgb (cr, 0, 0.6, 0);
    draw_quad (cr,
	       1.50, 1.50,
	       2.64, 1.63,
	       1.75, 2.75,
	       0.55, 2.63);
    cairo_fill (cr);
    draw_quad (cr,
	       0.55, 2.63,
	       1.75, 2.75,
	       0.98, 4.11,
	       -0.35, 4.05);
    cairo_fill (cr);

    cairo_pop_group_to_source (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_paint (cr);

    /* On the right side, we have the same two quads drawn both at the
     * same time.  This is seamless. */
    cairo_translate (cr, 10, 0);

    cairo_set_source_rgb (cr, 0, 0.6, 0);
    draw_quad (cr,
	       1.50, 1.50,
	       2.64, 1.63,
	       1.75, 2.75,
	       0.55, 2.63);
    draw_quad (cr,
	       0.55, 2.63,
	       1.75, 2.75,
	       0.98, 4.11,
	       -0.35, 4.05);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_seams,
	    "Check the fidelity of the rasterisation.",
	    "raster", /* keywords */
	    "target=raster", /* requirements */
	    500, 300,
	    NULL, draw)
