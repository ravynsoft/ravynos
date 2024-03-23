/*
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
 * Authors: Carl Worth <cworth@cworth.org>
 */

#include "cairo-stats.h"

#include <assert.h>

void
_cairo_stats_compute (cairo_stats_t *stats,
		      cairo_time_t  *values,
		      int	     num_values)
{
    cairo_time_t sum, mean, q1, q3, iqr;
    cairo_time_t outlier_min, outlier_max;
    int i, min_valid, num_valid;
    double s;

    assert (num_values > 0);

    if (num_values == 1) {
	stats->min_ticks = stats->median_ticks = values[0];
	stats->std_dev = 0;
	stats->iterations = 1;
	stats->values = values;
	return;
    }

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
    num_valid = num_values;
    do {
	num_values = num_valid;
	qsort (values, num_values, sizeof (cairo_time_t), _cairo_time_cmp);

	q1 = values[1*num_values/4];
	q3 = values[3*num_values/4];

	/* XXX assumes we have native uint64_t */
	iqr = q3 - q1;
	outlier_min = q1 - 3 * iqr / 2;
	outlier_max = q3 + 3 * iqr / 2;

	for (i = 0; i < num_values && values[i] < outlier_min; i++)
	    ;
	min_valid = i;

	for (i = 0; i < num_values && values[i] <= outlier_max; i++)
	    ;
	num_valid = i - min_valid;
	assert(num_valid);
	values += min_valid;
    } while (num_valid != num_values);

    stats->values = values;
    stats->iterations = num_valid;
    stats->min_ticks = values[0];
    stats->median_ticks = values[num_valid / 2];

    sum = 0;
    for (i = 0; i < num_valid; i++)
	sum = _cairo_time_add (sum, values[i]);
    mean = sum / num_valid;

    /* Let's use a normalized std. deviation for easier comparison. */
    s = 0;
    for (i = 0; i < num_valid; i++) {
	double delta = (values[i] - mean) / (double)mean;
	s += delta * delta;
    }
    stats->std_dev = sqrt(s / num_valid);
}

cairo_bool_t
_cairo_histogram_init (cairo_histogram_t *h,
		       int width, int height)
{
    h->width = width;
    h->height = height;
    if (h->width < 2 || h->height < 1)
	return FALSE;

    h->num_columns = width - 2;
    h->num_rows = height - 1;
    h->columns = malloc (sizeof(int)*h->num_columns);
    return h->columns != NULL;
}

cairo_bool_t
_cairo_histogram_compute (cairo_histogram_t *h,
			  const cairo_time_t *values,
			  int num_values)
{
    cairo_time_t delta;
    int i;

    if (num_values == 0)
	return FALSE;

    h->min_value = values[0];
    h->max_value = values[0];

    for (i = 1; i < num_values; i++) {
	if (values[i] < h->min_value)
	    h->min_value = values[i];
	if (values[i] > h->max_value)
	    h->max_value = values[i];
    }

    delta = h->max_value - h->min_value;
    if (delta == 0)
	return FALSE;

    memset(h->columns, 0, sizeof(int)*h->num_columns);
    h->max_count = 0;

    for (i = 0; i < num_values; i++) {
	int count = h->columns[(values[i] - h->min_value) * (h->num_columns - 1) / delta]++;
	if (count > h->max_count)
	    h->max_count = count;
    }

    return TRUE;
}

void
_cairo_histogram_printf (cairo_histogram_t *h,
			 FILE *file)
{
    int x, y, num_rows;

    num_rows = h->num_rows;
    if (h->max_count < num_rows)
	num_rows = h->max_count;
    for (y = 0; y < num_rows; y++) {
	int min_count = ((num_rows - y - 1) * h->max_count) / num_rows + h->max_count / (2*num_rows);
	fprintf (file, "|");
	for (x = 0; x < h->num_columns; x++)
	    fprintf (file, "%c", h->columns[x] > min_count ? 'x' : ' ');
	fprintf (file, "|\n");
    }

    fprintf(file, ".");
    for (x = 0; x < h->num_columns; x++)
	fprintf (file, "-");
    fprintf (file, ".\n");
}

void
_cairo_histogram_fini (cairo_histogram_t *h)
{
    free(h->columns);
}
