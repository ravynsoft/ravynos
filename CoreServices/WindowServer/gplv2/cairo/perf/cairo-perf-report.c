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

#include "config.h"

#define _GETDELIM 1/* for getline() on AIX */

#include "cairo-perf.h"
#include "cairo-missing.h"
#include "cairo-stats.h"
#include "cairo-ctype-inline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifdef _MSC_VER
#if _MSC_VER < 1800
static long long
strtoll (const char  *nptr,
	 char	    **endptr,
	 int	      base);
#endif

static char *
basename (char *path);
#endif

/* Ad-hoc parsing, macros with a strong dependence on the calling
 * context, and plenty of other ugliness is here.  But at least it's
 * not perl... */
#define parse_error(...) fprintf(stderr, __VA_ARGS__); return TEST_REPORT_STATUS_ERROR;
#define skip_char(c)							\
do {									\
    if (*s && *s == (c)) {						\
	s++;								\
    } else {								\
	 parse_error ("expected '%c' but found '%c'", c, *s);		\
    }									\
} while (0)
#define skip_space() while (*s && (*s == ' ' || *s == '\t')) s++;
#define parse_int(result)						\
do {									\
    (result) = strtol (s, &end, 10);					\
    if (*s && end != s) {						\
	s = end;							\
    } else {								\
	parse_error("expected integer but found %s", s);		\
    }									\
} while (0)
#define parse_long_long(result) 					\
do {									\
    (result) = strtoll (s, &end, 10);					\
    if (*s && end != s) {						\
	s = end;							\
    } else {								\
	parse_error("expected integer but found %s", s);		\
    }									\
} while (0)
#define parse_double(result)						\
do {									\
    (result) = strtod (s, &end);					\
    if (*s && end != s) {						\
	s = end;							\
    } else {								\
	parse_error("expected floating-point value but found %s", s);	\
    }									\
} while (0)
/* Here a string is simply a sequence of non-whitespace */
#define parse_string(result)						\
do {									\
    for (end = s; *end; end++)						\
	if (_cairo_isspace (*end))					\
	    break;							\
    (result) = strndup (s, end - s);					\
    if ((result) == NULL) {						\
	fprintf (stderr, "Out of memory.\n");				\
	exit (1);							\
    }									\
    s = end;								\
} while (0)

static test_report_status_t
test_report_parse (test_report_t *report,
		   int fileno,
		   char 	 *line,
		   char 	 *configuration)
{
    char *end;
    char *s = line;
    cairo_bool_t is_raw = FALSE;
    double min_time, median_time;

    /* The code here looks funny unless you understand that these are
     * all macro calls, (and then the code just looks sick). */
    if (*s == '\n')
	return TEST_REPORT_STATUS_COMMENT;

    skip_char ('[');
    skip_space ();
    if (*s == '#')
	return TEST_REPORT_STATUS_COMMENT;
    if (*s == '*') {
	s++;
	is_raw = TRUE;
    } else {
	parse_int (report->id);
    }
    skip_char (']');

    skip_space ();

    report->fileno = fileno;
    report->configuration = configuration;
    parse_string (report->backend);
    end = strrchr (report->backend, '.');
    if (end)
	*end++ = '\0';
    report->content = end ? end : xstrdup ("???");

    skip_space ();

    parse_string (report->name);
    end = strrchr (report->name, '.');
    if (end)
	*end++ = '\0';
    report->size = end ? atoi (end) : 0;

    skip_space ();

    report->samples = NULL;
    report->samples_size = 0;
    report->samples_count = 0;

    if (is_raw) {
	parse_double (report->stats.ticks_per_ms);
	skip_space ();

	report->samples_size = 5;
	report->samples = xmalloc (report->samples_size * sizeof (cairo_time_t));
	report->stats.min_ticks = 0;
	do {
	    if (report->samples_count == report->samples_size) {
		report->samples_size *= 2;
		report->samples = xrealloc (report->samples,
					    report->samples_size * sizeof (cairo_time_t));
	    }
	    parse_long_long (report->samples[report->samples_count]);
	    if (report->samples_count == 0) {
		report->stats.min_ticks =
		    report->samples[report->samples_count];
	    } else if (report->stats.min_ticks >
		       report->samples[report->samples_count]){
		report->stats.min_ticks =
		    report->samples[report->samples_count];
	    }
	    report->samples_count++;
	    skip_space ();
	} while (*s && *s != '\n');
	report->stats.iterations = 0;
	if (*s) skip_char ('\n');
    } else {
	parse_double (report->stats.min_ticks);
	skip_space ();

	parse_double (min_time);
	report->stats.ticks_per_ms = report->stats.min_ticks / min_time;

	skip_space ();

	parse_double (median_time);
	report->stats.median_ticks = median_time * report->stats.ticks_per_ms;

	skip_space ();

	parse_double (report->stats.std_dev);
	report->stats.std_dev /= 100.0;
	skip_char ('%');

	skip_space ();

	parse_int (report->stats.iterations);

	skip_space ();
	skip_char ('\n');
    }

    return TEST_REPORT_STATUS_SUCCESS;
}

/* We provide hereafter a win32 implementation of the basename
 * and strtoll functions which are not available otherwise.
 * The basename function is fully compliant to its GNU specs.
 */
#ifdef _MSC_VER

#if _MSC_VER < 1800
long long
strtoll (const char  *nptr,
	 char	    **endptr,
	 int	      base)
{
    return _atoi64(nptr);
}
#endif

static char *
basename (char *path)
{
    char *end, *s;

    end = (path + strlen(path) - 1);
    while (end && (end >= path + 1) && (*end == '/')) {
	*end = '\0';
	end--;
    }

    s = strrchr(path, '/');
    if (s) {
	if (s == end) {
	    return s;
	} else {
	    return s+1;
	}
    } else {
	return path;
    }
}
#endif /* ifndef _MSC_VER */

int
test_report_cmp_backend_then_name (const void *a,
				   const void *b)
{
    const test_report_t *a_test = a;
    const test_report_t *b_test = b;

    int cmp;

    cmp = strcmp (a_test->backend, b_test->backend);
    if (cmp)
	return cmp;

    cmp = strcmp (a_test->content, b_test->content);
    if (cmp)
	return cmp;

    /* A NULL name is a list-termination marker, so force it last. */
    if (a_test->name == NULL)
	if (b_test->name == NULL)
	    return 0;
	else
	    return 1;
    else if (b_test->name == NULL)
	return -1;

    cmp = strcmp (a_test->name, b_test->name);
    if (cmp)
	return cmp;

    if (a_test->size < b_test->size)
	return -1;
    if (a_test->size > b_test->size)
	return 1;

    return 0;
}

int
test_report_cmp_name (const void *a,
		      const void *b)
{
    const test_report_t *a_test = a;
    const test_report_t *b_test = b;

    int cmp;

    /* A NULL name is a list-termination marker, so force it last. */
    if (a_test->name == NULL)
	if (b_test->name == NULL)
	    return 0;
	else
	    return 1;
    else if (b_test->name == NULL)
	return -1;

    cmp = strcmp (a_test->name, b_test->name);
    if (cmp)
	return cmp;

    if (a_test->size < b_test->size)
	return -1;
    if (a_test->size > b_test->size)
	return 1;

    return 0;
}

void
cairo_perf_report_sort_and_compute_stats (cairo_perf_report_t *report,
					  int (*cmp) (const void*, const void*))
{
    test_report_t *base, *next, *last, *t;

    if (cmp == NULL)
	cmp = test_report_cmp_backend_then_name;

    /* First we sort, since the diff needs both lists in the same
     * order */
    qsort (report->tests, report->tests_count, sizeof (test_report_t), cmp);

    /* The sorting also brings all related raw reports together so we
     * can condense them and compute the stats.
     */
    base = &report->tests[0];
    last = &report->tests[report->tests_count - 1];
    while (base <= last) {
	next = base+1;
	if (next <= last) {
	    while (next <= last &&
		   test_report_cmp_backend_then_name (base, next) == 0)
	    {
		next++;
	    }
	    if (next != base) {
		unsigned int new_samples_count = base->samples_count;
		for (t = base + 1; t < next; t++)
		    new_samples_count += t->samples_count;
		if (new_samples_count > base->samples_size) {
		    base->samples_size = new_samples_count;
		    base->samples = xrealloc (base->samples,
					      base->samples_size * sizeof (cairo_time_t));
		}
		for (t = base + 1; t < next; t++) {
		    memcpy (&base->samples[base->samples_count], t->samples,
			    t->samples_count * sizeof (cairo_time_t));
		    base->samples_count += t->samples_count;
		}
	    }
	}
	if (base->samples)
	    _cairo_stats_compute (&base->stats, base->samples, base->samples_count);
	base = next;
    }
}

void
cairo_perf_report_load (cairo_perf_report_t *report,
			const char *filename, int id,
			int (*cmp) (const void *, const void *))
{
    FILE *file;
    test_report_status_t status;
    int line_number = 0;
    char *line = NULL;
    size_t line_size = 0;
    char *configuration;
    char *dot;
    char *baseName;
    const char *name;

    name = filename;
    if (name == NULL)
	name = "stdin";

    configuration = xstrdup (name);
    baseName = basename (configuration);
    report->configuration = xstrdup (baseName);
    free (configuration);

    dot = strrchr (report->configuration, '.');
    if (dot)
	*dot = '\0';

    report->name = name;
    report->tests_size = 16;
    report->tests = xmalloc (report->tests_size * sizeof (test_report_t));
    report->tests_count = 0;
    report->fileno = id;

    if (filename == NULL) {
	file = stdin;
    } else {
	file = fopen (filename, "r");
	if (file == NULL) {
	    fprintf (stderr, "Failed to open %s: %s\n",
		     filename, strerror (errno));
	    exit (1);
	}
    }

    while (1) {
	if (report->tests_count == report->tests_size) {
	    report->tests_size *= 2;
	    report->tests = xrealloc (report->tests,
				      report->tests_size * sizeof (test_report_t));
	}

	line_number++;
	if (getline (&line, &line_size, file) == -1)
	    break;

	status = test_report_parse (&report->tests[report->tests_count],
				    id, line, report->configuration);
	if (status == TEST_REPORT_STATUS_ERROR)
	    fprintf (stderr, "Ignoring unrecognized line %d of %s:\n%s",
		     line_number, filename, line);
	if (status == TEST_REPORT_STATUS_SUCCESS)
	    report->tests_count++;
	/* Do nothing on TEST_REPORT_STATUS_COMMENT */
    }

    free (line);

    if (filename != NULL)
	fclose (file);

    cairo_perf_report_sort_and_compute_stats (report, cmp);

    /* Add one final report with a NULL name to terminate the list. */
    if (report->tests_count == report->tests_size) {
	report->tests_size *= 2;
	report->tests = xrealloc (report->tests,
				  report->tests_size * sizeof (test_report_t));
    }
    report->tests[report->tests_count].name = NULL;
}
