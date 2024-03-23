/*
 * Copyright © 2006 Red Hat, Inc.
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
 */

#include "cairo-perf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

typedef struct _cairo_perf_report_options {
    double min_change;
    int use_utf;
    int print_change_bars;
    int use_ticks;
} cairo_perf_report_options_t;

typedef struct _cairo_perf_diff_files_args {
    const char **filenames;
    int num_filenames;
    cairo_perf_report_options_t options;
} cairo_perf_diff_files_args_t;

static int
test_diff_cmp_speedup_before_slowdown (const void *a,
				       const void *b)
{
    const test_diff_t *a_diff = a;
    const test_diff_t *b_diff = b;

    /* First make all speedups come before all slowdowns. */
    if (a_diff->change > 0 && b_diff->change < 0)
	return -1;
    if (a_diff->change < 0 && b_diff->change > 0)
	return 1;

    if (a_diff->change == b_diff->change)
	return 0;

    /* Large speedups come first. */
    if (a_diff->change > 0) {
	    if (a_diff->change > b_diff->change)
		    return -1;
	    else
		    return 1;
    }

    /* Large slowdowns come last. */
    if (a_diff->change < 0) {
	    if (a_diff->change < b_diff->change)
		    return 1;
	    else
		    return -1;
    }

    return 0;
}

static int
test_diff_cmp (const void *a,
	       const void *b)
{
    const test_diff_t *a_diff = a;
    const test_diff_t *b_diff = b;

    /* Reverse sort by magnitude of change so larger changes come
     * first */
    if (a_diff->change > b_diff->change)
	return -1;

    if (a_diff->change < b_diff->change)
	return 1;

    return 0;
}

#define CHANGE_BAR_WIDTH 70
static void
print_change_bar (double change,
		  double max_change,
		  int	 use_utf)
{
    int units_per_cell = ceil (max_change / CHANGE_BAR_WIDTH);
    static char const *ascii_boxes[8] = {
	"****","***" ,"***", "**",
	"**",  "*",   "*",   ""
    };
    static char const *utf_boxes[8] = {
	"█", "▉", "▊", "▋",
	"▌", "▍", "▎", "▏"
    };
    char const **boxes = use_utf ? utf_boxes : ascii_boxes;

    /* For a 1.0x speedup we want a zero-size bar to show "no
     * change". */
    change -= 1.0;

    while (change > units_per_cell) {
	printf ("%s", boxes[0]);
	change -= units_per_cell;
    }

    change /= units_per_cell;

    if (change > 7.5/8.0)
	printf ("%s", boxes[0]);
    else if (change > 6.5/8.0)
	printf ("%s", boxes[1]);
    else if (change > 5.5/8.0)
	printf ("%s", boxes[2]);
    else if (change > 4.5/8.0)
	printf ("%s", boxes[3]);
    else if (change > 3.5/8.0)
	printf ("%s", boxes[4]);
    else if (change > 2.5/8.0)
	printf ("%s", boxes[5]);
    else if (change > 1.5/8.0)
	printf ("%s", boxes[6]);
    else if (change > 0.5/8.0)
	printf ("%s", boxes[7]);

    printf ("\n");
}

static void
test_diff_print_binary (test_diff_t		    *diff,
			double			     max_change,
			cairo_perf_report_options_t *options)
{
    if (diff->tests[0]->size)
	printf ("%5s-%-4s %26s-%-3d",
		diff->tests[0]->backend, diff->tests[0]->content,
		diff->tests[0]->name, diff->tests[0]->size);
    else
	printf ("%5s %26s", diff->tests[0]->backend, diff->tests[0]->name);

    printf ("  %6.2f (%.2f %4.2f%%) -> %6.2f (%.2f %4.2f%%): %5.2fx ",
	    diff->tests[0]->stats.min_ticks / diff->tests[0]->stats.ticks_per_ms,
	    diff->tests[0]->stats.median_ticks / diff->tests[0]->stats.ticks_per_ms,
	    diff->tests[0]->stats.std_dev * 100,
	    diff->tests[1]->stats.min_ticks / diff->tests[1]->stats.ticks_per_ms,
	    diff->tests[1]->stats.median_ticks / diff->tests[1]->stats.ticks_per_ms,
	    diff->tests[1]->stats.std_dev * 100,
	    fabs (diff->change));

    if (diff->change > 1.0)
	printf ("speedup\n");
    else
	printf ("slowdown\n");

    if (options->print_change_bars)
	print_change_bar (fabs (diff->change), max_change,
			  options->use_utf);
}

static void
test_diff_print_multi (test_diff_t		   *diff,
		       double			    max_change,
		       cairo_perf_report_options_t *options)
{
    int i;
    double test_time;
    double change;

    if (diff->tests[0]->size) {
	printf ("%s (backend: %s-%s, size: %d)\n",
		diff->tests[0]->name,
		diff->tests[0]->backend,
		diff->tests[0]->content,
		diff->tests[0]->size);
    } else {
	printf ("%s (backend: %s)\n",
		diff->tests[0]->name,
		diff->tests[0]->backend);
    }

    for (i = 0; i < diff->num_tests; i++) {
	test_time = diff->tests[i]->stats.min_ticks;
	if (! options->use_ticks)
	    test_time /= diff->tests[i]->stats.ticks_per_ms;
	change = diff->max / test_time;
	printf ("[%d] %6.2f: %5.2fx ",
		diff->tests[i]->fileno,
		diff->tests[i]->stats.min_ticks / diff->tests[i]->stats.ticks_per_ms,
		change);

	if (options->print_change_bars)
	    print_change_bar (change, max_change, options->use_utf);
	else
	    printf("\n");
    }

    printf("\n");
}

static void
cairo_perf_reports_compare (cairo_perf_report_t 	*reports,
			    int 			 num_reports,
			    cairo_perf_report_options_t *options)
{
    int i;
    test_report_t **tests, *min_test;
    test_diff_t *diff, *diffs;
    int num_diffs, max_diffs;
    double max_change;
    double test_time;
    int seen_non_null;
    cairo_bool_t printed_speedup = FALSE;
    cairo_bool_t printed_slowdown = FALSE;

    assert (num_reports >= 2);

    tests = xmalloc (num_reports * sizeof (test_report_t *));

    max_diffs = reports[0].tests_count;
    for (i = 0; i < num_reports; i++) {
	tests[i] = reports[i].tests;
	if (reports[i].tests_count > max_diffs)
	    max_diffs = reports[i].tests_count;
    }

    diff = diffs = xmalloc (max_diffs * sizeof (test_diff_t));

    num_diffs = 0;
    while (1) {
	/* We expect iterations values of 0 when multiple raw reports
	 * for the same test have been condensed into the stats of the
	 * first. So we just skip these later reports that have no
	 * stats. */
	seen_non_null = 0;
	for (i = 0; i < num_reports; i++) {
	    while (tests[i]->name && tests[i]->stats.iterations == 0)
		tests[i]++;
	    if (tests[i]->name)
		seen_non_null++;
	}

	if (seen_non_null < 2)
	    break;

	/* Find the minimum of all current tests, (we have to do this
	 * in case some reports don't have a particular test). */
	for (i = 0; i < num_reports; i++) {
	    if (tests[i]->name) {
		min_test = tests[i];
		break;
	    }
	}
	for (++i; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) < 0)
	    {
		min_test = tests[i];
	    }
	}

	/* For each report that has the current test, record it into
	 * the diff structure. */
	diff->num_tests = 0;
	diff->tests = xmalloc (num_reports * sizeof (test_diff_t));
	for (i = 0; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) == 0)
	    {
		test_time = tests[i]->stats.min_ticks;
		if (! options->use_ticks)
		    test_time /= tests[i]->stats.ticks_per_ms;
		if (diff->num_tests == 0) {
		    diff->min = test_time;
		    diff->max = test_time;
		} else {
		    if (test_time < diff->min)
			diff->min = test_time;
		    if (test_time > diff->max)
			diff->max = test_time;
		}
		diff->tests[diff->num_tests++] = tests[i];
		tests[i]++;
	    }
	}
	diff->change = diff->max / diff->min;

	if (num_reports == 2) {
	    double old_time, new_time;
	    if (diff->num_tests == 1) {
		printf ("Only in %s: %s %s\n",
			diff->tests[0]->configuration,
			diff->tests[0]->backend,
			diff->tests[0]->name);
		continue;
	    }
	    old_time = diff->tests[0]->stats.min_ticks;
	    new_time = diff->tests[1]->stats.min_ticks;
	    if (! options->use_ticks) {
		old_time /= diff->tests[0]->stats.ticks_per_ms;
		new_time /= diff->tests[1]->stats.ticks_per_ms;
	    }
	    diff->change = old_time / new_time;
	    if (diff->change < 1.0)
		diff->change = - 1.0 / diff->change;
	}

	diff++;
	num_diffs++;
    }
    if (num_diffs == 0)
	goto DONE;

    if (num_reports == 2)
	qsort (diffs, num_diffs, sizeof (test_diff_t),
	       test_diff_cmp_speedup_before_slowdown);
    else
	qsort (diffs, num_diffs, sizeof (test_diff_t), test_diff_cmp);

    max_change = 1.0;
    for (i = 0; i < num_diffs; i++) {
	if (fabs (diffs[i].change) > max_change)
	    max_change = fabs (diffs[i].change);
    }

    if (num_reports == 2)
	printf ("old: %s\n"
		"new: %s\n",
		diffs->tests[0]->configuration,
		diffs->tests[1]->configuration);

    for (i = 0; i < num_diffs; i++) {
	diff = &diffs[i];

	/* Discard as uninteresting a change which is less than the
	 * minimum change required, (default may be overridden on
	 * command-line). */
	if (fabs (diff->change) - 1.0 < options->min_change)
	    continue;

	if (num_reports == 2) {
	    if (diff->change > 1.0 && ! printed_speedup) {
		printf ("Speedups\n"
			"========\n");
		printed_speedup = TRUE;
	    }
	    if (diff->change < 1.0 && ! printed_slowdown) {
		printf ("Slowdowns\n"
			"=========\n");
		printed_slowdown = TRUE;
	    }
	    test_diff_print_binary (diff, max_change, options);
	} else {
	    test_diff_print_multi (diff, max_change, options);
	}
    }

 DONE:
    for (i = 0; i < num_diffs; i++)
	free (diffs[i].tests);
    free (diffs);
    free (tests);
}

static void
usage (const char *argv0)
{
    char const *basename = strrchr(argv0, '/');
    basename = basename ? basename+1 : argv0;
    fprintf (stderr,
	     "Usage: %s [options] file1 file2 [...]\n\n",
	     basename);
    fprintf (stderr,
	     "Computes significant performance differences for cairo performance reports.\n"
	     "Each file should be the output of the cairo-perf program (or \"make perf\").\n"
	     "The following options are available:\n"
	     "\n"
	     "--no-utf    Use ascii stars instead of utf-8 change bars.\n"
	     "            Four stars are printed per factor of speedup.\n"
	     "\n"
	     "--no-bars   Don't display change bars at all.\n\n"
	     "\n"
	     "--use-ms    Use milliseconds to calculate differences.\n"
	     "            (instead of ticks which are hardware dependent)\n"
	     "\n"
	     "--min-change threshold[%%]\n"
	     "            Suppress all changes below the given threshold.\n"
	     "            The default threshold of 0.05 or 5%% ignores any\n"
	     "            speedup or slowdown of 1.05 or less. A threshold\n"
	     "            of 0 will cause all output to be reported.\n"
	);
    exit(1);
}

static void
parse_args (int 			   argc,
	    char const			 **argv,
	    cairo_perf_diff_files_args_t  *args)
{
    int i;

    for (i = 1; i < argc; i++) {
	if (strcmp (argv[i], "--no-utf") == 0) {
	    args->options.use_utf = 0;
	}
	else if (strcmp (argv[i], "--no-bars") == 0) {
	    args->options.print_change_bars = 0;
	}
	else if (strcmp (argv[i], "--use-ms") == 0) {
	    /* default */
	}
	else if (strcmp (argv[i], "--use-ticks") == 0) {
	    args->options.use_ticks = 1;
	}
	else if (strcmp (argv[i], "--min-change") == 0) {
	    char *end = NULL;
	    i++;
	    if (i >= argc)
		usage (argv[0]);
	    args->options.min_change = strtod (argv[i], &end);
	    if (*end) {
		if (*end == '%') {
		    args->options.min_change /= 100;
		} else {
		    usage (argv[0]);
		}
	    }
	}
	else {
	    args->num_filenames++;
	    args->filenames = xrealloc (args->filenames,
					args->num_filenames * sizeof (char *));
	    args->filenames[args->num_filenames - 1] = argv[i];
	}
    }
}

int
main (int	  argc,
      const char *argv[])
{
    cairo_perf_diff_files_args_t args = {
	NULL,			/* filenames */
	0,			/* num_filenames */
	{
	    0.05,		/* min change */
	    1,			/* use UTF-8? */
	    1,			/* display change bars? */
	}
    };
    cairo_perf_report_t *reports;
    test_report_t *t;
    int i;

    parse_args (argc, argv, &args);

    if (args.num_filenames < 2)
	usage (argv[0]);

    reports = xmalloc (args.num_filenames * sizeof (cairo_perf_report_t));

    for (i = 0; i < args.num_filenames; i++ ) {
	cairo_perf_report_load (&reports[i], args.filenames[i], i, NULL);
	printf ("[%d] %s\n", i, args.filenames[i]);
    }
    printf ("\n");

    cairo_perf_reports_compare (reports, args.num_filenames, &args.options);

    /* Pointless memory cleanup, (would be a great place for talloc) */
    free (args.filenames);
    for (i = 0; i < args.num_filenames; i++) {
	for (t = reports[i].tests; t->name; t++) {
	    free (t->samples);
	    free (t->backend);
	    free (t->name);
	}
	free (reports[i].tests);
	free (reports[i].configuration);
    }
    free (reports);

    return 0;
}
