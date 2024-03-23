/*
 * Copyright © 2006 Red Hat, Inc.
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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define WIDTH 800
#define HEIGHT 400

typedef enum {
    WM_NEW_PATH,
    WM_MOVE_TO,
    WM_LINE_TO,
    WM_HLINE_TO,
    WM_VLINE_TO,
    WM_REL_LINE_TO,
    WM_END
} wm_type_t;

typedef struct _wm_element {
    wm_type_t type;
    double x;
    double y;
} wm_element_t;

#include "world-map.h"

enum {
    STROKE = 1,
    FILL = 2,
};

static cairo_test_status_t
draw_world_map (cairo_t *cr, int width, int height, int mode)
{
    const wm_element_t *e;
    double cx, cy;

    cairo_set_line_width (cr, 0.2);

    cairo_set_source_rgb (cr, .68, .85, .90); /* lightblue */
    cairo_rectangle (cr, 0, 0, 800, 400);
    cairo_fill (cr);

    e = &countries[0];
    while (1) {
	switch (e->type) {
	case WM_NEW_PATH:
	case WM_END:
	    if (mode & FILL) {
		cairo_set_source_rgb (cr, .75, .75, .75); /* silver */
		cairo_fill_preserve (cr);
	    }
	    if (mode & STROKE) {
		cairo_set_source_rgb (cr, .50, .50, .50); /* gray */
		cairo_stroke (cr);
	    }
	    cairo_new_path (cr);
	    cairo_move_to (cr, e->x, e->y);
	    break;
	case WM_MOVE_TO:
	    cairo_close_path (cr);
	    cairo_move_to (cr, e->x, e->y);
	    break;
	case WM_LINE_TO:
	    cairo_line_to (cr, e->x, e->y);
	    break;
	case WM_HLINE_TO:
	    cairo_get_current_point (cr, &cx, &cy);
	    cairo_line_to (cr, e->x, cy);
	    break;
	case WM_VLINE_TO:
	    cairo_get_current_point (cr, &cx, &cy);
	    cairo_line_to (cr, cx, e->y);
	    break;
	case WM_REL_LINE_TO:
	    cairo_rel_line_to (cr, e->x, e->y);
	    break;
	}
	if (e->type == WM_END)
	    break;
	e++;
    }

    cairo_new_path (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_world_map_stroke (cairo_t *cr, int width, int height)
{
    return draw_world_map (cr, width, height, STROKE);
}

static cairo_test_status_t
draw_world_map_fill (cairo_t *cr, int width, int height)
{
    return draw_world_map (cr, width, height, FILL);
}

static cairo_test_status_t
draw_world_map_both (cairo_t *cr, int width, int height)
{
    return draw_world_map (cr, width, height, FILL | STROKE);
}

CAIRO_TEST (world_map,
	    "Tests a complex drawing (part of the performance suite)",
	    "fill, stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_world_map_both)
CAIRO_TEST (world_map_stroke,
	    "Tests a complex drawing (part of the performance suite)",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_world_map_stroke)
CAIRO_TEST (world_map_fill,
	    "Tests a complex drawing (part of the performance suite)",
	    "fill", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw_world_map_fill)
