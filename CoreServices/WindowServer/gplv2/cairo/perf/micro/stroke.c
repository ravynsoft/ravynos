/*
 * Copyright © 2006 Red Hat, Inc.
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
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-perf.h"

static cairo_time_t
do_stroke (cairo_t *cr, int width, int height, int loops)
{
    cairo_arc (cr,
	       width/2.0, height/2.0,
	       width/3.0,
	       0, 2 * M_PI);
    cairo_close_path (cr);

    cairo_set_line_width (cr, width/5.0);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static void
rounded_rectangle (cairo_t *cr,
		   double x, double y, double w, double h,
		   double radius)
{
    cairo_move_to (cr, x+radius, y);
    cairo_arc (cr, x+w-radius, y+radius,   radius, M_PI + M_PI / 2, M_PI * 2        );
    cairo_arc (cr, x+w-radius, y+h-radius, radius, 0,               M_PI / 2        );
    cairo_arc (cr, x+radius,   y+h-radius, radius, M_PI/2,          M_PI            );
    cairo_arc (cr, x+radius,   y+radius,   radius, M_PI,            270 * M_PI / 180);
}

static cairo_time_t
do_strokes (cairo_t *cr, int width, int height, int loops)
{
    /* a pair of overlapping rectangles */
    rounded_rectangle (cr,
		       2, 2, width/2. + 10, height/2. + 10,
		       10);
    rounded_rectangle (cr,
		       width/2. - 10, height/2. - 10,
		       width/2. - 2, height/2. - 2,
		       10);

    cairo_set_line_width (cr, 2.);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_stroke_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
stroke_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "stroke", NULL);
}

void
stroke (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_cover_sources_and_operators (perf, "stroke", do_stroke, NULL);
    cairo_perf_cover_sources_and_operators (perf, "strokes", do_strokes, NULL);
}
