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

#ifndef _CAIRO_STATS_H_
#define _CAIRO_STATS_H_

#include "cairo-perf.h"

void
_cairo_stats_compute (cairo_stats_t *stats,
		      cairo_time_t  *values,
		      int	     num_values);

cairo_bool_t
_cairo_histogram_init (cairo_histogram_t *h,
		       int width, int height);

cairo_bool_t
_cairo_histogram_compute (cairo_histogram_t *h,
			  const cairo_time_t  *values,
			  int num_values);

void
_cairo_histogram_printf (cairo_histogram_t *h,
			 FILE *file);

void
_cairo_histogram_fini (cairo_histogram_t *h);

#endif /* _CAIRO_STATS_H_ */
