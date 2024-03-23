/*
 * Copyright © 2008 Chris Wilson
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
 * Authors: Chris Wilson <chris@chris-wilson.co.uk>
 */

#ifndef CAIRO_PERF_GRAPH_H
#define CAIRO_PERF_GRAPH_H

#include <gtk/gtk.h>

#include "cairo-perf.h"

typedef struct _test_case {
    const char *backend;
    const char *content;
    const char *name;
    int size;

    test_report_t *min_test;

    cairo_bool_t shown;
    double baseline;
    double min, max;
    GdkColor color;
} test_case_t;

typedef struct _GraphView GraphView;

GtkWidget *
graph_view_new (void);

void
graph_view_set_reports (GraphView	    *gv,
			test_case_t	    *tests,
			cairo_perf_report_t *reports,
			int		     num_reports);

void
graph_view_update_visible (GraphView *gv);

#endif
