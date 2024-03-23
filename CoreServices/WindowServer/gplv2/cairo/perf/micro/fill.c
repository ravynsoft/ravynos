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
do_fill (cairo_t *cr, int width, int height, int loops)
{
    cairo_arc (cr,
	       width/2.0, height/2.0,
	       width/3.0,
	       0, 2 * M_PI);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_fill_annuli (cairo_t *cr, int width, int height, int loops)
{
    cairo_new_sub_path (cr);
    cairo_arc (cr,
	       width/2.0, height/2.0,
	       width/3.0,
	       0, 2 * M_PI);

    cairo_new_sub_path (cr);
    cairo_arc_negative (cr,
	       width/2.0, height/2.0,
	       width/4.0,
	       2 * M_PI, 0);

    cairo_new_sub_path (cr);
    cairo_arc (cr,
	       width/2.0, height/2.0,
	       width/6.0,
	       0, 2 * M_PI);

    cairo_new_sub_path (cr);
    cairo_arc_negative (cr,
	       width/2.0, height/2.0,
	       width/8.0,
	       2 * M_PI, 0);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_fill_eo_noaa (cairo_t *cr, int width, int height, int loops)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

    cairo_arc (cr,
	       width/2.0, height/2.0,
	       width/3.0,
	       0, 2 * M_PI);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
fill_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "fill", NULL);
}

void
fill (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_cover_sources_and_operators (perf, "fill", do_fill, NULL);
    cairo_perf_cover_sources_and_operators (perf, "fill-annuli", do_fill_annuli, NULL);
    cairo_perf_cover_sources_and_operators (perf, "fill-eo-noaa", do_fill_eo_noaa, NULL);
}
