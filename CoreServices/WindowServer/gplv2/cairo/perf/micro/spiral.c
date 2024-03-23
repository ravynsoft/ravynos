/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright (c) 2008  M Joonas Pihlaja
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "cairo-perf.h"
#include <assert.h>

#define MAX_SEGMENTS 2560

typedef enum {
    PIXALIGN,                   /* pixel aligned path */
    NONALIGN                    /* unaligned path. */
} align_t;

typedef enum {
    RECTCLOSE,                  /* keeps the path rectilinear */
    DIAGCLOSE                   /* forces a diagonal */
} close_t;

static cairo_time_t
draw_spiral (cairo_t *cr,
             cairo_fill_rule_t fill_rule,
             align_t align,
             close_t close,
             int width, int height, int loops)
{
    int i;
    int n=0;
    double x[MAX_SEGMENTS];
    double y[MAX_SEGMENTS];
    int step = 3;
    int side = width < height ? width : height;

    assert(5*(side/step/2+1)+2 < MAX_SEGMENTS);

#define L(x_,y_) (x[n] = (x_), y[n] = (y_), n++)
#define M(x_,y_) L(x_,y_)
#define v(t) L(x[n-1], y[n-1] + (t))
#define h(t) L(x[n-1] + (t), y[n-1])

    switch (align) {
    case PIXALIGN: M(0,0); break;
    case NONALIGN: M(0.1415926, 0.7182818); break;
    }

    while (side >= step && side >= 0) {
        v(side);
        h(side);
        v(-side);
        h(-side+step);
        v(step);
        side -= 2*step;
    }

    switch (close) {
    case RECTCLOSE: L(x[n-1],y[0]); break;
    case DIAGCLOSE: L(x[0],y[0]); break;
    }

    assert(n < MAX_SEGMENTS);

    cairo_save (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_translate (cr, 1, 1);
    cairo_set_fill_rule (cr, fill_rule);
    cairo_set_source_rgb (cr, 1, 0, 0);

    cairo_new_path (cr);
    cairo_move_to (cr, x[0], y[0]);
    for (i = 1; i < n; i++) {
	cairo_line_to (cr, x[i], y[i]);
    }
    cairo_close_path (cr);

    cairo_perf_timer_start ();
    while (loops--)
        cairo_fill_preserve (cr);
    cairo_perf_timer_stop ();

    cairo_restore (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
draw_spiral_box (cairo_t *cr,
		 cairo_fill_rule_t fill_rule,
		 align_t align,
		 int width, int height, int loops)
{
    const int step = 3;
    int side = (width < height ? width : height) - 2;

    cairo_save (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_set_fill_rule (cr, fill_rule);
    cairo_translate (cr, 1, 1);
    if (align == NONALIGN)
	cairo_translate (cr, 0.1415926, 0.7182818);

    cairo_new_path (cr);
    while (side >= step) {
	cairo_rectangle (cr, 0, 0, side, side);
	cairo_translate (cr, step, step);
	side -= 2*step;
    }

    cairo_perf_timer_start ();
    while (loops--)
        cairo_fill_preserve (cr);
    cairo_perf_timer_stop ();

    cairo_restore (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
draw_spiral_stroke (cairo_t *cr,
		    align_t align,
		    int width, int height, int loops)
{
    const int step = 3;
    int side = width < height ? width : height;

    cairo_save (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    cairo_translate (cr, 1, 1);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_set_line_width (cr, 4.);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);

    cairo_new_path (cr);
    switch (align) {
    case PIXALIGN: cairo_move_to (cr, 0,0); break;
    case NONALIGN: cairo_move_to (cr, 0.1415926, 0.7182818); break;
    }
    while (side >= step) {
	cairo_rel_line_to (cr, 0, side);
        side -= step;
	if (side <= 0)
	    break;

	cairo_rel_line_to (cr, side, 0);
        side -= step;
	if (side <= 0)
	    break;

	cairo_rel_line_to (cr, 0, -side);
        side -= step;
	if (side <= 0)
	    break;

	cairo_rel_line_to (cr, -side, 0);
        side -= step;
	if (side <= 0)
	    break;
    }

    cairo_perf_timer_start ();
    while (loops--)
        cairo_stroke_preserve (cr);
    cairo_perf_timer_stop ();

    cairo_restore (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
draw_spiral_eo_pa_re (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_EVEN_ODD,
                        PIXALIGN,
                        RECTCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_nz_pa_re (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_WINDING,
                        PIXALIGN,
                        RECTCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_eo_na_re (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_EVEN_ODD,
                        NONALIGN,
                        RECTCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_nz_na_re (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_WINDING,
                        NONALIGN,
                        RECTCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_eo_pa_di (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_EVEN_ODD,
                        PIXALIGN,
                        DIAGCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_nz_pa_di (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_WINDING,
                        PIXALIGN,
                        DIAGCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_eo_na_di (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_EVEN_ODD,
                        NONALIGN,
                        DIAGCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_nz_na_di (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral (cr,
                        CAIRO_FILL_RULE_WINDING,
                        NONALIGN,
                        DIAGCLOSE,
                        width, height, loops);
}

static cairo_time_t
draw_spiral_nz_pa_box (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral_box (cr,
			    CAIRO_FILL_RULE_WINDING, PIXALIGN,
			    width, height, loops);
}

static cairo_time_t
draw_spiral_nz_na_box (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral_box (cr,
			    CAIRO_FILL_RULE_WINDING, NONALIGN,
			    width, height, loops);
}


static cairo_time_t
draw_spiral_eo_pa_box (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral_box (cr,
			    CAIRO_FILL_RULE_EVEN_ODD, PIXALIGN,
			    width, height, loops);
}

static cairo_time_t
draw_spiral_eo_na_box (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral_box (cr,
			    CAIRO_FILL_RULE_EVEN_ODD, NONALIGN,
			    width, height, loops);
}

static cairo_time_t
draw_spiral_stroke_pa (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral_stroke (cr,
			       PIXALIGN,
			       width, height, loops);
}

static cairo_time_t
draw_spiral_stroke_na (cairo_t *cr, int width, int height, int loops)
{
    return draw_spiral_stroke (cr,
			       NONALIGN,
			       width, height, loops);
}

cairo_bool_t
spiral_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "spiral", NULL);
}

void
spiral (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "spiral-box-nonalign-evenodd-fill", draw_spiral_eo_na_box, NULL);
    cairo_perf_run (perf, "spiral-box-nonalign-nonzero-fill", draw_spiral_nz_na_box, NULL);
    cairo_perf_run (perf, "spiral-box-pixalign-evenodd-fill", draw_spiral_eo_pa_box, NULL);
    cairo_perf_run (perf, "spiral-box-pixalign-nonzero-fill", draw_spiral_nz_pa_box, NULL);
    cairo_perf_run (perf, "spiral-diag-nonalign-evenodd-fill", draw_spiral_eo_na_di, NULL);
    cairo_perf_run (perf, "spiral-diag-nonalign-nonzero-fill", draw_spiral_nz_na_di, NULL);
    cairo_perf_run (perf, "spiral-diag-pixalign-evenodd-fill", draw_spiral_eo_pa_di, NULL);
    cairo_perf_run (perf, "spiral-diag-pixalign-nonzero-fill", draw_spiral_nz_pa_di, NULL);
    cairo_perf_run (perf, "spiral-rect-nonalign-evenodd-fill", draw_spiral_eo_na_re, NULL);
    cairo_perf_run (perf, "spiral-rect-nonalign-nonzero-fill", draw_spiral_nz_na_re, NULL);
    cairo_perf_run (perf, "spiral-rect-pixalign-evenodd-fill", draw_spiral_eo_pa_re, NULL);
    cairo_perf_run (perf, "spiral-rect-pixalign-nonzero-fill", draw_spiral_nz_pa_re, NULL);
    cairo_perf_run (perf, "spiral-nonalign-stroke", draw_spiral_stroke_na, NULL);
    cairo_perf_run (perf, "spiral-pixalign-stroke", draw_spiral_stroke_pa, NULL);
}
