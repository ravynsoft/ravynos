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

typedef struct _cairo_perf_report_options {
    double min_change;
    int use_utf;
    int print_change_bars;
} cairo_perf_report_options_t;

typedef struct _cairo_perf_diff_files_args {
    const char **filenames;
    int num_filenames;
    cairo_perf_report_options_t options;
} cairo_perf_diff_files_args_t;

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
}

static void
test_diff_print (test_diff_t		     *diff,
		 double 		      max_change,
		 cairo_perf_report_options_t *options)
{
    int i;
    double test_time;
    double change;

    if (diff->tests[0]->size != 0) {
	printf ("(%s, size: %d)\n",
		diff->tests[0]->name,
		diff->tests[0]->size);
    } else {
	printf ("(%s)\n", diff->tests[0]->name);
    }

    for (i = 0; i < diff->num_tests; i++) {
	test_time = diff->tests[i]->stats.min_ticks;
	test_time /= diff->tests[i]->stats.ticks_per_ms;
	change = diff->max / test_time;
	printf ("%8s-%s-%s\t%6.2f: %5.2fx ",
		diff->tests[i]->backend,
		diff->tests[i]->content,
		diff->tests[i]->configuration,
		diff->tests[i]->stats.min_ticks / diff->tests[i]->stats.ticks_per_ms,
		change);

	if (options->print_change_bars)
	    print_change_bar (change, max_change, options->use_utf);
	printf ("\n");
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
	int num_tests;

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
	if (! seen_non_null)
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
	    if (tests[i]->name && test_report_cmp_name (tests[i], min_test) < 0)
		min_test = tests[i];
	}

	num_tests = 0;
	for (i = 0; i < num_reports; i++) {
	    test_report_t *test;
	    int n = 0;

	    test = tests[i];
	    while (test[n].name &&
		    test_report_cmp_name (&test[n], min_test) == 0)
	    {
		n++;
	    }

	    num_tests += n;
	}

	/* For each report that has the current test, record it into
	 * the diff structure. */
	diff->num_tests = 0;
	diff->tests = xmalloc (num_tests * sizeof (test_diff_t));
	for (i = 0; i < num_reports; i++) {
	    while (tests[i]->name &&
		    test_report_cmp_name (tests[i], min_test) == 0)
	    {
		test_time = tests[i]->stats.min_ticks;
		if (test_time > 0) {
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
		}
		tests[i]++;
	    }
	}
	diff->change = diff->max / diff->min;

	diff++;
	num_diffs++;
    }
    if (num_diffs == 0)
	goto DONE;

    qsort (diffs, num_diffs, sizeof (test_diff_t), test_diff_cmp);

    max_change = 1.0;
    for (i = 0; i < num_diffs; i++) {
	if (fabs (diffs[i].change) > max_change)
	    max_change = fabs (diffs[i].change);
    }

    for (i = 0; i < num_diffs; i++) {
	diff = &diffs[i];

	/* Discard as uninteresting a change which is less than the
	 * minimum change required, (default may be overridden on
	 * command-line). */
	if (fabs (diff->change) - 1.0 < options->min_change)
	    continue;

	test_diff_print (diff, max_change, options);
    }

    for (i = 0; i < num_diffs; i++)
	free (diffs[i].tests);
 DONE:
    free (diffs);
    free (tests);
}

static void
usage (const char *argv0)
{
    char const *basename = strrchr(argv0, '/');
    basename = basename ? basename+1 : argv0;
    fprintf (stderr,
	     "Usage: %s [options] file [...]\n\n",
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

    if (args.num_filenames) {
	reports = xcalloc (args.num_filenames, sizeof (cairo_perf_report_t));
	for (i = 0; i < args.num_filenames; i++) {
	    cairo_perf_report_load (&reports[i], args.filenames[i], i,
				    test_report_cmp_name);
	    printf ("loaded: %s, %d tests\n",
		    args.filenames[i], reports[i].tests_count);
	}
    } else {
	args.num_filenames = 1;
	reports = xcalloc (args.num_filenames, sizeof (cairo_perf_report_t));
	cairo_perf_report_load (&reports[0], NULL, 0, test_report_cmp_name);
    }

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
