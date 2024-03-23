/*
 * Copyright © 2006 Joonas Pihlaja
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
 * Author: Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
 */
#include "cairo-perf.h"

/* Options passed in flags to mosaic_perform(): */
#define MOSAIC_FILL 1		/* do rasterise */
#define MOSAIC_TESSELLATE 0	/* just tessellate */
#define MOSAIC_CURVE_TO 2	/* use curve bounded regions */
#define MOSAIC_LINE_TO 0	/* use line bounded regions */

struct mosaic_region {
    unsigned rgb;		/* colour of this region in 0xRRGGBB format */
    unsigned ncurves;		/* number of boundary curves. */
};

struct mosaic_region_iter {
    int do_curves;
    struct mosaic_region const *region;
    double const *points;
};

#include "mosaic.h"

static void
mosaic_region_iter_init (struct mosaic_region_iter *iter, int do_curves)
{
    iter->region = mosaic_regions;
    iter->points = mosaic_curve_points;
    iter->do_curves = do_curves;
}

/* Create the next closed region as a path. */
static int
mosaic_next_path (cairo_t *cr, struct mosaic_region_iter *iter)
{
    double const *points = iter->points;
    unsigned i;
    unsigned ncurves = iter->region->ncurves;
    if (0 == ncurves) {
	return 0;
    }

    cairo_new_path (cr);
    cairo_move_to (cr, points[0], points[1]);
    points += 2;
    for (i=0; i < ncurves; i++, points += 6) {
	if (iter->do_curves) {
	    cairo_curve_to (cr,
			    points[0], points[1],
			    points[2], points[3],
			    points[4], points[5]);
	}
	else {
	    cairo_line_to (cr,
			    points[4], points[5]);
	}
    }
    cairo_close_path (cr);
    {
	unsigned rgb = iter->region->rgb;
	double r = ((rgb >> 16) & 255) / 255.0;
	double g = ((rgb >>  8) & 255) / 255.0;
	double b = ((rgb >>  0) & 255) / 255.0;
	cairo_set_source_rgb (cr, r, g, b);
    }

    iter->points = iter->points + 2*(1 + 3*iter->region->ncurves);
    iter->region++;
    return 1;
}

static cairo_time_t
mosaic_perform(cairo_t *cr, unsigned flags, int width, int height, int loops)
{
    struct mosaic_region_iter iter;

    /* Scale to fit the window.*/
    double minx = -40.7;
    double maxx = 955.1;
    double miny = -88.4;
    double maxy = 884.5;

    cairo_identity_matrix (cr);

    if (flags & MOSAIC_FILL) {
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
    }

    cairo_scale (cr, width / (maxx - minx) , height / (maxy - miny));
    cairo_translate (cr, -minx, -miny);

    /* Iterate over all closed regions in the mosaic filling or
     * tessellating them as dictated by the flags.  */

    cairo_perf_timer_start ();
    while (loops--) {
	mosaic_region_iter_init (&iter, flags & MOSAIC_CURVE_TO);
	while (mosaic_next_path (cr, &iter)) {
	    if (flags & MOSAIC_FILL) {
		cairo_fill (cr);
	    }
	    else {
		double x, y;
		cairo_get_current_point (cr, &x, &y);
		cairo_in_fill (cr, x, y);
	    }
	}
    }
    cairo_perf_timer_stop ();

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
mosaic_fill_curves (cairo_t *cr, int width, int height, int loops)
{
    return mosaic_perform (cr, MOSAIC_FILL | MOSAIC_CURVE_TO, width, height, loops);
}

static cairo_time_t
mosaic_fill_lines (cairo_t *cr, int width, int height, int loops)
{
    return mosaic_perform (cr, MOSAIC_FILL | MOSAIC_LINE_TO, width, height, loops);
}

static cairo_time_t
mosaic_tessellate_lines (cairo_t *cr, int width, int height, int loops)
{
    return mosaic_perform (cr, MOSAIC_TESSELLATE | MOSAIC_LINE_TO, width, height, loops);
}

static cairo_time_t
mosaic_tessellate_curves (cairo_t *cr, int width, int height, int loops)
{
    return mosaic_perform (cr, MOSAIC_TESSELLATE | MOSAIC_CURVE_TO, width, height, loops);
}

cairo_bool_t
mosaic_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "mosaic", NULL);
}

void
mosaic (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "mosaic-fill-curves", mosaic_fill_curves, NULL);
    cairo_perf_run (perf, "mosaic-fill-lines", mosaic_fill_lines, NULL);
    cairo_perf_run (perf, "mosaic-tessellate-curves", mosaic_tessellate_curves, NULL);
    cairo_perf_run (perf, "mosaic-tessellate-lines", mosaic_tessellate_lines, NULL);
}
