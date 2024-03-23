/*
 * Author: Christian Rohlfs, 2022
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
 */

#include "cairo-test.h"

#define WIDTH   100
#define HEIGHT  WIDTH

static cairo_test_status_t
draw_round(cairo_t *cr, int width, int height)
{
    /* Fail condition:
            0  >=  2 * ( 1 - tolerance / (line_width / 2) )^2 - 1,
       with the default tolerance of 0.1, `failing_line_width` is in
            [0.117157287525381; 0.682842712474619]
       range.
    */

    double failing_line_width = 0.3;

    cairo_set_tolerance(cr, 0.1);
    cairo_scale(cr, width, height);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_move_to(cr, 0.2, 0.2);
    cairo_line_to(cr, 0.8, 0.2);
    cairo_line_to(cr, 0.8, 0.8);
    cairo_line_to(cr, 0.2, 0.8);
    cairo_close_path(cr);

    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, failing_line_width);
    cairo_stroke(cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_bevel(cairo_t *cr, int width, int height)
{
    /* Fail condition:
            arc_height < tolerance
       For 90° join `arc_height` is equal to
            line_width / 2 * (1 - sqrt(1/2))
       which is 0.1464466094067262 of a `line_width`.
    */

    double line_width = 0.3;
    double failing_tolerance = 0.3 * line_width;

    cairo_set_tolerance(cr, width * failing_tolerance);
    cairo_scale(cr, width, height);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_move_to(cr, 0.2, 0.2);
    cairo_line_to(cr, 0.8, 0.2);
    cairo_line_to(cr, 0.8, 0.8);
    cairo_line_to(cr, 0.2, 0.8);
    cairo_close_path(cr);

    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, line_width);
    cairo_stroke(cr);

    return CAIRO_TEST_SUCCESS;
}


CAIRO_TEST (round_join_bug_520_round,
    "Tests round joins within tolerance",
    "stroke, cap, join", /* keywords */
    NULL, /* requirements */
    WIDTH, HEIGHT,
    NULL, draw_round)

CAIRO_TEST (round_join_bug_520_bevel,
    "Tests round joins omission when `arc_height < tolerance`",
    "stroke, cap, join", /* keywords */
    NULL, /* requirements */
    WIDTH, HEIGHT,
    NULL, draw_bevel)

