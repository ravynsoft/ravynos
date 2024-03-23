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

#define GLIB_DISABLE_DEPRECATION_WARNINGS

#include "cairo-perf.h"
#include "cairo-perf-graph.h"

#include <gtk/gtk.h>

struct _GraphView {
    GtkWidget widget;

    test_case_t *cases;
    cairo_perf_report_t *reports;
    int num_reports;
    double ymin, ymax;

    int selected_report;
};

typedef struct _GraphViewClass {
    GtkWidgetClass parent_class;
} GraphViewClass;

static GType graph_view_get_type (void);

enum {
    REPORT_SELECTED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE (GraphView, graph_view, GTK_TYPE_WIDGET)

static void
draw_baseline_performance (test_case_t		*cases,
			   cairo_perf_report_t	*reports,
			   int			 num_reports,
			   cairo_t		*cr,
			   const cairo_matrix_t *m)
{
    test_report_t **tests;
    double dots[2] = { 0, 1.};
    int i;

    tests = xmalloc (num_reports * sizeof (test_report_t *));
    for (i = 0; i < num_reports; i++)
	tests[i] = reports[i].tests;

    while (cases->backend != NULL) {
	test_report_t *min_test;
	double baseline, last_y;
	double x, y;

	if (! cases->shown) {
	    cases++;
	    continue;
	}

	min_test = cases->min_test;

	for (i = 0; i < num_reports; i++) {
	    while (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) < 0)
	    {
		tests[i]++;
	    }
	}

	/* first the stroke */
	cairo_save (cr);
	cairo_set_line_width (cr, 2.);
	gdk_cairo_set_source_color (cr, &cases->color);
	for (i = 0; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) == 0)
	    {
		baseline = tests[i]->stats.min_ticks;

		x = i; y = 0;
		cairo_matrix_transform_point (m, &x, &y);
		x = floor (x);
		y = floor (y);
		cairo_move_to (cr, x, y);
		last_y = y;
		break;
	    }
	}

	for (++i; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) == 0)
	    {
		x = i, y = tests[i]->stats.min_ticks / baseline;

		if (y < 1.)
		    y = -1./y + 1;
		else
		    y -= 1;

		cairo_matrix_transform_point (m, &x, &y);
		x = floor (x);
		y = floor (y);
		cairo_line_to (cr, x, last_y);
		cairo_line_to (cr, x, y);
		last_y = y;
	    }
	}
	{
	    x = num_reports, y = 0;
	    cairo_matrix_transform_point (m, &x, &y);
	    x = floor (x);
	    cairo_line_to (cr, x, last_y);
	}

	cairo_set_line_width (cr, 1.);
	cairo_stroke (cr);

	/* then draw the points */
	for (i = 0; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) == 0)
	    {
		baseline = tests[i]->stats.min_ticks;

		x = i; y = 0;
		cairo_matrix_transform_point (m, &x, &y);
		x = floor (x);
		y = floor (y);
		cairo_move_to (cr, x, y);
		cairo_close_path (cr);
		last_y = y;

		tests[i]++;
		break;
	    }
	}

	for (++i; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) == 0)
	    {
		x = i, y = tests[i]->stats.min_ticks / baseline;

		if (y < 1.)
		    y = -1./y + 1;
		else
		    y -= 1;

		cairo_matrix_transform_point (m, &x, &y);
		x = floor (x);
		y = floor (y);
		cairo_move_to (cr, x, last_y);
		cairo_close_path (cr);
		cairo_move_to (cr, x, y);
		cairo_close_path (cr);
		last_y = y;

		tests[i]++;
	    }
	}
	{
	    x = num_reports, y = 0;
	    cairo_matrix_transform_point (m, &x, &y);
	    x = floor (x);
	    cairo_move_to (cr, x, last_y);
	    cairo_close_path (cr);
	}
	cairo_set_source_rgba (cr, 0, 0, 0, .5);
	cairo_set_dash (cr, dots, 2, 0.);
	cairo_set_line_width (cr, 3.);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_stroke (cr);
	cairo_restore (cr);

	cases++;
    }
    free (tests);
}

static void
draw_hline (cairo_t		 *cr,
	    const cairo_matrix_t *m,
	    double		  y0,
	    double		  xmin,
	    double		  xmax)
{
    double x, y;
    double py_offset;

    py_offset = fmod (cairo_get_line_width (cr) / 2., 1.);

    x = xmin; y = y0;
    cairo_matrix_transform_point (m, &x, &y);
    cairo_move_to (cr, floor (x), floor (y) + py_offset);

    x = xmax; y = y0;
    cairo_matrix_transform_point (m, &x, &y);
    cairo_line_to (cr, ceil (x), floor (y) + py_offset);

    cairo_stroke (cr);
}

static void
draw_label (cairo_t		 *cr,
	    const cairo_matrix_t *m,
	    double		  y0,
	    double		  xmin,
	    double		  xmax)
{
    double x, y;
    char buf[80];
    cairo_text_extents_t extents;

    snprintf (buf, sizeof (buf), "%.0fx", fabs (y0));
    cairo_text_extents (cr, buf, &extents);

    x = xmin; y = y0;
    cairo_matrix_transform_point (m, &x, &y);
    cairo_move_to (cr,
		   x - extents.width - 4,
		   y - (extents.height/2. + extents.y_bearing));
    cairo_show_text (cr, buf);


    snprintf (buf, sizeof (buf), "%.0fx", fabs (y0));
    cairo_text_extents (cr, buf, &extents);

    x = xmax; y = y0;
    cairo_matrix_transform_point (m, &x, &y);
    cairo_move_to (cr,
		   x + 4,
		   y - (extents.height/2. + extents.y_bearing));
    cairo_show_text (cr, buf);
}

#define ALIGN_X(v) ((v)<<0)
#define ALIGN_Y(v) ((v)<<2)
static void
draw_rotated_label (cairo_t    *cr,
		    const char *text,
		    double	x,
		    double	y,
		    double	angle,
		    int 	align)
{
    cairo_text_extents_t extents;

    cairo_text_extents (cr, text, &extents);

    cairo_save (cr); {
	cairo_translate (cr, x, y);
	cairo_rotate (cr, angle);
	switch (align) {
	case ALIGN_X(0) | ALIGN_Y(0):
	    cairo_move_to (cr,
			   -extents.x_bearing,
			   -extents.y_bearing);
	    break;
	case ALIGN_X(0) | ALIGN_Y(1):
	    cairo_move_to (cr,
			   -extents.x_bearing,
			   - (extents.height/2. + extents.y_bearing));
	    break;
	case ALIGN_X(0) | ALIGN_Y(2):
	    cairo_move_to (cr,
			   -extents.x_bearing,
			   - (extents.height + extents.y_bearing));
	    break;

	case ALIGN_X(1) | ALIGN_Y(0):
	    cairo_move_to (cr,
			   - (extents.width/2. + extents.x_bearing),
			   -extents.y_bearing);
	    break;
	case ALIGN_X(1) | ALIGN_Y(1):
	    cairo_move_to (cr,
			   - (extents.width/2. + extents.x_bearing),
			   - (extents.height/2. + extents.y_bearing));
	    break;
	case ALIGN_X(1) | ALIGN_Y(2):
	    cairo_move_to (cr,
			   - (extents.width/2. + extents.x_bearing),
			   - (extents.height + extents.y_bearing));
	    break;

	case ALIGN_X(2) | ALIGN_Y(0):
	    cairo_move_to (cr,
			   - (extents.width + extents.x_bearing),
			   -extents.y_bearing);
	    break;
	case ALIGN_X(2) | ALIGN_Y(1):
	    cairo_move_to (cr,
			   - (extents.width + extents.x_bearing),
			   - (extents.height/2. + extents.y_bearing));
	    break;
	case ALIGN_X(2) | ALIGN_Y(2):
	    cairo_move_to (cr,
			   - (extents.width + extents.x_bearing),
			   - (extents.height + extents.y_bearing));
	    break;
	}
	cairo_show_text (cr, text);
    } cairo_restore (cr);
}

#define PAD 36
static void
graph_view_draw (GraphView *self,
		 cairo_t   *cr)
{
    cairo_matrix_t m;
    const double dash[2] = {4, 4};
    double range;
    int i;

    if (self->widget.allocation.width < 4 *PAD)
	return;
    if (self->widget.allocation.height < 3 *PAD)
	return;

    range = floor (self->ymax+1) - ceil (self->ymin-1);

    cairo_matrix_init_translate (&m, PAD, self->widget.allocation.height - PAD);
    cairo_matrix_scale (&m,
			(self->widget.allocation.width-2*PAD)/(self->num_reports),
			-(self->widget.allocation.height-2*PAD)/range);
    cairo_matrix_translate (&m, 0,   floor (self->ymax+1));

    if (self->selected_report != -1) {
	cairo_save (cr); {
	    double x0, x1, y;
	    x0 = self->selected_report; y = 0;
	    cairo_matrix_transform_point (&m, &x0, &y);
	    x0 = floor (x0);
	    x1 = self->selected_report + 1; y = 0;
	    cairo_matrix_transform_point (&m, &x1, &y);
	    x1 = ceil (x1);
	    y = (x1 - x0) / 8;
	    y = MIN (y, PAD / 2);
	    x0 -= y;
	    x1 += y;
	    cairo_rectangle (cr, x0, PAD/2, x1-x0, self->widget.allocation.height-2*PAD + PAD);
	    gdk_cairo_set_source_color (cr, &self->widget.style->base[GTK_STATE_SELECTED]);
	    cairo_fill (cr);
	} cairo_restore (cr);
    }

    cairo_save (cr); {
	cairo_pattern_t *linear;
	double x, y;

	gdk_cairo_set_source_color (cr,
				    &self->widget.style->fg[GTK_WIDGET_STATE (self)]);
	cairo_set_line_width (cr, 2.);
	draw_hline (cr, &m, 0, 0, self->num_reports);

	cairo_set_line_width (cr, 1.);
	cairo_set_dash (cr, NULL, 0, 0);

	for (i = ceil (self->ymin-1); i <= floor (self->ymax+1); i++) {
	    if (i != 0)
		draw_hline (cr, &m, i, 0, self->num_reports);
	}

	cairo_set_font_size (cr, 11);

	linear = cairo_pattern_create_linear (0, PAD, 0, self->widget.allocation.height-2*PAD);
	cairo_pattern_add_color_stop_rgb (linear, 0, 0, 1, 0);
	cairo_pattern_add_color_stop_rgb (linear, 1, 1, 0, 0);
	cairo_set_source (cr, linear);
	cairo_pattern_destroy (linear);

	for (i = ceil (self->ymin-1); i <= floor (self->ymax+1); i++) {
	    if (i != 0)
		draw_label (cr, &m, i, 0, self->num_reports);
	}

	x = 0, y = floor (self->ymax+1);
	cairo_matrix_transform_point (&m, &x, &y);
	draw_rotated_label (cr, "Faster", x - 7, y + 14,
			    270./360 * 2 * G_PI,
			    ALIGN_X(2) | ALIGN_Y(1));
	x = self->num_reports, y = floor (self->ymax+1);
	cairo_matrix_transform_point (&m, &x, &y);
	draw_rotated_label (cr, "Faster", x + 11, y + 14,
			    270./360 * 2 * G_PI,
			    ALIGN_X(2) | ALIGN_Y(1));

	x = 0, y = ceil (self->ymin-1);
	cairo_matrix_transform_point (&m, &x, &y);
	draw_rotated_label (cr, "Slower", x - 7, y - 14,
			    90./360 * 2 * G_PI,
			    ALIGN_X(2) | ALIGN_Y(1));
	x = self->num_reports, y = ceil (self->ymin-1);
	cairo_matrix_transform_point (&m, &x, &y);
	draw_rotated_label (cr, "Slower", x + 11, y - 14,
			    90./360 * 2 * G_PI,
			    ALIGN_X(2) | ALIGN_Y(1));
    } cairo_restore (cr);

    draw_baseline_performance (self->cases,
			       self->reports, self->num_reports,
			       cr, &m);

    cairo_save (cr); {
	cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
	cairo_set_line_width (cr, 1.);
	cairo_set_dash (cr, dash, 2, 0);
	draw_hline (cr, &m, 0, 0, self->num_reports);
    } cairo_restore (cr);
}

static gboolean
graph_view_expose (GtkWidget	  *w,
		   GdkEventExpose *ev)
{
    GraphView *self = (GraphView *) w;
    cairo_t *cr;

    cr = gdk_cairo_create (w->window);
    gdk_cairo_set_source_color (cr, &w->style->base[GTK_WIDGET_STATE (w)]);
    cairo_paint (cr);

    graph_view_draw (self, cr);

    cairo_destroy (cr);

    return FALSE;
}

static gboolean
graph_view_button_press (GtkWidget	*w,
			 GdkEventButton *ev)
{
    GraphView *self = (GraphView *) w;
    cairo_matrix_t m;
    double x,y;
    int i;

    cairo_matrix_init_translate (&m, PAD, self->widget.allocation.height-PAD);
    cairo_matrix_scale (&m, (self->widget.allocation.width-2*PAD)/self->num_reports, -(self->widget.allocation.height-2*PAD)/(self->ymax - self->ymin));
    cairo_matrix_translate (&m, 0, -self->ymin);
    cairo_matrix_invert (&m);

    x = ev->x;
    y = ev->y;
    cairo_matrix_transform_point (&m, &x, &y);

    i = floor (x);
    if (i < 0 || i >= self->num_reports)
	i = -1;

    if (i != self->selected_report) {
	self->selected_report = i;
	gtk_widget_queue_draw (w);

	g_signal_emit (w, signals[REPORT_SELECTED], 0, i);
    }

    return FALSE;
}

static gboolean
graph_view_button_release (GtkWidget	  *w,
			   GdkEventButton *ev)
{
    return FALSE;
}

static void
graph_view_realize (GtkWidget *widget)
{
    GdkWindowAttr attributes;

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width  = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.event_mask = gtk_widget_get_events (widget) |
			    GDK_BUTTON_PRESS_MASK |
			    GDK_BUTTON_RELEASE_MASK |
			    GDK_EXPOSURE_MASK;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				     &attributes,
				     GDK_WA_X | GDK_WA_Y |
				     GDK_WA_VISUAL | GDK_WA_COLORMAP);
    gdk_window_set_user_data (widget->window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
graph_view_finalize (GObject *obj)
{
    G_OBJECT_CLASS (graph_view_parent_class)->finalize (obj);
}

static void
graph_view_class_init (GraphViewClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

    object_class->finalize = graph_view_finalize;

    widget_class->realize = graph_view_realize;
    widget_class->expose_event = graph_view_expose;
    widget_class->button_press_event = graph_view_button_press;
    widget_class->button_release_event = graph_view_button_release;

    signals[REPORT_SELECTED] =
	g_signal_new ("report-selected",
		      G_TYPE_FROM_CLASS (object_class),
		      G_SIGNAL_RUN_FIRST,
		      0,//G_STRUCT_OFFSET (GraphView, report_selected),
		      NULL, NULL,
		      g_cclosure_marshal_VOID__INT,
		      G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
graph_view_init (GraphView *self)
{
    self->selected_report = -1;
}

GtkWidget *
graph_view_new (void)
{
    return g_object_new (graph_view_get_type (), NULL);
}

void
graph_view_update_visible (GraphView *gv)
{
    double min, max;
    test_case_t *cases;

    cases = gv->cases;

    min = max = 1.;
    while (cases->name != NULL) {
	if (cases->shown) {
	    if (cases->min < min)
		min = cases->min;
	    if (cases->max > max)
		max = cases->max;
	}
	cases++;
    }
    gv->ymin = -1/min + 1;
    gv->ymax = max - 1;

    gtk_widget_queue_draw (&gv->widget);
}

void
graph_view_set_reports (GraphView	    *gv,
			test_case_t	    *cases,
			cairo_perf_report_t *reports,
			int		     num_reports)
{
    /* XXX ownership? */
    gv->cases = cases;
    gv->reports = reports;
    gv->num_reports = num_reports;

    graph_view_update_visible (gv);
}
