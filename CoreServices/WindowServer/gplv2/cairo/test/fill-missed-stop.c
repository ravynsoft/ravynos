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
 *  The tessellator has a regression where a trapezoid may continue
 *  below the end of a polygon edge (i.e. the bottom of the trapezoid
 *  is miscomputed.)  This can only happen if the right edge of a
 *  trapezoid stops earlier than the left edge and there is no start
 *  event at the end point of the right edge.
 */

#include "cairo-test.h"

#define SIZE 50

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 0, 0);

    cairo_translate (cr, 1, 1);

    /* What it should look like, with # marking the filled areas:
     *
     * |\    |\
     * |#\   |#\
     * |##\__|##\
     *     \#|
     *      \|
     *
     * What it looke like with the bug, when the rightmost edge's end
     * is missed:
     *
     * |\    |\
     * |#\   |#\
     * |##\__|##\
     *     \#####|
     *      \####|
     */

    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, SIZE/2, SIZE);
    cairo_line_to (cr, SIZE/2, 0);
    cairo_line_to (cr, SIZE, SIZE/2);
    cairo_line_to (cr, 0, SIZE/2);
    cairo_close_path (cr);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (fill_missed_stop,
	    "Tests that the tessellator doesn't miss stop events when generating trapezoids",
	    "fill", /* keywords */
	    NULL, /* requirements */
	    SIZE+3, SIZE+3,
	    NULL, draw)
