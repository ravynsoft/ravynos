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

#include "config.h"

#include "cairo-perf.h"
#include "cairo-stats.h"

#include <stdio.h>

#if HAVE_UNISTD_H && HAVE_SYS_IOCTL_H
#define USE_TERMINAL_SIZE 1
#else
#define USE_TERMINAL_SIZE 0
#endif

#if USE_TERMINAL_SIZE
#include <unistd.h>
#include <sys/ioctl.h>
#endif

static void
report_print (const cairo_perf_report_t *report,
	      int show_histogram)
{
    const test_report_t *test;
    cairo_histogram_t h;

    if (show_histogram) {
	int num_rows = 23;
	int num_cols = 80;

#if USE_TERMINAL_SIZE
	int fd = fileno(stdout);
	if (isatty(fd)) {
	    struct winsize ws;

	    if(ioctl(fd, TIOCGWINSZ, &ws) == 0 ) {
		num_rows = ws.ws_row - 1;
		num_cols = ws.ws_col;
	    }
	}
#endif

	if (!_cairo_histogram_init (&h, num_cols, num_rows))
	    show_histogram = 0;
    }

    for (test = report->tests; test->name != NULL; test++) {
	if (test->stats.iterations == 0)
	    continue;

	if (show_histogram) {
	    const cairo_time_t *values;
	    int num_values;

	    if (show_histogram > 1) {
		values = test->stats.values;
		num_values = test->stats.iterations;
	    } else {
		values = test->samples;
		num_values = test->samples_count;
	    }

	    if (_cairo_histogram_compute (&h, values, num_values))
		_cairo_histogram_printf (&h, stdout);
	}

	if (test->size) {
	    printf ("%5s-%-4s %26s-%-3d  ",
		    test->backend, test->content,
		    test->name, test->size);
	} else {
	    printf ("%5s %26s  ", test->backend, test->name);
	}
	printf("%6.2f %4.2f%% (%d/%d)\n",
	       test->stats.median_ticks / test->stats.ticks_per_ms,
	       test->stats.std_dev * 100,
	       test->stats.iterations, test->samples_count);
    }

    if (show_histogram)
	_cairo_histogram_fini (&h);
}

int
main (int	  argc,
      const char *argv[])
{
    cairo_bool_t show_histogram = 0;
    int i;

    for (i = 1; i < argc; i++ ) {
	cairo_perf_report_t report;

	if (strcmp(argv[i], "--histogram") == 0) {
	    show_histogram = 1;
	    continue;
	}

	if (strcmp(argv[i], "--short-histogram") == 0) {
	    show_histogram = 2;
	    continue;
	}

	cairo_perf_report_load (&report, argv[i], i, NULL);
	report_print (&report, show_histogram);
    }

    return 0;
}
