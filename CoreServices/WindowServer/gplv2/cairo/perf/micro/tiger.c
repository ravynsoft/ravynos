/*
 * Copyright © 2011 Intel Corporation
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
 * Author: Chris Wilson <chris@chris-wilson.o.uk>
 */

#include "cairo-perf.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "../../test/tiger.inc"

static cairo_time_t
do_tiger (cairo_t *cr, int width, int height, int loops)
{
    unsigned int i;

    cairo_perf_timer_start ();

    while (loops--) {
	cairo_identity_matrix (cr);

	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (cr, 0.1, 0.2, 0.3, 1.0);
	cairo_paint (cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	cairo_translate (cr, width/2, height/2);
	cairo_scale (cr, .85 * width/500, .85 * height/500);

	for (i = 0; i < sizeof (tiger_commands)/sizeof(tiger_commands[0]);i++) {
	    const struct command *cmd = &tiger_commands[i];
	    switch (cmd->type) {
	    case 'm':
		cairo_move_to (cr, cmd->x0, cmd->y0);
		break;
	    case 'l':
		cairo_line_to (cr, cmd->x0, cmd->y0);
		break;
	    case 'c':
		cairo_curve_to (cr,
				cmd->x0, cmd->y0,
				cmd->x1, cmd->y1,
				cmd->x2, cmd->y2);
		break;
	    case 'f':
		cairo_set_source_rgba (cr,
				       cmd->x0, cmd->y0, cmd->x1, cmd->y1);
		cairo_fill (cr);
		break;
	    }
	}
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mono_tiger (cairo_t *cr, int width, int height, int loops)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
    return do_tiger (cr, width, height, loops);
}

static cairo_time_t
do_fast_tiger (cairo_t *cr, int width, int height, int loops)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_FAST);
    return do_tiger (cr, width, height, loops);
}

static cairo_time_t
do_best_tiger (cairo_t *cr, int width, int height, int loops)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_BEST);
    return do_tiger (cr, width, height, loops);
}

cairo_bool_t
tiger_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "tiger", NULL);
}

void
tiger (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "tiger-mono", do_mono_tiger, NULL);
    cairo_perf_run (perf, "tiger-fast", do_fast_tiger, NULL);
    cairo_perf_run (perf, "tiger-best", do_best_tiger, NULL);
}
