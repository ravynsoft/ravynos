/*
 * Copyright © 2006 Dan Amelang
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * the authors not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The authors make no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dan Amelang <dan@amelang.net>
 *
 * This test was originally created to test _cairo_fixed_from_double.
 * cairo_pattern_create_radial was selected as the entry point into
 * cairo as it makes several calls to _cairo_fixed_from_double and
 * presents a somewhat realistic use-case (although the RADIALS_COUNT
 * isn't very realistic).
 */
#include "cairo-perf.h"
#include <time.h>

#define RADIALS_COUNT (10000)

static struct
{
    double cx0;
    double cy0;
    double radius0;
    double cx1;
    double cy1;
    double radius1;
} radials[RADIALS_COUNT];

static double
generate_double_in_range (double min, double max)
{
    double d;

    d = rand () / (double) RAND_MAX;
    d *= max - min;
    d += min;

    return d;
}

static cairo_time_t
do_pattern_create_radial (cairo_t *cr, int width, int height, int loops)
{
    cairo_perf_timer_start ();

    while (loops--) {
	cairo_pattern_t *pattern;
	int i;

	for (i = 0; i < RADIALS_COUNT; i++) {
	    pattern =
		cairo_pattern_create_radial (radials[i].cx0, radials[i].cy0,
					     radials[i].radius0,
					     radials[i].cx1, radials[i].cy1,
					     radials[i].radius1);
	    cairo_pattern_destroy (pattern);
	}
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
pattern_create_radial_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "pattern-create-radial", NULL);
}

void
pattern_create_radial (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    int i;

    srand (time (0));
    for (i = 0; i < RADIALS_COUNT; i++)
    {
        radials[i].cx0 = generate_double_in_range (-50000.0, 50000.0);
        radials[i].cy0 = generate_double_in_range (-50000.0, 50000.0);
        radials[i].radius0 = generate_double_in_range (0.0, 1000.0);
        radials[i].cx1 = generate_double_in_range (-50000.0, 50000.0);
        radials[i].cy1 = generate_double_in_range (-50000.0, 50000.0);
        radials[i].radius1 = generate_double_in_range (0.0, 1000.0);
    }

    cairo_perf_run (perf, "pattern-create-radial",
                          do_pattern_create_radial, NULL);
}
