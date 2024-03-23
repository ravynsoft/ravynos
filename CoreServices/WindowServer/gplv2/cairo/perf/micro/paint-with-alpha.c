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
do_paint_with_alpha (cairo_t *cr, int width, int height, int loops)
{
    cairo_perf_timer_start ();

    while (loops--)
	cairo_paint_with_alpha (cr, 0.5);

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static double
count_paint_with_alpha (cairo_t *cr, int width, int height)
{
    return width * height / 1e6; /* Mpix/s */
}

cairo_bool_t
paint_with_alpha_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "paint-with-alpha", NULL);
}

void
paint_with_alpha (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_cover_sources_and_operators (perf, "paint-with-alpha",
					    do_paint_with_alpha,
					    count_paint_with_alpha);
}
