/*
 * Copyright © 2006 Mozilla Corporation
 * Copyright © 2006 Red Hat, Inc.
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
 * Authors: Vladimir Vukicevic <vladimir@pobox.com>
 *	    Carl Worth <cworth@cworth.org>
 */

#ifndef _CAIRO_PERF_H_
#define _CAIRO_PERF_H_

#include "cairo-boilerplate.h"
#include "../src/cairo-time-private.h"
#include <stdio.h>

typedef struct _cairo_stats {
    cairo_time_t min_ticks;
    cairo_time_t median_ticks;
    double ticks_per_ms;
    double std_dev;
    int iterations;
    cairo_time_t *values;
} cairo_stats_t;

typedef struct _cairo_histogram {
    int width, height, max_count;
    int num_columns, num_rows;
    cairo_time_t min_value, max_value;
    int *columns;
} cairo_histogram_t;


/* timers */

void
cairo_perf_timer_start (void);

void
cairo_perf_timer_stop (void);

typedef void
(*cairo_perf_timer_synchronize_t) (void *closure);

void
cairo_perf_timer_set_synchronize (cairo_perf_timer_synchronize_t  synchronize,
				  void				 *closure);

cairo_time_t
cairo_perf_timer_elapsed (void);

/* yield */

void
cairo_perf_yield (void);

/* running a test case */
typedef struct _cairo_perf {
    FILE *summary;
    cairo_bool_t summary_continuous;

    /* Options from command-line */
    unsigned int iterations;
    cairo_bool_t exact_iterations;
    cairo_bool_t raw;
    cairo_bool_t list_only;
    cairo_bool_t observe;
    char **names;
    unsigned int num_names;
    char **exclude_names;
    unsigned int num_exclude_names;
    cairo_bool_t exact_names;

    double ms_per_iteration;
    cairo_bool_t fast_and_sloppy;

    unsigned int tile_size;

    /* Stuff used internally */
    cairo_time_t *times;
    const cairo_boilerplate_target_t **targets;
    int num_targets;
    const cairo_boilerplate_target_t *target;
    cairo_bool_t has_described_backend;
    unsigned int test_number;
    unsigned int size;
    cairo_t *cr;
} cairo_perf_t;

typedef cairo_time_t
(*cairo_perf_func_t) (cairo_t *cr, int width, int height, int loops);

typedef double
(*cairo_count_func_t) (cairo_t *cr, int width, int height);

cairo_bool_t
cairo_perf_can_run (cairo_perf_t *perf,
		    const char	 *name,
		    cairo_bool_t *is_explicit);

void
cairo_perf_run (cairo_perf_t	   *perf,
		const char	   *name,
		cairo_perf_func_t   perf_func,
		cairo_count_func_t  count_func);

void
cairo_perf_cover_sources_and_operators (cairo_perf_t	   *perf,
					const char	   *name,
					cairo_perf_func_t   perf_func,
					cairo_count_func_t  count_func);

/* reporter convenience routines */

typedef struct _test_report {
    int id;
    int fileno;
    const char *configuration;
    char *backend;
    char *content;
    char *name;
    int size;

    /* The samples only exists for "raw" reports */
    cairo_time_t *samples;
    unsigned int samples_size;
    unsigned int samples_count;

    /* The stats are either read directly or computed from samples.
     * If the stats have not yet been computed from samples, then
     * iterations will be 0. */
    cairo_stats_t stats;
} test_report_t;

typedef struct _test_diff {
    test_report_t **tests;
    int num_tests;
    double min;
    double max;
    double change;
} test_diff_t;

typedef struct _cairo_perf_report {
    char *configuration;
    const char *name;
    int fileno;
    test_report_t *tests;
    int tests_size;
    int tests_count;
} cairo_perf_report_t;

typedef enum {
    TEST_REPORT_STATUS_SUCCESS,
    TEST_REPORT_STATUS_COMMENT,
    TEST_REPORT_STATUS_ERROR
} test_report_status_t;

void
cairo_perf_report_load (cairo_perf_report_t *report,
			const char *filename, int id,
			int (*cmp) (const void *, const void *));

void
cairo_perf_report_sort_and_compute_stats (cairo_perf_report_t *report,
					  int (*cmp) (const void *, const void *));

int
test_report_cmp_backend_then_name (const void *a,
				   const void *b);

int
test_report_cmp_name (const void *a,
		      const void *b);

#define CAIRO_PERF_ENABLED_DECL(func) cairo_bool_t (func ## _enabled) (cairo_perf_t *perf)
#define CAIRO_PERF_RUN_DECL(func) void (func) (cairo_perf_t *perf, cairo_t *cr, int width, int height)

#define CAIRO_PERF_DECL(func) CAIRO_PERF_RUN_DECL(func); CAIRO_PERF_ENABLED_DECL(func)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

CAIRO_PERF_DECL (fill);
CAIRO_PERF_DECL (paint);
CAIRO_PERF_DECL (paint_with_alpha);
CAIRO_PERF_DECL (mask);
CAIRO_PERF_DECL (stroke);
CAIRO_PERF_DECL (subimage_copy);
CAIRO_PERF_DECL (disjoint);
CAIRO_PERF_DECL (hatching);
CAIRO_PERF_DECL (tessellate);
CAIRO_PERF_DECL (text);
CAIRO_PERF_DECL (glyphs);
CAIRO_PERF_DECL (hash_table);
CAIRO_PERF_DECL (pattern_create_radial);
CAIRO_PERF_DECL (zrusin);
CAIRO_PERF_DECL (world_map);
CAIRO_PERF_DECL (box_outline);
CAIRO_PERF_DECL (mosaic);
CAIRO_PERF_DECL (long_lines);
CAIRO_PERF_DECL (unaligned_clip);
CAIRO_PERF_DECL (rectangles);
CAIRO_PERF_DECL (rounded_rectangles);
CAIRO_PERF_DECL (long_dashed_lines);
CAIRO_PERF_DECL (composite_checker);
CAIRO_PERF_DECL (twin);
CAIRO_PERF_DECL (dragon);
CAIRO_PERF_DECL (pythagoras_tree);
CAIRO_PERF_DECL (intersections);
CAIRO_PERF_DECL (spiral);
CAIRO_PERF_DECL (wave);
CAIRO_PERF_DECL (many_strokes);
CAIRO_PERF_DECL (wide_strokes);
CAIRO_PERF_DECL (many_fills);
CAIRO_PERF_DECL (wide_fills);
CAIRO_PERF_DECL (many_curves);
CAIRO_PERF_DECL (curve);
CAIRO_PERF_DECL (a1_curve);
CAIRO_PERF_DECL (line);
CAIRO_PERF_DECL (a1_line);
CAIRO_PERF_DECL (pixel);
CAIRO_PERF_DECL (a1_pixel);
CAIRO_PERF_DECL (sierpinski);
CAIRO_PERF_DECL (fill_clip);
CAIRO_PERF_DECL (tiger);

#endif
