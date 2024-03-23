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
typedef struct _edge {
    point_t p1, p2;
    gdouble top, bot;
    int dir;
} edge_t;
typedef struct _box {
    point_t p1, p2;
} box_t;

typedef struct _polygon {
    struct _polygon *next, *prev;
    int num_edges;
    int size;
    edge_t edges[0];
} polygon_t;

typedef struct _PolygonView {
    GtkWidget widget;

    cairo_surface_t *pixmap;
    int pixmap_width, pixmap_height;

    box_t extents;
    polygon_t *polygons;

    double px, py;

    gint mag_x, mag_y;
    gint mag_size;
    gdouble mag_zoom;
    gboolean in_mag_drag;
    gint mag_drag_x, mag_drag_y;
} PolygonView;

typedef struct _PolygonViewClass {
    GtkWidgetClass parent_class;
} PolygonViewClass;

G_DEFINE_TYPE (PolygonView, polygon_view, GTK_TYPE_WIDGET)

double highlight = -1;

static void draw_edges (cairo_t *cr, polygon_t *p, gdouble sf, int dir)
{
    int n;

    if (dir < 0)
	cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
    else
	cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);

    for (n = 0; n < p->num_edges; n++) {
	const edge_t *e = &p->edges[n];
	double dx, dy;
	double x1, x2;

	if (e->dir != dir)
	    continue;

	dx = e->p2.x - e->p1.x;
	dy = e->p2.y - e->p1.y;

	x1 = e->p1.x + (e->top - e->p1.y) / dy * dx;
	x2 = e->p1.x + (e->bot - e->p1.y) / dy * dx;

	cairo_arc (cr, x1, e->top, 2/sf, 0, 2*M_PI);
	cairo_arc (cr, x2, e->bot, 2/sf, 0, 2*M_PI);
	cairo_fill (cr);
    }

    if (dir < 0)
	cairo_set_source_rgba (cr, 0.0, 0.0, 1.0, 0.5);
    else
	cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 0.5);

    for (n = 0; n < p->num_edges; n++) {
	const edge_t *e = &p->edges[n];

	if (e->dir != dir)
	    continue;

	cairo_move_to (cr, e->p1.x, e->p1.y);
	cairo_line_to (cr, e->p2.x, e->p2.y);
    }
    cairo_save (cr); {
	cairo_identity_matrix (cr);
	cairo_set_line_width (cr, 1.);
	cairo_stroke (cr);
    } cairo_restore (cr);

    if (dir < 0)
	cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
    else
	cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);

    for (n = 0; n < p->num_edges; n++) {
	const edge_t *e = &p->edges[n];
	double dx, dy;
	double x1, x2;

	if (e->dir != dir)
	    continue;

	dx = e->p2.x - e->p1.x;
	dy = e->p2.y - e->p1.y;

	x1 = e->p1.x + (e->top - e->p1.y) / dy * dx;
	x2 = e->p1.x + (e->bot - e->p1.y) / dy * dx;

	cairo_move_to (cr, x1, e->top);
	cairo_line_to (cr, x2, e->bot);
    }
    cairo_save (cr); {
	cairo_identity_matrix (cr);
	cairo_set_line_width (cr, 1.);
	cairo_stroke (cr);
    } cairo_restore (cr);
}

static void draw_polygon (cairo_t *cr, polygon_t *p, gdouble sf)
{
    draw_edges (cr, p, sf, -1);

    draw_edges (cr, p, sf, +1);
}

static cairo_surface_t *
pixmap_create (PolygonView *self, cairo_surface_t *target)
{
    cairo_surface_t *surface =
	cairo_surface_create_similar (target, CAIRO_CONTENT_COLOR,
				      self->widget.allocation.width,
				      self->widget.allocation.height);
    cairo_t *cr = cairo_create (surface);
    polygon_t *polygon;
    gdouble sf_x, sf_y, sf;
    gdouble mid, dim;
    gdouble x0,  y0;
    box_t extents;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    if (self->polygons == NULL) {
	cairo_destroy(cr);
	return surface;
    }

    extents = self->extents;

    mid = (extents.p2.x + extents.p1.x) / 2.;
    dim = (extents.p2.x - extents.p1.x) / 2. * 1.25;
    sf_x = self->widget.allocation.width / dim / 2;

    mid = (extents.p2.y + extents.p1.y) / 2.;
    dim = (extents.p2.y - extents.p1.y) / 2. * 1.25;
    sf_y = self->widget.allocation.height / dim / 2;

    sf = MIN (sf_x, sf_y);

    mid = (extents.p2.x + extents.p1.x) / 2.;
    dim = sf_x / sf * (extents.p2.x - extents.p1.x) / 2. * 1.25;
    x0 = mid - dim;
    mid = (extents.p2.y + extents.p1.y) / 2.;
    dim = sf_y / sf * (extents.p2.y - extents.p1.y) / 2. * 1.25;
    y0 = mid - dim;

    cairo_save (cr); {
	cairo_scale (cr, sf, sf);
	cairo_translate (cr, -x0, -y0);

	for (polygon = self->polygons; polygon; polygon = polygon->next) {
	    if (polygon->num_edges == 0)
		continue;

	    draw_polygon (cr, polygon, sf);
	}

	if (highlight != -1) {
	    cairo_move_to (cr, extents.p1.x, highlight);
	    cairo_line_to (cr, extents.p2.x, highlight);
	    cairo_set_source_rgb (cr, 0, .7, 0);
	    cairo_save (cr);
	    cairo_identity_matrix (cr);
	    cairo_set_line_width (cr, 1.);
	    cairo_stroke (cr);
	    cairo_restore (cr);
	}
    } cairo_restore (cr);

    cairo_destroy (cr);
    return surface;
}

static void
polygon_view_draw (PolygonView *self, cairo_t *cr)
{
    polygon_t *polygon;
    gdouble sf_x, sf_y, sf;
    gdouble mid, dim;
    gdouble x0,  y0;
    box_t extents;

    extents = self->extents;

    mid = (extents.p2.x + extents.p1.x) / 2.;
    dim = (extents.p2.x - extents.p1.x) / 2. * 1.25;
    sf_x = self->widget.allocation.width / dim / 2;

    mid = (extents.p2.y + extents.p1.y) / 2.;
    dim = (extents.p2.y - extents.p1.y) / 2. * 1.25;
    sf_y = self->widget.allocation.height / dim / 2;

    sf = MIN (sf_x, sf_y);

    mid = (extents.p2.x + extents.p1.x) / 2.;
    dim = sf_x / sf * (extents.p2.x - extents.p1.x) / 2. * 1.25;
    x0 = mid - dim;
    mid = (extents.p2.y + extents.p1.y) / 2.;
    dim = sf_y / sf * (extents.p2.y - extents.p1.y) / 2. * 1.25;
    y0 = mid - dim;

    if (self->pixmap_width != self->widget.allocation.width ||
	self->pixmap_height != self->widget.allocation.height)
    {
	cairo_surface_destroy (self->pixmap);
	self->pixmap = pixmap_create (self, cairo_get_target (cr));
	self->pixmap_width = self->widget.allocation.width;
	self->pixmap_height = self->widget.allocation.height;
    }

    cairo_set_source_surface (cr, self->pixmap, 0, 0);
    cairo_paint (cr);

    if (self->polygons == NULL)
	return;

    /* draw a zoom view of the area around the mouse */
    if (1) {
	double zoom = self->mag_zoom;
	int size = self->mag_size;
	int mag_x = self->mag_x;
	int mag_y = self->mag_y;

	if (1) {
	    if (self->px + size < self->widget.allocation.width/2)
		mag_x = self->px + size/4;
	    else
		mag_x = self->px - size/4 - size;
	    mag_y = self->py - size/2;
	    if (mag_y < 0)
		mag_y = 0;
	    if (mag_y + size > self->widget.allocation.height)
		mag_y = self->widget.allocation.height - size;
	}

	cairo_save (cr); {
	    /* bottom right */
	    cairo_rectangle (cr, mag_x, mag_y, size, size);
	    cairo_stroke_preserve (cr);
	    cairo_set_source_rgb (cr, 1, 1, 1);
	    cairo_fill_preserve (cr);
	    cairo_clip (cr);

	    /* compute roi in extents */
	    cairo_translate (cr, mag_x + size/2, mag_y + size/2);

	    cairo_save (cr); {
		cairo_scale (cr, zoom, zoom);
		cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));
		for (polygon = self->polygons; polygon; polygon = polygon->next) {
		    if (polygon->num_edges == 0)
			continue;

		    draw_polygon (cr, polygon, zoom);
		}

		if (highlight != -1) {
		    cairo_move_to (cr, extents.p1.x, highlight);
		    cairo_line_to (cr, extents.p2.x, highlight);
		    cairo_set_source_rgb (cr, 0, .7, 0);
		    cairo_save (cr);
		    cairo_identity_matrix (cr);
		    cairo_set_line_width (cr, 1.);
		    cairo_stroke (cr);
		    cairo_restore (cr);
		}
	    } cairo_restore (cr);

	    /* grid */
	    cairo_save (cr); {
		int i;

		cairo_translate (cr,
				 -zoom*fmod (self->px/sf + x0, 1.),
				 -zoom*fmod (self->py/sf + y0, 1.));
		zoom /= 2;
		for (i = -size/2/zoom; i <= size/2/zoom + 1; i+=2) {
		    cairo_move_to (cr, zoom*i, -size/2);
		    cairo_line_to (cr, zoom*i, size/2 + zoom);
		    cairo_move_to (cr, -size/2, zoom*i);
		    cairo_line_to (cr, size/2 + zoom, zoom*i);
		}
		zoom *= 2;
		cairo_set_source_rgba (cr, .7, .7, .7, .5);
		cairo_set_line_width (cr, 1.);
		cairo_stroke (cr);

		for (i = -size/2/zoom - 1; i <= size/2/zoom + 1; i++) {
		    cairo_move_to (cr, zoom*i, -size/2);
		    cairo_line_to (cr, zoom*i, size/2 + zoom);
		    cairo_move_to (cr, -size/2, zoom*i);
		    cairo_line_to (cr, size/2 + zoom, zoom*i);
		}
		cairo_set_source_rgba (cr, .1, .1, .1, .5);
		cairo_set_line_width (cr, 2.);
		cairo_stroke (cr);
	    } cairo_restore (cr);

	} cairo_restore (cr);
    }
}

static gboolean
polygon_view_expose (GtkWidget *w, GdkEventExpose *ev)
{
    PolygonView *self = (PolygonView *) w;
    cairo_t *cr;

    cr = gdk_cairo_create (w->window);
    gdk_cairo_region (cr, ev->region);
    cairo_clip (cr);

    polygon_view_draw (self, cr);

    cairo_destroy (cr);
    return FALSE;
}

static gboolean
polygon_view_key_press (GtkWidget *w, GdkEventKey *ev)
{
    switch (ev->keyval) {
    case GDK_Escape:
    case GDK_Q:
	gtk_main_quit ();
	break;
    }

    return FALSE;
}

static gboolean
polygon_view_button_press (GtkWidget *w, GdkEventButton *ev)
{
    PolygonView *self = (PolygonView *) w;

    if (ev->x < self->mag_x ||
	ev->y < self->mag_y ||
	ev->x > self->mag_x + self->mag_size ||
	ev->y > self->mag_y + self->mag_size)
    {
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
polygon_view_button_release (GtkWidget *w, GdkEventButton *ev)
{
    PolygonView *self = (PolygonView *) w;

    self->in_mag_drag = FALSE;

    return FALSE;
}

static void
polygon_view_update_mouse (PolygonView *self, GdkEventMotion *ev)
{
    self->px = ev->x;
    self->py = ev->y;

    gtk_widget_queue_draw (&self->widget);
}

static void
polygon_view_update_magnifier (PolygonView *self, gint *xy)
{
    self->mag_x = xy[0];
    self->mag_y = xy[1];

    gtk_widget_queue_draw (&self->widget);
}

static gboolean
polygon_view_motion (GtkWidget *w, GdkEventMotion *ev)
{
    PolygonView *self = (PolygonView *) w;

    if (self->in_mag_drag) {
	int xy[2];

	xy[0] = self->mag_x + ev->x - self->mag_drag_x;
	xy[1] = self->mag_y + ev->y - self->mag_drag_y;

	polygon_view_update_magnifier (self, xy);

	self->mag_drag_x = ev->x;
	self->mag_drag_y = ev->y;
    } else if (ev->x < self->mag_x ||
	       ev->y < self->mag_y ||
	       ev->x > self->mag_x + self->mag_size ||
	       ev->y > self->mag_y + self->mag_size)
    {
	polygon_view_update_mouse (self, ev);
    }

    return FALSE;
}

static void
polygon_view_realize (GtkWidget *widget)
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
polygon_view_size_allocate (GtkWidget *w, GdkRectangle *r)
{
    PolygonView *self = (PolygonView *) w;

    GTK_WIDGET_CLASS (polygon_view_parent_class)->size_allocate (w, r);

    self->mag_x = w->allocation.width - self->mag_size - 10;
    self->mag_y = w->allocation.height - self->mag_size - 10;
}

static void
polygon_view_finalize (GObject *obj)
{
    G_OBJECT_CLASS (polygon_view_parent_class)->finalize (obj);
}

static void
polygon_view_class_init (PolygonViewClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

    object_class->finalize = polygon_view_finalize;

    widget_class->realize = polygon_view_realize;
    widget_class->size_allocate = polygon_view_size_allocate;
    widget_class->expose_event = polygon_view_expose;
    widget_class->key_press_event = polygon_view_key_press;
    widget_class->button_press_event = polygon_view_button_press;
    widget_class->button_release_event = polygon_view_button_release;
    widget_class->motion_notify_event = polygon_view_motion;
}

static void
polygon_view_init (PolygonView *self)
{
    self->mag_zoom = 64;
    self->mag_size = 200;

    self->extents.p1.x = G_MAXDOUBLE;
    self->extents.p1.y = G_MAXDOUBLE;
    self->extents.p2.x = -G_MAXDOUBLE;
    self->extents.p2.y = -G_MAXDOUBLE;

    GTK_WIDGET_SET_FLAGS (self, GTK_CAN_FOCUS);
}

static polygon_t *
_polygon_add_edge (PolygonView *view, polygon_t *polygon,
		   point_t *p1, point_t *p2,
		   gdouble top, gdouble bot, int dir)
{
    if (polygon == NULL)
	return NULL;

    if (top < view->extents.p1.y)
	view->extents.p1.y = top;
    if (bot > view->extents.p2.y)
	view->extents.p2.y = bot;

    if (p1->x < view->extents.p1.x)
	view->extents.p1.x = p1->x;
    if (p1->x > view->extents.p2.x)
	view->extents.p2.x = p1->x;
    if (p2->x < view->extents.p1.x)
	view->extents.p1.x = p2->x;
    if (p2->x > view->extents.p2.x)
	view->extents.p2.x = p2->x;

    if (polygon->num_edges == polygon->size) {
	int newsize = 2 * polygon->size;
	void *newpolygon;

	newpolygon = g_realloc (polygon,
			      sizeof (polygon_t) + newsize * sizeof (edge_t));
	if (newpolygon == NULL)
	    return polygon;

	polygon = newpolygon;
	polygon->size = newsize;

	if (polygon->next != NULL)
	    polygon->next->prev = newpolygon;
	if (polygon->prev != NULL)
	    polygon->prev->next = newpolygon;
	else
	    view->polygons = newpolygon;
    }

    polygon->edges[polygon->num_edges].p1 = *p1;
    polygon->edges[polygon->num_edges].p2 = *p2;
    polygon->edges[polygon->num_edges].top = top;
    polygon->edges[polygon->num_edges].bot = bot;
    polygon->edges[polygon->num_edges].dir = dir;
    polygon->num_edges++;

    return polygon;
}

static polygon_t *
polygon_new (PolygonView *view)
{
    polygon_t *t;

    t = g_malloc (sizeof (polygon_t) + 128 * sizeof (edge_t));
    t->prev = NULL;
    t->next = view->polygons;
    if (view->polygons)
	view->polygons->prev = t;
    view->polygons = t;

    t->size = 128;
    t->num_edges = 0;

    return t;
}

int
main (int argc, char **argv)
{
    PolygonView *view;
    polygon_t *polygon = NULL;
    GtkWidget *window;
    FILE *file;
    char *line = NULL;
    size_t len = 0;

    gtk_init (&argc, &argv);

    view = g_object_new (polygon_view_get_type (), NULL);

    file = fopen (argv[1], "r");
    if (file != NULL) {
	while (getline (&line, &len, file) != -1) {
	    point_t p1, p2;
	    double top, bottom;
	    int dir;

	    if (strncmp (line, "polygon: ", sizeof("polygon: ")-1) == 0) {
		if (polygon && polygon->num_edges) {
		    g_print ("read polygon with %d edges\n", polygon->num_edges);

		    polygon = polygon_new (view);
		} else if (polygon == NULL)
		    polygon = polygon_new (view);
	    } else if (sscanf (line, "  [%*d] = [(%lf, %lf), (%lf, %lf)], top=%lf, bottom=%lf, dir=%d", &p1.x, &p1.y, &p2.x, &p2.y, &top, &bottom, &dir) == 7) {
		polygon = _polygon_add_edge (view, polygon, &p1, &p2,
					     top, bottom, dir);
	    }
	}

	if (polygon && polygon->num_edges)
	    g_print ("read polygon with %d edges\n", polygon->num_edges);

	g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
		 view->extents.p1.x, view->extents.p1.y,
		 view->extents.p2.x, view->extents.p2.y);
	fclose (file);
    }

    if (argc > 2)
	highlight = atof (argv[2]);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (gtk_main_quit), NULL);
    gtk_widget_set_size_request (window, 800, 800);
    gtk_container_add (GTK_CONTAINER (window), &view->widget);
    gtk_widget_show_all (window);

    gtk_main ();
    return 0;
}
