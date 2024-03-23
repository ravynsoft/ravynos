#include "config.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <math.h>

typedef struct _point {
    gdouble x, y;
} point_t;
typedef struct _box {
    point_t p1, p2;
} box_t;
typedef struct _line {
    point_t p1, p2;
} line_t;

typedef struct _edge {
    gulong id;
    line_t line;
    gdouble top, bottom;
    point_t p1, p2;
    int dir;
} edge_t;
typedef struct _trapezoid {
    gdouble top, bottom;
    const edge_t *left, *right;
} trapezoid_t;
typedef struct _traps {
    int num_traps;
    int size;
    trapezoid_t traps[0];
} traps_t;

typedef struct _edges {
    GHashTable *ht;

    int num_edges;
    int size;
    edge_t edges[0];
} edges_t;

typedef struct _event {
    enum {
	START_EDGE,
	END_EDGE,
	INTERSECTION,
	START_TRAP,
	END_TRAP,
    } type;

    int x, y; /* (top, bottom) for trap */
    long e1, e2;
} event_t;

typedef struct _events {
    struct _events *prev, *next;

    box_t extents;
    edges_t *edges;
    traps_t *prototraps;
    traps_t *traps;

    int current_event;
    int num_events;
    int size_events;
    event_t *events;
} events_t;

typedef struct _EventView {
    GtkWidget widget;

    events_t *events_list;
    events_t *current_events;

    double px, py;

    gint mag_x, mag_y;
    gint mag_size;
    gdouble mag_zoom;
    gboolean in_mag_drag;
    gint mag_drag_x, mag_drag_y;
} EventView;

typedef struct _EventViewClass {
    GtkWidgetClass parent_class;
} EventViewClass;

G_DEFINE_TYPE (EventView, event_view, GTK_TYPE_WIDGET)

static edge_t *
edges_lookup (edges_t *edges, gulong id)
{
    return &edges->edges[GPOINTER_TO_UINT(g_hash_table_lookup (edges->ht,
							       GUINT_TO_POINTER
							       (id)))];
}

static gdouble
_compute_intersection_x_for_y (const line_t *line,
			       gdouble y)
{
    gdouble dx = line->p2.x - line->p1.x;
    gdouble dy = line->p2.y - line->p1.y;
    gdouble x;

    if (y == line->p1.y)
	return line->p1.x;
    if (y == line->p2.y)
	return line->p2.x;

    x = line->p1.x;
    if (dy != 0)
	x +=  (y - line->p1.y)*dx/dy;
    return x;
}

static void
_compute_intersection_point (const line_t *line,
			     gdouble y,
			     point_t *p)
{
    p->x = _compute_intersection_x_for_y (line, p->y = y);
}

static void
_edge_path (cairo_t *cr, const cairo_matrix_t *m, const edge_t *e)
{
    double x, y;

    x = e->p1.x; y = e->p1.y;
    cairo_matrix_transform_point (m, &x, &y);
    cairo_move_to (cr, x, y);

    x = e->p2.x; y = e->p2.y;
    cairo_matrix_transform_point (m, &x, &y);
    cairo_line_to (cr, x, y);

    if (e->dir < 0) {
	cairo_set_source_rgb (cr, 0, 0, 1);
    } else {
	cairo_set_source_rgb (cr, 1, 0, 0);
    }
}

static void
_events_draw (events_t *events, cairo_t *cr, cairo_matrix_t *m)
{
    double dash[2] = {8, 8};
    point_t p;
    int n;

    /* first the existing and proto-traps */
    cairo_save (cr); {
	cairo_set_matrix (cr, m);

	cairo_set_source_rgba (cr, 1, 0, 0, .15);
	for (n = 0; n < events->prototraps->num_traps; n++) {
	    const trapezoid_t *t = &events->prototraps->traps[n];

	    _compute_intersection_point (&t->left->line, t->top, &p);
	    cairo_move_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->right->line, t->top, &p);
	    cairo_line_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->right->line, t->bottom, &p);
	    cairo_line_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->left->line, t->bottom, &p);
	    cairo_line_to (cr, p.x, p.y);
	    cairo_close_path (cr);
	    cairo_fill (cr);
	}

	cairo_set_source_rgba (cr, 0, 1, 0, .2);
	for (n = 0; n < events->traps->num_traps; n++) {
	    const trapezoid_t *t = &events->traps->traps[n];

	    _compute_intersection_point (&t->left->line, t->top, &p);
	    cairo_move_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->right->line, t->top, &p);
	    cairo_line_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->right->line, t->bottom, &p);
	    cairo_line_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->left->line, t->bottom, &p);
	    cairo_line_to (cr, p.x, p.y);
	    cairo_close_path (cr);
	    cairo_fill (cr);
	}
    } cairo_restore (cr);

    /* known edges */
    cairo_save (cr);
    cairo_set_line_width (cr, 1.);
    for (n = 0; n < events->edges->num_edges; n++) {
	const edge_t *e = &events->edges->edges[n];
	double x, y;

	x = e->p1.x; y = e->p1.y;
	cairo_matrix_transform_point (m, &x, &y);
	cairo_move_to (cr, x, y);

	x = e->p2.x; y = e->p2.y;
	cairo_matrix_transform_point (m, &x, &y);
	cairo_line_to (cr, x, y);

	if (e->dir < 0) {
	    cairo_set_source_rgba (cr, 0, 0, 1., .4);
	    cairo_set_dash (cr, dash, 2, fmod (e->p1.x, dash[0]+dash[1]) + dash[0]);
	} else {
	    cairo_set_source_rgba (cr, 1, 0, 0., 4);
	    cairo_set_dash (cr, dash, 2, fmod (e->p1.x, dash[0]+dash[1]));
	}

	cairo_stroke (cr);

	x = e->p1.x; y = e->p1.y;
	cairo_matrix_transform_point (m, &x, &y);
	cairo_arc (cr, x, y, 2., 0, 2 * G_PI);

	x = e->p2.x; y = e->p2.y;
	cairo_matrix_transform_point (m, &x, &y);
	cairo_arc (cr, x, y, 2., 0, 2 * G_PI);

	cairo_fill (cr);
    }
    cairo_restore (cr);

    /* event time */
    cairo_save (cr); {
	event_t *e;
	double x, y;

	e = &events->events[events->current_event];

	cairo_set_line_width (cr, 2.);
	cairo_set_matrix (cr, m);
	cairo_move_to (cr,
		       events->extents.p1.x,
		       e->y);
	cairo_line_to (cr,
		       events->extents.p2.x,
		       e->y);
	cairo_identity_matrix (cr);
	cairo_stroke (cr);

	x = e->x; y = e->y;
	cairo_matrix_transform_point (m, &x, &y);
	switch (e->type) {
	case START_EDGE:
	case END_EDGE:
	case INTERSECTION:
	    cairo_arc (cr, x, y, 4., 0, 2 * G_PI);
	    break;
	case START_TRAP:
	case END_TRAP:
	    break;
	}
	switch (e->type) {
	case START_EDGE:
	    cairo_set_source_rgb (cr, 1, 0, 0);
	    break;
	case END_EDGE:
	    cairo_set_source_rgb (cr, 0, 0, 1);
	    break;
	case INTERSECTION:
	    cairo_set_source_rgb (cr, 1, 0, 1);
	    break;
	case START_TRAP:
	case END_TRAP:
	    break;
	}
	cairo_fill (cr);

	cairo_set_line_width (cr, 1.);
	switch (e->type) {
	case START_EDGE:
	    _edge_path (cr, m, edges_lookup (events->edges, e->e1));
	    cairo_stroke (cr);
	    break;
	case END_EDGE:
	    _edge_path (cr, m, edges_lookup (events->edges, e->e1));
	    cairo_stroke (cr);
	    break;
	case INTERSECTION:
	    _edge_path (cr, m, edges_lookup (events->edges, e->e1));
	    cairo_stroke (cr);
	    _edge_path (cr, m, edges_lookup (events->edges, e->e2));
	    cairo_stroke (cr);
	    break;
	}
    } cairo_restore (cr);
}

static void
event_view_draw (EventView *self, cairo_t *cr)
{
    events_t *events;
    gdouble sf_x, sf_y, sf;
    gdouble mid, dim;
    gdouble x0, x1,  y0, y1;
    cairo_matrix_t m;

    cairo_save (cr);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_restore (cr);

    events = self->current_events;
    if (events == NULL)
	return;

    mid = (events->extents.p2.x + events->extents.p1.x) / 2.;
    dim = (events->extents.p2.x - events->extents.p1.x) / 2. * 1.2;
    sf_x = self->widget.allocation.width / dim / 2;

    mid = (events->extents.p2.y + events->extents.p1.y) / 2.;
    dim = (events->extents.p2.y - events->extents.p1.y) / 2. * 1.2;
    sf_y = self->widget.allocation.height / dim / 2;

    sf = MIN (sf_x, sf_y);

    mid = (events->extents.p2.x + events->extents.p1.x) / 2.;
    dim = sf_x / sf * (events->extents.p2.x - events->extents.p1.x) / 2. * 1.2;
    x0 = mid - dim;
    x1 = mid + dim;
    mid = (events->extents.p2.y + events->extents.p1.y) / 2.;
    dim = sf_y / sf * (events->extents.p2.y - events->extents.p1.y) / 2. * 1.2;
    y0 = mid - dim;
    y1 = mid + dim;

    cairo_matrix_init_scale (&m, sf, sf);
    cairo_matrix_translate (&m, -x0, -y0);
    _events_draw (events, cr, &m);

    /* draw a zoom view of the area around the mouse */
    cairo_save (cr); {
	double zoom = self->mag_zoom;
	int size = self->mag_size;

	/* bottom right */
	cairo_rectangle (cr, self->mag_x, self->mag_y, size, size);
	cairo_stroke_preserve (cr);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_fill_preserve (cr);
	cairo_clip (cr);

	/* compute roi in extents */
	cairo_translate (cr, self->mag_x + size/2, self->mag_y + size/2);

	cairo_matrix_init_scale (&m, zoom, zoom);
	cairo_matrix_translate (&m, -(self->px / sf + x0), -(self->py /sf + y0));
	_events_draw (events, cr, &m);

	/* grid */
	cairo_save (cr); {
	    int i;

	    cairo_translate (cr,
			     -zoom*fmod (self->px/sf + x0, 1.),
			     -zoom*fmod (self->py/sf + y0, 1.));
	    for (i = -size/2/zoom - 1; i <= size/2/zoom + 1; i++) {
		cairo_move_to (cr, zoom*i, -size/2);
		cairo_line_to (cr, zoom*i, size/2 + zoom);
		cairo_move_to (cr, -size/2, zoom*i);
		cairo_line_to (cr, size/2 + zoom, zoom*i);
	    }
	    cairo_set_source_rgba (cr, .7, .7, .7, .5);
	    cairo_set_line_width (cr, 1.);
	    cairo_stroke (cr);
	} cairo_restore (cr);
    } cairo_restore (cr);
}

static void
event_view_draw_labels (EventView *self, cairo_t *cr)
{
    events_t *events;

    events = self->current_events;
    if (events == NULL)
	return;

}

static gboolean
event_view_expose (GtkWidget *w, GdkEventExpose *ev)
{
    EventView *self = (EventView *) w;
    cairo_t *cr;

    cr = gdk_cairo_create (w->window);
    gdk_cairo_region (cr, ev->region);
    cairo_clip (cr);

    event_view_draw (self, cr);
    event_view_draw_labels (self, cr);

    cairo_destroy (cr);
    return FALSE;
}

static void
traps_clear (traps_t *traps)
{
    traps->num_traps = 0;
}

static traps_t *
traps_add (traps_t *traps, int top, int bot, const edge_t *e1, const edge_t *e2)
{
    trapezoid_t *t;

    if (traps->num_traps == traps->size) {
	traps->size *= 2;
	traps = g_realloc (traps,
			   sizeof (traps_t) + traps->size*sizeof (trapezoid_t));
    }

    t = &traps->traps[traps->num_traps++];
    t->top = top;
    if (bot > e1->bottom)
	bot = e1->bottom;
    if (bot > e2->bottom)
	bot = e2->bottom;
    t->bottom = bot;

    t->left = e1;
    t->right = e2;

    return traps;
}

static void
traps_remove (traps_t *traps, int top, const edge_t *e1, const edge_t *e2)
{
    int n;

    for (n = 0; n < traps->num_traps; n++) {
	trapezoid_t *t = &traps->traps[n];
	if (t->top == top && t->left == e1 && t->right == e2)
	    break;
    }
    if (n < traps->num_traps) {
	g_memmove (&traps->traps[n],
		   &traps->traps[n+1],
		   (traps->num_traps-n+1) * sizeof (trapezoid_t));
	traps->num_traps--;
    }
}

static void
event_next (EventView *self)
{
    events_t *events;
    event_t *e;

    events = self->current_events;
    if (++events->current_event == events->num_events) {
	return;
    } else if (events->current_event >= events->num_events) {
	traps_clear (events->prototraps);
	traps_clear (events->traps);
	events->current_event = 0;

	self->current_events = events->next;
	if (self->current_events == NULL)
	    self->current_events = self->events_list;
	events = self->current_events;
    }

    e = &events->events[events->current_event];
    switch (e->type) {
    case START_TRAP:
	events->prototraps = traps_add (events->prototraps,
					e->x, G_MAXINT,
					edges_lookup (events->edges, e->e1),
					edges_lookup (events->edges, e->e2));
	break;
    case END_TRAP:
	traps_remove (events->prototraps,
		      e->x,
		      edges_lookup (events->edges, e->e1),
		      edges_lookup (events->edges, e->e2));
	events->traps = traps_add (events->traps,
				   e->x, e->y,
				   edges_lookup (events->edges, e->e1),
				   edges_lookup (events->edges, e->e2));
	break;
    }
}

static gboolean
event_view_button_press (GtkWidget *w, GdkEventButton *ev)
{
    EventView *self = (EventView *) w;

    if (ev->x < self->mag_x ||
	ev->y < self->mag_y ||
	ev->x > self->mag_x + self->mag_size ||
	ev->y > self->mag_y + self->mag_size)
    {
	if (ev->type == GDK_BUTTON_PRESS) {
	    event_next (self);
	    gtk_widget_queue_draw (w);
	}
    }
    else
    {
	self->in_mag_drag = TRUE;
	self->mag_drag_x = ev->x;
	self->mag_drag_y = ev->y;
    }

    return FALSE;
}

static gboolean
event_view_button_release (GtkWidget *w, GdkEventButton *ev)
{
    EventView *self = (EventView *) w;

    self->in_mag_drag = FALSE;

    return FALSE;
}

static gboolean
event_view_motion (GtkWidget *w, GdkEventMotion *ev)
{
    EventView *self = (EventView *) w;

    if (self->in_mag_drag) {
	self->mag_x += ev->x - self->mag_drag_x;
	self->mag_y += ev->y - self->mag_drag_y;

	gtk_widget_queue_draw (&self->widget);

	self->mag_drag_x = ev->x;
	self->mag_drag_y = ev->y;
    } else if (ev->x < self->mag_x ||
	       ev->y < self->mag_y ||
	       ev->x > self->mag_x + self->mag_size ||
	       ev->y > self->mag_y + self->mag_size)
    {
	self->px = ev->x;
	self->py = ev->y;

	gtk_widget_queue_draw (&self->widget);
    }

    return FALSE;
}

static void
event_view_realize (GtkWidget *widget)
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
	                    GDK_KEY_PRESS_MASK |
	                    GDK_KEY_RELEASE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_BUTTON_MOTION_MASK |
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
event_view_size_allocate (GtkWidget *w, GdkRectangle *r)
{
    EventView *self = (EventView *) w;

    GTK_WIDGET_CLASS (event_view_parent_class)->size_allocate (w, r);

    self->mag_x = w->allocation.width - self->mag_size - 10;
    self->mag_y = w->allocation.height - self->mag_size - 10;
}

static void
event_view_finalize (GObject *obj)
{
    G_OBJECT_CLASS (event_view_parent_class)->finalize (obj);
}

static void
event_view_class_init (EventViewClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

    object_class->finalize = event_view_finalize;

    widget_class->realize = event_view_realize;
    widget_class->size_allocate = event_view_size_allocate;
    widget_class->expose_event = event_view_expose;
    widget_class->button_press_event = event_view_button_press;
    widget_class->button_release_event = event_view_button_release;
    widget_class->motion_notify_event = event_view_motion;
}

static void
event_view_init (EventView *self)
{
    self->mag_zoom = 10;
    self->mag_size = 200;
}

static traps_t *
traps_new (void)
{
    traps_t *t;

    t = g_malloc (sizeof (traps_t) + 16 * sizeof (trapezoid_t));

    t->size = 16;
    t->num_traps = 0;

    return t;
}

static edges_t *
_edges_add_edge (edges_t *edges, edge_t *e, box_t *extents)
{
    if (e->top < extents->p1.y)
	extents->p1.y = e->top;
    if (e->bottom > extents->p2.y)
	extents->p2.y = e->bottom;

    _compute_intersection_point (&e->line, e->top, &e->p1);
    _compute_intersection_point (&e->line, e->bottom, &e->p2);

    if (e->p1.x < extents->p1.x)
	extents->p1.x = e->p1.x;
    if (e->p2.x < extents->p1.x)
	extents->p1.x = e->p2.x;

    if (e->p1.x > extents->p2.x)
	extents->p2.x = e->p1.x;
    if (e->p2.x > extents->p2.x)
	extents->p2.x = e->p2.x;

    if (edges->num_edges == edges->size) {
	edges->size *= 2;
	edges = g_realloc (edges,
			   sizeof (edges_t) + edges->size * sizeof (edge_t));
    }

    g_hash_table_insert (edges->ht,
			 GUINT_TO_POINTER (e->id),
			 GUINT_TO_POINTER (edges->num_edges));
    edges->edges[edges->num_edges++] = *e;

    return edges;
}

static void
_events_add_event (events_t *events,
		   int type,
		   int x, int y,
		   gulong e1, gulong e2)
{
    event_t *e;

    if (events->num_events == events->size_events) {
	int newsize = 2 * events->size_events;
	void *newevents;

	newevents = g_renew (event_t, events->events, newsize);
	events->events = newevents;
	events->size_events = newsize;
    }

    e = &events->events[events->num_events++];
    e->type = type;
    e->x = x;
    e->y = y;
    e->e1 = e1;
    e->e2 = e2;
}

static edges_t *
edges_new (void)
{
    edges_t *t;

    t = g_malloc (sizeof (edges_t) + 16 * sizeof (edge_t));
    t->ht = g_hash_table_new (NULL, NULL);
    t->size = 16;
    t->num_edges = 0;

    return t;
}

static events_t *
events_new (void)
{
    events_t *events;

    events = g_malloc (sizeof (events_t));

    events->next = NULL;
    events->prev = NULL;

    events->events = g_new (event_t, 16);
    events->size_events = 16;
    events->num_events = 0;
    events->current_event = 0;

    events->edges = edges_new ();
    events->prototraps = traps_new ();
    events->traps = traps_new ();

    events->extents.p1.x = G_MAXDOUBLE;
    events->extents.p1.y = G_MAXDOUBLE;
    events->extents.p2.x = -G_MAXDOUBLE;
    events->extents.p2.y = -G_MAXDOUBLE;

    return events;
}

static void
events_read (EventView *ev, const char *filename)
{
    FILE *file;

    file = fopen (filename, "r");
    if (file != NULL) {
	char *line = NULL;
	size_t len = 0;
	events_t *events;

	events = ev->events_list = events_new ();
	while (getline (&line, &len, file) != -1) {
	    line = g_strstrip (line);
	    if (*line == '\0') {
		events->next = events_new ();
		events->next->prev = events;
		events = events->next;
	    } else if (g_str_has_prefix (line, "edge:")) {
		edge_t edge;

		sscanf (line, "edge: %lu (%lf, %lf) (%lf, %lf) (%lf, %lf) %d",
			&edge.id,
			&edge.line.p1.x,
			&edge.line.p1.y,
			&edge.line.p2.x,
			&edge.line.p2.y,
			&edge.top,
			&edge.bottom,
			&edge.dir);

		events->edges = _edges_add_edge (events->edges,
						 &edge,
						 &events->extents);
	    } else if (g_str_has_prefix (line, "event:")) {
		int type;
		int x,y;
		gulong e1, e2;

		sscanf (line, "event: %d (%d, %d) %lu %lu",
			&type, &x, &y,
			&e1, &e2);

		_events_add_event (events, type, x, y, e1, e2);
	    } else if (g_str_has_prefix (line, "begin trap:")) {
		int top;
		gulong e1, e2;

		sscanf (line, "begin trap: %lu %lu %u", &e1, &e2, &top);

		_events_add_event (events, START_TRAP, top, 0, e1, e2);
	    } else if (g_str_has_prefix (line, "end trap:")) {
		int top, bottom;
		gulong e1, e2;

		sscanf (line, "end trap: %lu %lu %d %d",
			&e1, &e2, &top, &bottom);

		_events_add_event (events, END_TRAP, top, bottom, e1, e2);
	    }
	}

	ev->current_events = ev->events_list;

	free (line);
	fclose (file);
    }
}

static gboolean
timeout_advance (EventView *self)
{
    event_next (self);
    gtk_widget_queue_draw (&self->widget);
    return TRUE;
}

int
main (int argc, char **argv)
{
    EventView *ev;
    GtkWidget *window, *hbox;

    gtk_init (&argc, &argv);

    hbox = gtk_hbox_new (TRUE, 0);

    ev = g_object_new (event_view_get_type (), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), &ev->widget, TRUE, TRUE, 0);
    gtk_widget_show (&ev->widget);

    events_read (ev, argv[1]);
    g_timeout_add (750, (GSourceFunc) timeout_advance, ev);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 800, 800);
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_add (GTK_CONTAINER (window), hbox);
    gtk_widget_show (hbox);
    gtk_widget_show (window);

    gtk_main ();
    return 0;
}
