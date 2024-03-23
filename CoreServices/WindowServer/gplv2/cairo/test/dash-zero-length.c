/*
 * Copyright © 2006 Jeff Muizelaar
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Jeff Muizelaar. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Jeff Muizelaar. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * JEFF MUIZELAAR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL JEFF MUIZELAAR BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Jeff Muizelaar <jeff@infidigm.net>
 */

#include "cairo-test.h"

#define IMAGE_WIDTH 19
#define IMAGE_HEIGHT 61

/* A test of the two extremes of dashing: a solid line
 * and an invisible one. Also test that capping works
 * on invisible lines.
 */

static void
draw_dash (cairo_t *cr, double *dash, int num_dashes)
{
    cairo_set_dash (cr, dash, num_dashes, 0.0);
    cairo_move_to (cr,  1, 2);
    cairo_line_to (cr, 18, 2);
    cairo_stroke (cr);
    cairo_translate (cr, 0, 3);
}
static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    static double solid_line[] = { 4, 0 };
    static double invisible_line[] = { 0, 4 };
    static double dotted_line[] = { 0, 6 };
    static double zero_1_of_3[] = { 0, 2, 3 };
    static double zero_2_of_3[] = { 1, 0, 3 };
    static double zero_3_of_3[] = { 1, 2, 0 };
    static double zero_1_of_4[] = { 0, 2, 3, 4 };
    static double zero_2_of_4[] = { 1, 0, 3, 4 };
    static double zero_3_of_4[] = { 1, 2, 0, 4 };
    static double zero_4_of_4[] = { 1, 2, 3, 0 };
    static double zero_1_2_of_4[] = { 0, 0, 3, 4 };
    static double zero_1_3_of_4[] = { 0, 2, 0, 4 };
/* Clearly it would be nice to draw this one as well, but it seems to trigger a bug in ghostscript. */
#if BUG_FIXED_IN_GHOSTSCRIPT
    static double zero_1_4_of_4[] = { 0, 2, 3, 0 };
#endif
    static double zero_2_3_of_4[] = { 1, 0, 0, 4 };
    static double zero_2_4_of_4[] = { 1, 0, 3, 0 };
    static double zero_3_4_of_4[] = { 1, 2, 0, 0 };
    static double zero_1_2_3_of_4[] = { 0, 0, 0, 4 };
    static double zero_1_2_4_of_4[] = { 0, 0, 3, 0 };
    static double zero_1_3_4_of_4[] = { 0, 2, 0, 0 };
    static double zero_2_3_4_of_4[] = { 1, 0, 0, 0 };

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_set_line_width (cr, 2);

    draw_dash (cr, solid_line, 2);
    draw_dash (cr, invisible_line, 2);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    draw_dash (cr, dotted_line, 2);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);

    draw_dash (cr, zero_1_of_3, 3);
    draw_dash (cr, zero_2_of_3, 3);
    draw_dash (cr, zero_3_of_3, 3);
    draw_dash (cr, zero_1_of_4, 4);
    draw_dash (cr, zero_2_of_4, 4);
    draw_dash (cr, zero_3_of_4, 4);
    draw_dash (cr, zero_4_of_4, 4);
    draw_dash (cr, zero_1_2_of_4, 4);
    draw_dash (cr, zero_1_3_of_4, 4);
#if BUG_FIXED_IN_GHOSTSCRIPT
    draw_dash (cr, zero_1_4_of_4, 4);
#endif
    draw_dash (cr, zero_2_3_of_4, 4);
    draw_dash (cr, zero_2_4_of_4, 4);
    draw_dash (cr, zero_3_4_of_4, 4);
    draw_dash (cr, zero_1_2_3_of_4, 4);
    draw_dash (cr, zero_1_2_4_of_4, 4);
    draw_dash (cr, zero_1_3_4_of_4, 4);
    draw_dash (cr, zero_2_3_4_of_4, 4);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (dash_zero_length,
	    "Tests cairo_set_dash with zero length",
	    "dash, stroke", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
