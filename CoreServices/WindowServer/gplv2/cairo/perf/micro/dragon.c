/*
 * Copyright © 2007 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 *
 * Inspiration (and path!) taken from
 * http://labs.trolltech.com/blogs/2007/08/31/rasterizing-dragons/
 */

#include "cairo-perf.h"

static inline int
next_pot (int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

static cairo_bool_t
direction (int i)
{
    int pivot, np2;

    if (i < 2)
	return TRUE;

    np2 = next_pot (i + 1);
    if (np2 == i + 1)
	return TRUE;

    pivot = np2 / 2 - 1;
    return ! direction (2 * pivot - i);
}

static void
path (cairo_t *cr, int step, int dir, int iterations)
{
    double dx, dy;
    int i;

    switch (dir) {
	default:
	case 0: dx =  step; dy =  0; break;
	case 1: dx = -step; dy =  0; break;
	case 2: dx =  0; dy =  step; break;
	case 3: dx =  0; dy = -step; break;
    }

    for (i = 0; i < iterations; i++) {
	cairo_rel_line_to (cr, dx, dy);

	if (direction (i)) {
	    double t = dx;
	    dx = dy;
	    dy = -t;
	} else {
	    double t = dx;
	    dx = -dy;
	    dy = t;
	}
    }
}

static cairo_time_t
do_dragon (cairo_t *cr, int width, int height, int loops)
{
    cairo_pattern_t *pattern;
    double cx, cy, r;

    cx = cy = .5 * MAX (width, height);
    r = .5 * MIN (width, height);

    cairo_perf_timer_start ();

    while (loops--) {
	pattern = cairo_pattern_create_radial (cx, cy, 0., cx, cy, r);
	cairo_pattern_add_color_stop_rgb (pattern, 0., .0, .0, .0);
	cairo_pattern_add_color_stop_rgb (pattern, 0.25, .5, .4, .4);
	cairo_pattern_add_color_stop_rgb (pattern, .5, .8, .8, .9);
	cairo_pattern_add_color_stop_rgb (pattern, 1., .9, .9, 1.);
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	cairo_paint (cr);

	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width (cr, 4.);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 0, 2048);
	pattern = cairo_pattern_create_radial (cx, cy, 0., cx, cy, r);
	cairo_pattern_add_color_stop_rgb (pattern, 0., 1., 1., 1.);
	cairo_pattern_add_color_stop_rgb (pattern, 1., 0., 0., 0.);
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	cairo_stroke(cr);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 1, 2048);
	pattern = cairo_pattern_create_radial (cx, cy, 0., cx, cy, r);
	cairo_pattern_add_color_stop_rgb (pattern, 1., 1., 1., 0.);
	cairo_pattern_add_color_stop_rgb (pattern, 0., 1., 0., 0.);
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	cairo_stroke(cr);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 2, 2048);
	pattern = cairo_pattern_create_radial (cx, cy, 0., cx, cy, r);
	cairo_pattern_add_color_stop_rgb (pattern, 1., 0., 1., 1.);
	cairo_pattern_add_color_stop_rgb (pattern, 0., 0., 1., 0.);
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	cairo_stroke(cr);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 3, 2048);
	pattern = cairo_pattern_create_radial (cx, cy, 0., cx, cy, r);
	cairo_pattern_add_color_stop_rgb (pattern, 1., 1., 0., 1.);
	cairo_pattern_add_color_stop_rgb (pattern, 0., 0., 0., 1.);
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	cairo_stroke(cr);
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_dragon_solid (cairo_t *cr, int width, int height, int loops)
{
    double cx, cy;

    cx = cy = .5 * MAX (width, height);

    cairo_perf_timer_start ();

    while (loops--) {
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_paint (cr);

	cairo_set_line_width (cr, 4.);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 0, 2048);
	cairo_set_source_rgb (cr, 1, 0, 0);
	cairo_stroke(cr);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 1, 2048);
	cairo_set_source_rgb (cr, 0, 1, 0);
	cairo_stroke(cr);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 2, 2048);
	cairo_set_source_rgb (cr, 0, 0, 1);
	cairo_stroke(cr);

	cairo_move_to (cr, cx, cy);
	path (cr, 12, 3, 2048);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_stroke(cr);
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_dragon_solid_unaligned (cairo_t *cr, int width, int height, int loops)
{
    cairo_translate (cr, 0.01, 0.01);
    return do_dragon_solid (cr, width, height, loops);
}

static cairo_time_t
do_dragon_solid_aligned_clip (cairo_t *cr, int width, int height, int loops)
{
    cairo_reset_clip (cr);
    cairo_rectangle (cr, 10, 10, width/2 + 10, height/2 + 10);
    cairo_rectangle (cr, width/2-20, height/2-20, width/2 + 10, height/2 + 10);
    cairo_clip (cr);

    return do_dragon_solid (cr, width, height, loops);
}

static cairo_time_t
do_dragon_unaligned_solid_aligned_clip (cairo_t *cr, int width, int height, int loops)
{
    cairo_translate (cr, 0.01, 0.01);
    cairo_reset_clip (cr);
    cairo_rectangle (cr, 10, 10, width/2 + 10, height/2 + 10);
    cairo_rectangle (cr, width/2-20, height/2-20, width/2 + 10, height/2 + 10);
    cairo_clip (cr);

    return do_dragon_solid (cr, width, height, loops);
}

static cairo_time_t
do_dragon_solid_unaligned_clip (cairo_t *cr, int width, int height, int loops)
{
    cairo_reset_clip (cr);
    cairo_rectangle (cr, 10.5, 10.5, width/2 + 10, height/2 + 10);
    cairo_rectangle (cr, width/2-20, height/2-20, width/2 + 9.5, height/2 + 9.5);
    cairo_clip (cr);

    return do_dragon_solid (cr, width, height, loops);
}

static cairo_time_t
do_dragon_unaligned_solid_unaligned_clip (cairo_t *cr, int width, int height, int loops)
{
    cairo_translate (cr, 0.01, 0.01);
    cairo_reset_clip (cr);
    cairo_rectangle (cr, 10.5, 10.5, width/2 + 10, height/2 + 10);
    cairo_rectangle (cr, width/2-20, height/2-20, width/2 + 9.5, height/2 + 9.5);
    cairo_clip (cr);

    return do_dragon_solid (cr, width, height, loops);
}

static cairo_time_t
do_dragon_solid_circle_clip (cairo_t *cr, int width, int height, int loops)
{
    cairo_reset_clip (cr);
    cairo_arc (cr, width/2., height/2., MIN (width, height)/2. - 10, 0, 2 * M_PI);
    cairo_clip (cr);

    return do_dragon_solid (cr, width, height, loops);
}

cairo_bool_t
dragon_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "dragon", NULL);
}

void
dragon (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "dragon-solid", do_dragon_solid, NULL);
    cairo_perf_run (perf, "dragon-unaligned-solid", do_dragon_solid_unaligned, NULL);
    cairo_perf_run (perf, "dragon-solid-aligned-clip", do_dragon_solid_aligned_clip, NULL);
    cairo_perf_run (perf, "dragon-unaligned-solid-aligned-clip", do_dragon_unaligned_solid_aligned_clip, NULL);
    cairo_perf_run (perf, "dragon-solid-unaligned-clip", do_dragon_solid_unaligned_clip, NULL);
    cairo_perf_run (perf, "dragon-unaligned-solid-unaligned-clip", do_dragon_unaligned_solid_unaligned_clip, NULL);
    cairo_perf_run (perf, "dragon-solid-circle-clip", do_dragon_solid_circle_clip, NULL);
    cairo_perf_run (perf, "dragon", do_dragon, NULL);
}
