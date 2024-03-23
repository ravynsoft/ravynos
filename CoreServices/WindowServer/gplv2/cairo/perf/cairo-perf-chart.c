/*
 * Copyright © 2006 Red Hat, Inc.
 * Copyright © 2009 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission. The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Authors: Carl Worth <cworth@cworth.org>
 *	    Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-perf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

struct chart {
    cairo_perf_report_t *reports;
    const char **names;

    cairo_t *cr;
    int width, height;
    int num_tests, num_reports;
    double min_value, max_value;
    double *average;

    cairo_bool_t use_html;
    cairo_bool_t relative;
};
struct color {
    double red, green, blue;
};

#define FONT_SIZE 12
#define PAD (4)

static double
to_factor (double x)
{
#if 1
    if (x > 1.)
	return (x-1) * 100.;
    else
	return (1. - 1./x) * 100.;
#else
    return log (x);
#endif
}

static int
_double_cmp (const void *_a,
	     const void *_b)
{
    const double *a = _a;
    const double *b = _b;

    if (*a > *b)
	return 1;
    if (*a < *b)
	return -1;
    return 0;
}

static void
trim_outliers (double *values,
	       int     num_values,
	       double *min,
	       double *max)
{
    double q1, q3, iqr;
    double outlier_min, outlier_max;
    int i;

    /* First, identify any outliers, using the definition of "mild
     * outliers" from:
     *
     *		http://en.wikipedia.org/wiki/Outliers
     *
     * Which is that outliers are any values less than Q1 - 1.5 * IQR
     * or greater than Q3 + 1.5 * IQR where Q1 and Q3 are the first
     * and third quartiles and IQR is the inter-quartile range (Q3 -
     * Q1).
     */
    qsort (values, num_values,
	   sizeof (double), _double_cmp);

    q1		= values[1*num_values / 6];
    q3		= values[5*num_values / 6];

    iqr = q3 - q1;

    outlier_min = q1 - 3 * iqr;
    outlier_max = q3 + 3 * iqr;

    i = 0;
    while (i < num_values && values[i] < outlier_min)
	i++;
    if (i == num_values)
	return;

    *min = values[i];

    while (i < num_values && values[i] <= outlier_max)
	i++;

    *max = values[i-1];
}

static void
find_ranges (struct chart *chart)
{
    test_report_t **tests, *min_test;
    double *values;
    int num_values, size_values;
    double min = 0, max = 0;
    double test_time;
    int seen_non_null;
    int num_tests = 0;
    double slow_sum = 0, fast_sum = 0, sum = 0;
    int slow_count = 0, fast_count = 0;
    int *count;
    int i;

    num_values = 0;
    size_values = 64;
    values = xmalloc (size_values * sizeof (double));

    chart->average = xmalloc(chart->num_reports * sizeof(double));
    count = xmalloc(chart->num_reports * sizeof(int));
    for (i = 0; i < chart->num_reports; i++) {
	chart->average[i] = 0;
	count[i] = 0;
    }

    tests = xmalloc (chart->num_reports * sizeof (test_report_t *));
    for (i = 0; i < chart->num_reports; i++)
	tests[i] = chart->reports[i].tests;

    while (1) {
	/* We expect iterations values of 0 when multiple raw reports
	 * for the same test have been condensed into the stats of the
	 * first. So we just skip these later reports that have no
	 * stats. */
	seen_non_null = 0;
	for (i = 0; i < chart->num_reports; i++) {
	    while (tests[i]->name && tests[i]->stats.iterations == 0)
		tests[i]++;
	    if (tests[i]->name)
		seen_non_null++;
	}
	if (! seen_non_null)
	    break;

	num_tests++;

	/* Find the minimum of all current tests, (we have to do this
	 * in case some reports don't have a particular test). */
	for (i = 0; i < chart->num_reports; i++) {
	    if (tests[i]->name) {
		min_test = tests[i];
		break;
	    }
	}
	for (++i; i < chart->num_reports; i++) {
	    if (tests[i]->name && test_report_cmp_name (tests[i], min_test) < 0)
		min_test = tests[i];
	}

	test_time = 0;
	for (i = 0; i < chart->num_reports; i++) {
	    double report_time = HUGE_VAL;

	    while (tests[i]->name &&
		   test_report_cmp_name (tests[i], min_test) == 0)
	    {
		double time = tests[i]->stats.min_ticks;
		if (time < report_time) {
		    time /= tests[i]->stats.ticks_per_ms;
		    if (time < report_time)
			report_time = time;
		}
		tests[i]++;
	    }

	    if (report_time != HUGE_VAL) {
		if (test_time == 0)
		    test_time = report_time;

		chart->average[i] += report_time / test_time;
		count[i]++;

		if (chart->relative) {
		    if (test_time != report_time) {
			double v = to_factor (test_time / report_time);
			if (num_values == size_values) {
			    size_values *= 2;
			    values = xrealloc (values,
					       size_values * sizeof (double));
			}
			values[num_values++] = v;
			if (v < min)
			    min = v;
			if (v > max)
			    max = v;
			if (v > 0)
			    fast_sum += v/100, fast_count++;
			else
			    slow_sum += v/100, slow_count++;
			sum += v/100;
			printf ("%s %d: %f\n", min_test->name, num_values, v);
		    }
		} else {
		    if (report_time < min)
			min = report_time;
		    if (report_time > max)
			max = report_time;
		}
	    }
	}
    }

    for (i = 0; i < chart->num_reports; i++) {
	if (count[i])
	    chart->average[i] = count[i] / chart->average[i];
	else
	    chart->average[i] = 1.;
    }

    if (chart->relative)
	trim_outliers (values, num_values, &min, &max);
    chart->min_value = min;
    chart->max_value = max;
    chart->num_tests = num_tests + !!chart->relative;

    free (values);
    free (tests);
    free (count);

    printf ("%d: slow[%d] average: %f, fast[%d] average: %f, %f\n",
	    num_values, slow_count, slow_sum / slow_count, fast_count, fast_sum / fast_count, sum / num_values);
}

#define SET_COLOR(C, R, G, B) (C)->red = (R), (C)->green = (G), (C)->blue = (B)
static void
hsv_to_rgb (double	  h,
	    double	  s,
	    double	  v,
	    struct color *color)
{
    double m, n, f;
    int i;

    while (h < 0)
	h += 6.;
    while (h > 6.)
	h -= 6.;

    if (s < 0.)
	s = 0.;
    if (s > 1.)
	s = 1.;

    if (v < 0.)
	v = 0.;
    if (v > 1.)
	v = 1.;

    i = floor (h);
    f = h - i;
    if ((i & 1) == 0)
	f = 1 - f;

    m = v * (1 - s);
    n = v * (1 - s * f);
    switch(i){
    default:
    case 6:
    case 0: SET_COLOR (color, v, n, m); break;
    case 1: SET_COLOR (color, n, v, m); break;
    case 2: SET_COLOR (color, m, v, n); break;
    case 3: SET_COLOR (color, m, n, v); break;
    case 4: SET_COLOR (color, n, m, v); break;
    case 5: SET_COLOR (color, v, m, n); break;
    }
}

static void set_report_color (struct chart *chart, int report)
{
    struct color color;

    hsv_to_rgb (6. / chart->num_reports * report, .7, .7, &color);
    cairo_set_source_rgb (chart->cr, color.red, color.green, color.blue);
}

static void set_report_gradient (struct chart *chart, int report,
				 double x, double y, double w, double h)
{
    struct color color;
    cairo_pattern_t *p;

    hsv_to_rgb (6. / chart->num_reports * report, .7, .7, &color);

    p = cairo_pattern_create_linear (x, 0, x+w, 0);
    cairo_pattern_add_color_stop_rgba (p, 0.0,
				       color.red, color.green, color.blue,
				       .50);
    cairo_pattern_add_color_stop_rgba (p, 0.5,
				       color.red, color.green, color.blue,
				       .50);
    cairo_pattern_add_color_stop_rgba (p, 1.0,
				       color.red, color.green, color.blue,
				       1.0);
    cairo_set_source (chart->cr, p);
    cairo_pattern_destroy (p);
}

static void
test_background (struct chart *c,
		 int	       test)
{
    double dx, x;

    dx = c->width / (double) c->num_tests;
    x = dx * test;

    if (test & 1)
	cairo_set_source_rgba (c->cr, .2, .2, .2, .2);
    else
	cairo_set_source_rgba (c->cr, .8, .8, .8, .2);

    cairo_rectangle (c->cr, floor (x), 0,
		     floor (dx + x) - floor (x), c->height);
    cairo_fill (c->cr);
}

static void
add_chart (struct chart *c,
	   int		 test,
	   int		 report,
	   double	 value)
{
    double dx, dy, x;

    if (fabs (value) < 0.1)
	return;

    if (c->relative) {
	cairo_text_extents_t extents;
	char buf[80];
	double y;

	dy = (c->height/2. - PAD) / MAX (-c->min_value, c->max_value);
	/* the first report is always skipped, as it is used as the baseline */
	dx = c->width / (double) (c->num_tests * c->num_reports);
	x = dx * (c->num_reports * test + report - .5);

	cairo_rectangle (c->cr,
			 floor (x), c->height / 2.,
			 floor (x + dx) - floor (x),
			 ceil (-dy*value - c->height/2.) + c->height/2.);
	if (dx < 5) {
	    set_report_color (c, report);
	    cairo_fill (c->cr);
	} else {
	    set_report_gradient (c, report,
				 floor (x), c->height / 2.,
				 floor (x + dx) - floor (x),
				 ceil (-dy*value - c->height/2.) + c->height/2.);

	    cairo_fill_preserve (c->cr);
	    cairo_save (c->cr);
	    cairo_clip_preserve (c->cr);
	    set_report_color (c, report);
	    cairo_stroke (c->cr);
	    cairo_restore (c->cr);
	}

	/* Skip the label if the difference between the two is less than 0.1% */
	if (fabs (value) < 0.1)
		return;

	cairo_save (c->cr);
	cairo_set_font_size (c->cr, dx - 2);

	if (value < 0) {
	    sprintf (buf, "%.1f", -value/100 + 1);
	} else {
	    sprintf (buf, "%.1f", value/100 + 1);
	}
	cairo_text_extents (c->cr, buf, &extents);

	/* will it be clipped? */
	y = -dy * value;
	if (y < -c->height/2) {
	    y = -c->height/2;
	} else if (y > c->height/2) {
	    y = c->height/2;
	}

	if (y < 0) {
	    if (y > -extents.width - 6)
		    y -= extents.width + 6;
	} else {
	    if (y < extents.width + 6)
		    y += extents.width + 6;
	}

	cairo_translate (c->cr,
			 floor (x) + (floor (x + dx) - floor (x))/2,
			 floor (y) + c->height/2.);
	cairo_rotate (c->cr, -M_PI/2);
	if (y < 0) {
	    cairo_move_to (c->cr, -extents.x_bearing -extents.width - 4, -extents.y_bearing/2);
	} else {
	    cairo_move_to (c->cr, 2, -extents.y_bearing/2);
	}

	cairo_set_source_rgb (c->cr, .95, .95, .95);
	cairo_show_text (c->cr, buf);
	cairo_restore (c->cr);
    } else {
	dy = (c->height - PAD) / c->max_value;
	dx = c->width / (double) (c->num_tests * (c->num_reports+1));
	x = dx * ((c->num_reports+1) * test + report + .5);

	cairo_rectangle (c->cr,
			 floor (x), c->height,
			 floor (x + dx) - floor (x),
			 floor (c->height - dy*value) - c->height);
	if (dx < 5) {
	    set_report_color (c, report);
	    cairo_fill (c->cr);
	} else {
	    set_report_gradient (c, report,
				 floor (x), c->height,
				 floor (x + dx) - floor (x),
				 floor (c->height - dy*value) - c->height);
	    cairo_fill_preserve (c->cr);
	    cairo_save (c->cr);
	    cairo_clip_preserve (c->cr);
	    set_report_color (c, report);
	    cairo_stroke (c->cr);
	    cairo_restore (c->cr);
	}
    }
}

static void
add_average (struct chart *c,
	     int		 test,
	     int		 report,
	     double	 value)
{
    double dx, dy, x;
    cairo_text_extents_t extents;
    char buf[80];
    double y;

    if (fabs (value) < 0.1)
	return;

    dy = (c->height/2. - PAD) / MAX (-c->min_value, c->max_value);
    /* the first report is always skipped, as it is used as the baseline */
    dx = c->width / (double) (c->num_tests * c->num_reports);
    x = dx * (c->num_reports * test + report - .5);

    cairo_rectangle (c->cr,
		     floor (x), c->height / 2.,
		     floor (x + dx) - floor (x),
		     ceil (-dy*value - c->height/2.) + c->height/2.);
    if (dx < 5) {
	set_report_color (c, report);
	cairo_fill (c->cr);
    } else {
	set_report_gradient (c, report,
			     floor (x), c->height / 2.,
			     floor (x + dx) - floor (x),
			     ceil (-dy*value - c->height/2.) + c->height/2.);

	cairo_fill_preserve (c->cr);
	cairo_save (c->cr);
	cairo_clip_preserve (c->cr);
	set_report_color (c, report);
	cairo_stroke (c->cr);
	cairo_restore (c->cr);
    }

    /* Skip the label if the difference between the two is less than 0.1% */
    if (fabs (value) < 0.1)
	return;

    cairo_save (c->cr);
    cairo_set_font_size (c->cr, dx - 2);

    if (value < 0) {
	sprintf (buf, "%.1f", -value/100 + 1);
    } else {
	sprintf (buf, "%.1f", value/100 + 1);
    }
    cairo_text_extents (c->cr, buf, &extents);

    /* will it be clipped? */
    y = -dy * value;
    if (y < -c->height/2) {
	y = -c->height/2;
    } else if (y > c->height/2) {
	y = c->height/2;
    }

    if (y < 0) {
	if (y > -extents.width - 6)
	    y -= extents.width + 6;
    } else {
	if (y < extents.width + 6)
	    y += extents.width + 6;
    }

    cairo_translate (c->cr,
		     floor (x) + (floor (x + dx) - floor (x))/2,
		     floor (y) + c->height/2.);
    cairo_rotate (c->cr, -M_PI/2);
    if (y < 0) {
	cairo_move_to (c->cr, -extents.x_bearing -extents.width - 4, -extents.y_bearing/2);
    } else {
	cairo_move_to (c->cr, 2, -extents.y_bearing/2);
    }

    cairo_set_source_rgb (c->cr, .95, .95, .95);
    cairo_show_text (c->cr, buf);
    cairo_restore (c->cr);
}

static void
add_label (struct chart *c,
	   int		 test,
	   const char	*label)
{
    cairo_text_extents_t extents;
    double dx, x;

    cairo_save (c->cr);
    dx = c->width / (double) c->num_tests;
    if (dx / 2 - PAD < 4)
	return;
    cairo_set_font_size (c->cr, dx / 2 - PAD);
    cairo_text_extents (c->cr, label, &extents);

    cairo_set_source_rgb (c->cr, .5, .5, .5);

    x = (test + .5) * dx;
    cairo_save (c->cr);
    cairo_translate (c->cr, x, c->height - PAD / 2);
    cairo_rotate (c->cr, -M_PI/2);
    cairo_move_to (c->cr, 0, -extents.y_bearing/2);
    cairo_show_text (c->cr, label);
    cairo_restore (c->cr);

    cairo_save (c->cr);
    cairo_translate (c->cr, x, PAD / 2);
    cairo_rotate (c->cr, -M_PI/2);
    cairo_move_to (c->cr, -extents.width, -extents.y_bearing/2);
    cairo_show_text (c->cr, label);
    cairo_restore (c->cr);

    cairo_restore (c->cr);
}

static void
add_base_line (struct chart *c)
{
    double y;

    cairo_save (c->cr);
    cairo_set_line_width (c->cr, 2.);
    if (c->relative) {
	y = c->height / 2.;
    } else {
	y = c->height;
    }
    cairo_move_to (c->cr, 0, y);
    cairo_line_to (c->cr, c->width, y);
    cairo_set_source_rgb (c->cr, 1, 1, 1);
    cairo_stroke (c->cr);
    cairo_restore (c->cr);
}

static void
add_absolute_lines (struct chart *c)
{
    const double dashes[] = { 2, 4 };
    const double vlog_steps[] = { 10, 5, 4, 3, 2, 1, .5, .4, .3, .2, .1};
    double v, y, dy;
    unsigned int i;
    char buf[80];
    cairo_text_extents_t extents;

    v = c->max_value / 2.;

    for (i = 0; i < sizeof (vlog_steps) / sizeof (vlog_steps[0]); i++) {
	double vlog = log (v) / log (vlog_steps[i]);
	if (vlog > 1) {
	    v = pow (vlog_steps[i], floor (vlog));
	    goto done;
	}
    }
    return;
done:

    dy = (c->height - PAD) / c->max_value;

    cairo_save (c->cr);
    cairo_set_line_width (c->cr, 1.);
    cairo_set_dash (c->cr, dashes, sizeof (dashes) / sizeof (dashes[0]), 0);

    i = 0;
    do {
	y = c->height - ++i * v * dy;
	if (y < PAD)
	    break;

	cairo_set_font_size (c->cr, 8);

	sprintf (buf, "%.0fs", i*v/1000);
	cairo_text_extents (c->cr, buf, &extents);

	cairo_set_source_rgba (c->cr, .75, 0, 0, .95);
	cairo_move_to (c->cr, 1-extents.x_bearing, floor (y) - (extents.height/2 + extents.y_bearing) + .5);
	cairo_show_text (c->cr, buf);

	cairo_move_to (c->cr, c->width-extents.width-1, floor (y) - (extents.height/2 + extents.y_bearing) + .5);
	cairo_show_text (c->cr, buf);

	cairo_set_source_rgba (c->cr, .75, 0, 0, .5);
	cairo_move_to (c->cr,
		       ceil (extents.width + extents.x_bearing + 2),
		       floor (y) + .5);
	cairo_line_to (c->cr,
		       floor (c->width - (extents.width + extents.x_bearing + 2)),
		       floor (y) + .5);
	cairo_stroke (c->cr);
    } while (1);

    cairo_restore (c->cr);
}

static void
add_relative_lines (struct chart *c)
{
    const double dashes[] = { 2, 4 };
    const double v_steps[] = { 10, 5, 1, .5, .1, .05, .01};
    const int precision_steps[] = { 0, 0, 0, 1, 1, 2, 2};
    int precision;
    double v, y, dy, mid;
    unsigned int i;
    char buf[80];
    cairo_text_extents_t extents;

    v = MAX (-c->min_value, c->max_value) / 200;

    for (i = 0; i < sizeof (v_steps) / sizeof (v_steps[0]); i++) {
	if (v > v_steps[i]) {
	    v = v_steps[i];
	    precision = precision_steps[i];
	    goto done;
	}
    }
    return;
done:

    mid = c->height/2.;
    dy = (mid - PAD) / MAX (-c->min_value, c->max_value);

    cairo_save (c->cr);
    cairo_set_line_width (c->cr, 1.);
    cairo_set_dash (c->cr, dashes, sizeof (dashes) / sizeof (dashes[0]), 0);
    cairo_set_font_size (c->cr, 8);

    i = 0;
    do {
	y = ++i * v * dy * 100;
	if (y > mid)
	    break;

	sprintf (buf, "%.*fx", precision, i*v + 1);
	cairo_text_extents (c->cr, buf, &extents);

	cairo_set_source_rgba (c->cr, .75, 0, 0, .95);
	cairo_move_to (c->cr, 1-extents.x_bearing, floor (mid + y) - (extents.height/2 + extents.y_bearing) + .5);
	cairo_show_text (c->cr, buf);

	cairo_move_to (c->cr, c->width-extents.width-1, floor (mid + y) - (extents.height/2 + extents.y_bearing) + .5);
	cairo_show_text (c->cr, buf);

	cairo_set_source_rgba (c->cr, 0, .75, 0, .95);
	cairo_move_to (c->cr, 1-extents.x_bearing, ceil (mid - y) - (extents.height/2 + extents.y_bearing) + .5);
	cairo_show_text (c->cr, buf);

	cairo_move_to (c->cr, c->width-extents.width-1, ceil (mid - y) - (extents.height/2 + extents.y_bearing) + .5);
	cairo_show_text (c->cr, buf);

	/* trim the dashes to no obscure the labels */
	cairo_set_source_rgba (c->cr, .75, 0, 0, .5);
	cairo_move_to (c->cr,
		       ceil (extents.width + extents.x_bearing + 2),
		       floor (mid + y) + .5);
	cairo_line_to (c->cr,
		       floor (c->width - (extents.width + 2)),
		       floor (mid + y) + .5);
	cairo_stroke (c->cr);

	cairo_set_source_rgba (c->cr, 0, .75, 0, .5);
	cairo_move_to (c->cr,
		       ceil (extents.width + extents.x_bearing + 2),
		       ceil (mid - y) + .5);
	cairo_line_to (c->cr,
		       floor (c->width - (extents.width + 2)),
		       ceil (mid - y) + .5);
	cairo_stroke (c->cr);

    } while (1);

    cairo_restore (c->cr);
}

static void
add_slower_faster_guide (struct chart *c)
{
    cairo_text_extents_t extents;

    cairo_save (c->cr);

    cairo_set_font_size (c->cr, FONT_SIZE);

    cairo_text_extents (c->cr, "FASTER", &extents);
    cairo_set_source_rgba (c->cr, 0, .75, 0, .5);
    cairo_move_to (c->cr,
		   c->width/4. - extents.width/2. + extents.x_bearing,
		   1 - extents.y_bearing);
    cairo_show_text (c->cr, "FASTER");
    cairo_move_to (c->cr,
		   3*c->width/4. - extents.width/2. + extents.x_bearing,
		   1 - extents.y_bearing);
    cairo_show_text (c->cr, "FASTER");

    cairo_text_extents (c->cr, "SLOWER", &extents);
    cairo_set_source_rgba (c->cr, .75, 0, 0, .5);
    cairo_move_to (c->cr,
		   c->width/4. - extents.width/2. + extents.x_bearing,
		   c->height - 1);
    cairo_show_text (c->cr, "SLOWER");
    cairo_move_to (c->cr,
		   3*c->width/4. - extents.width/2. + extents.x_bearing,
		   c->height - 1);
    cairo_show_text (c->cr, "SLOWER");

    cairo_restore (c->cr);
}

static void
cairo_perf_reports_compare (struct chart *chart,
			    cairo_bool_t  print)
{
    test_report_t **tests, *min_test;
    double test_time, best_time;
    int num_test = 0;
    int seen_non_null;
    int i;

    tests = xmalloc (chart->num_reports * sizeof (test_report_t *));
    for (i = 0; i < chart->num_reports; i++)
	tests[i] = chart->reports[i].tests;

    if (print) {
	if (chart->use_html) {
	    printf ("<table style=\"text-align:right\" cellspacing=\"4\">\n");
	    printf ("<tr><td></td>");
	    for (i = 0; i < chart->num_reports; i++) {
		printf ("<td>%s</td>", chart->names[i] ? chart->names[i] : "");
	    }
	    printf ("</tr>\n");
	}
    }

    while (1) {
	/* We expect iterations values of 0 when multiple raw reports
	 * for the same test have been condensed into the stats of the
	 * first. So we just skip these later reports that have no
	 * stats. */
	seen_non_null = 0;
	for (i = 0; i < chart->num_reports; i++) {
	    while (tests[i]->name && tests[i]->stats.iterations == 0)
		tests[i]++;
	    if (tests[i]->name)
		seen_non_null++;
	}
	if (! seen_non_null)
	    break;

	/* Find the minimum of all current tests, (we have to do this
	 * in case some reports don't have a particular test). */
	for (i = 0; i < chart->num_reports; i++) {
	    if (tests[i]->name) {
		min_test = tests[i];
		break;
	    }
	}
	for (++i; i < chart->num_reports; i++) {
	    if (tests[i]->name && test_report_cmp_name (tests[i], min_test) < 0)
		min_test = tests[i];
	}

	add_label (chart, num_test, min_test->name);
	if (print) {
	    if (chart->use_html) {
		printf ("<tr><td>%s</td>", min_test->name);
	    } else {
		if (min_test->size) {
		    printf ("%16s, size %4d:\n",
			    min_test->name,
			    min_test->size);
		} else {
		    printf ("%26s:",
			    min_test->name);
		}
	    }
	}

	test_time = 0;
	best_time = HUGE_VAL;
	for (i = 0; i < chart->num_reports; i++) {
	    test_report_t *initial = tests[i];
	    double report_time = HUGE_VAL;

	    while (tests[i]->name &&
		   test_report_cmp_name (tests[i], min_test) == 0)
	    {
		double time = tests[i]->stats.min_ticks;
		if (time < report_time) {
		    time /= tests[i]->stats.ticks_per_ms;
		    if (time < report_time)
			report_time = time;
		}
		tests[i]++;
	    }

	    if (test_time == 0 && report_time != HUGE_VAL)
		test_time = report_time;
	    if (report_time < best_time)
		best_time = report_time;

	    tests[i] = initial;
	}

	for (i = 0; i < chart->num_reports; i++) {
	    double report_time = HUGE_VAL;

	    while (tests[i]->name &&
		   test_report_cmp_name (tests[i], min_test) == 0)
	    {
		double time = tests[i]->stats.min_ticks;
		if (time > 0) {
		    time /= tests[i]->stats.ticks_per_ms;
		    if (time < report_time)
			report_time = time;
		}
		tests[i]++;
	    }

	    if (print) {
		if (chart->use_html) {
		    if (report_time < HUGE_VAL) {
			if (report_time / best_time < 1.01) {
			    printf ("<td><strong>%.1f</strong></td>", report_time/1000);
			} else {
			    printf ("<td>%.1f</td>", report_time/1000);
			}
		    } else {
			printf ("<td></td>");
		    }
		} else {
		    if (report_time < HUGE_VAL)
			printf (" %6.1f",  report_time/1000);
		    else
			printf ("    ---");
		}
	    }

	    if (report_time < HUGE_VAL) {
		if (chart->relative) {
		    add_chart (chart, num_test, i,
			       to_factor (test_time / report_time));
		} else {
		    add_chart (chart, num_test, i, report_time);
		}
	    }
	}

	if (print) {
	    if (chart->use_html) {
		printf ("</tr>\n");
	    } else {
		printf ("\n");
	    }
	}

	num_test++;
    }
    if (chart->relative) {
	add_label (chart, num_test, "(geometric mean)");
	for (i = 0; i < chart->num_reports; i++)
	    add_average (chart, num_test, i, to_factor (chart->average[i]));
    }
    free (tests);

    if (print) {
	if (chart->use_html)
	    printf ("</table>\n");

	printf ("\n");
	for (i = 0; i < chart->num_reports; i++) {
	    if (chart->names[i]) {
		printf ("[%s] %s\n",
			chart->names[i], chart->reports[i].configuration);
	    } else {
		printf ("[%d] %s\n",
			i, chart->reports[i].configuration);
	    }
	}
    }
}

static void
add_legend (struct chart *chart)
{
    cairo_text_extents_t extents;
    const char *str;
    int i, x, y;

    cairo_set_font_size (chart->cr, FONT_SIZE);

    x = PAD;
    y = chart->height + PAD;
    for (i = chart->relative; i < chart->num_reports; i++) {
	str = chart->names[i] ?
	      chart->names[i] : chart->reports[i].configuration;

	set_report_color (chart, i);

	cairo_rectangle (chart->cr, x, y + 6, 8, 8);
	cairo_fill (chart->cr);

	cairo_set_source_rgb (chart->cr, 1, 1, 1);
	cairo_move_to (chart->cr, x + 10, y + FONT_SIZE + PAD / 2.);
	cairo_text_extents (chart->cr, str, &extents);
	cairo_show_text (chart->cr, str);

	x += 10 + 2 * PAD + ceil (extents.width);
    }

    if (chart->relative) {
	char buf[80];

	str = chart->names[0] ?
	      chart->names[0] : chart->reports[0].configuration;

	sprintf (buf, "(relative to %s)", str);
	cairo_text_extents (chart->cr, buf, &extents);

	cairo_set_source_rgb (chart->cr, 1, 1, 1);
	cairo_move_to (chart->cr,
		       chart->width - 1 - extents.width,
		       y + FONT_SIZE + PAD / 2.);
	cairo_show_text (chart->cr, buf);
    }
}

static void
usage (void)
{
	printf("Usage:\n");
	printf("  cairo-perf-chart [OPTION...] <result1> <result2>...<resultN>\n");
	printf("\n");
	printf("Help Options:\n");
	printf("  --help, --?\tShow help options\n");
	printf("\n");
	printf("Application Options:\n");
	printf("  --html\tOutput an HTML table comparing the results\n");
	printf("  --height=\tSet the height of the output graph"\
			" (default 480)\n");
	printf("  --width=\tSet the width of the output graph"\
			" (default 640)\n");
	printf("  --name\tSet the name of graph series."\
			" This only sets the name for the\n\t\tfirst result file."\
			" The graph series is usually set using the\n\t\tfile name for"\
			" the results file.\n");
	printf("\n");
	printf("Example:\n");
	printf("  cairo-perf-chart --width=1024 --height=768 run1 run2 run3\n");
	return;
}

int
main (int	  argc,
      const char *argv[])
{
    cairo_surface_t *surface;
    struct chart chart;
    test_report_t *t;
    int i;

    chart.use_html = 0;
    chart.width = 640;
    chart.height = 480;

    chart.reports = xcalloc (argc-1, sizeof (cairo_perf_report_t));
    chart.names = xcalloc (argc-1, sizeof (cairo_perf_report_t));

    chart.num_reports = 0;
    for (i = 1; i < argc; i++) {
	if (strcmp (argv[i], "--html") == 0) {
	    chart.use_html = 1;
	} else if (strncmp (argv[i], "--width=", 8) == 0) {
	    chart.width = atoi (argv[i] + 8);
	} else if (strncmp (argv[i], "--height=", 9) == 0) {
	    chart.height = atoi (argv[i] + 9);
	} else if (strcmp (argv[i], "--name") == 0) {
	    if (i + 1 < argc)
		chart.names[chart.num_reports] = argv[++i];
	} else if (strncmp (argv[i], "--name=", 7) == 0) {
	    chart.names[chart.num_reports] = argv[i] + 7;
	} else if ((strcmp (argv[i], "--help") == 0) ||
		(strcmp (argv[i], "--?") == 0)) {
		usage();
		return 0;
	} else {
	    cairo_perf_report_load (&chart.reports[chart.num_reports++],
				    argv[i], i,
				    test_report_cmp_name);
	}
    }

    for (chart.relative = 0; chart.relative <= 1; chart.relative++) {
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					      chart.width,
					      chart.height + (FONT_SIZE + PAD) + 2*PAD);
	chart.cr = cairo_create (surface);
	cairo_surface_destroy (surface);

	cairo_set_source_rgb (chart.cr, 0, 0, 0);
	cairo_paint (chart.cr);

	find_ranges (&chart);

	for (i = 0; i < chart.num_tests; i++)
	    test_background (&chart, i);
	if (chart.relative) {
	    add_relative_lines (&chart);
	    add_slower_faster_guide (&chart);
	} else
	    add_absolute_lines (&chart);

	cairo_save (chart.cr);
	cairo_rectangle (chart.cr, 0, 0, chart.width, chart.height);
	cairo_clip (chart.cr);
	cairo_perf_reports_compare (&chart, !chart.relative);
	cairo_restore (chart.cr);

	add_base_line (&chart);
	add_legend (&chart);

	cairo_surface_write_to_png (cairo_get_target (chart.cr),
				    chart.relative ? "relative.png" : "absolute.png");
	cairo_destroy (chart.cr);
    }

    /* Pointless memory cleanup, (would be a great place for talloc) */
    for (i = 0; i < chart.num_reports; i++) {
	for (t = chart.reports[i].tests; t->name; t++) {
	    free (t->samples);
	    free (t->backend);
	    free (t->name);
	}
	free (chart.reports[i].tests);
	free (chart.reports[i].configuration);
    }
    free (chart.names);
    free (chart.reports);

    return 0;
}
