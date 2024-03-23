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
 */

#include "cairo-perf.h"
#define _USE_MATH_DEFINES /* for M_SQRT2 on win32 */
#include <math.h>

static void
add_rectangle (cairo_t *cr, double size)
{
    double x, y;

    if (size < 1)
	return;

    cairo_get_current_point (cr, &x, &y);

    cairo_rel_move_to (cr, -size/2., -size/2.);
    cairo_rel_line_to (cr, size, 0);
    cairo_rel_line_to (cr, 0, size);
    cairo_rel_line_to (cr, -size, 0);
    cairo_close_path (cr);

    cairo_save (cr);
    cairo_translate (cr, -size/2., size);
    cairo_move_to (cr, x, y);
    cairo_rotate (cr, M_PI/4);
    add_rectangle (cr, size / M_SQRT2);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_translate (cr, size/2., size);
    cairo_move_to (cr, x, y);
    cairo_rotate (cr, -M_PI/4);
    add_rectangle (cr, size / M_SQRT2);
    cairo_restore (cr);
}

static cairo_time_t
do_pythagoras_tree (cairo_t *cr, int width, int height, int loops)
{
    double size = 128;

    cairo_perf_timer_start ();

    while (loops--) {
	cairo_save (cr);
	cairo_translate (cr, 0, height);
	cairo_scale (cr, 1, -1);

	cairo_move_to (cr, width/2, size/2);
	add_rectangle (cr, size);
	cairo_set_source_rgb (cr, 0., 0., 0.);
	cairo_fill (cr);
	cairo_restore (cr);
    }

    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
pythagoras_tree_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "pythagoras-tree", NULL);
}

void
pythagoras_tree (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "pythagoras-tree", do_pythagoras_tree, NULL);
}
