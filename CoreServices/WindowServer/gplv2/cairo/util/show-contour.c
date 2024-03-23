#include "config.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <math.h>

typedef struct _point {
    gdouble x, y;
} point_t;
typedef struct _box {
    point_t p1, p2;
} box_t;

typedef struct _contour {
    struct _contour *next, *prev;
    int direction;
    int num_points;
    int size;
    point_t points[0];
} contour_t;

typedef struct _TrapView {
    GtkWidget widget;

    cairo_surface_t *pixmap;
    int pixmap_width, pixmap_height;

    box_t extents;
    contour_t *contours;

    double px, py;

    gint mag_x, mag_y;
    gint mag_size;
    gdouble mag_zoom;
    gboolean in_mag_drag;
    gint mag_drag_x, mag_drag_y;
} TrapView;

typedef struct _TrapViewClass {
    GtkWidgetClass parent_class;
} TrapViewClass;

G_DEFINE_TYPE (TrapView, trap_view, GTK_TYPE_WIDGET)

static cairo_surface_t *
pixmap_create (TrapView *self, cairo_surface_t *target)
{
    cairo_surface_t *surface =
	cairo_surface_create_similar (target, CAIRO_CONTENT_COLOR,
				      self->widget.allocation.width,
				      self->widget.allocation.height);
    cairo_t *cr = cairo_create (surface);
    contour_t *contour;
    gdouble sf_x, sf_y, sf;
    gdouble mid, dim;
    gdouble x0,  y0;
    int n;
    box_t extents;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    if (self->contours == NULL) {
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

    for (contour = self->contours; contour; contour = contour->next) {
	if (contour->num_points == 0)
	    continue;

	cairo_save (cr); {
	    cairo_scale (cr, sf, sf);
	    cairo_translate (cr, -x0, -y0);
	    switch (contour->direction) {
	    case -1:
		cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
		break;
	    case 0:
		cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
		break;
	    case 1:
		cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
		break;
	    }
	    {
		const point_t *p = &contour->points[0];
		cairo_arc (cr, p->x, p->y, 4/sf, 0, 2 * M_PI);
		cairo_save (cr);
		cairo_identity_matrix (cr);
		cairo_stroke (cr);
		cairo_restore (cr);
	    }
	    for (n = 0; n < contour->num_points; n++) {
		const point_t *p = &contour->points[n];
		cairo_arc (cr, p->x, p->y, 2/sf, 0, 2 * M_PI);
		cairo_fill (cr);
	    }
	    for (n = 0; n < contour->num_points; n++) {
		const point_t *p = &contour->points[n];
		cairo_line_to (cr, p->x, p->y);
	    }
	} cairo_restore (cr);

	switch (contour->direction) {
	case -1:
	    cairo_set_source_rgb (cr, 0.3, 0.3, 0.9);
	    break;
	case 0:
	    cairo_set_source_rgb (cr, 0.3, 0.9, 0.3);
	    break;
	case 1:
	    cairo_set_source_rgb (cr, 0.9, 0.3, 0.3);
	    break;
	}
	cairo_set_line_width (cr, 1.);
	cairo_stroke (cr);
    }

    cairo_destroy (cr);
    return surface;
}

static void
trap_view_draw (TrapView *self, cairo_t *cr)
{
    contour_t *contour;
    gdouble sf_x, sf_y, sf;
    gdouble mid, dim;
    gdouble x0,  y0;
    int n;
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

    if (self->contours == NULL)
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
		for (contour = self->contours; contour; contour = contour->next) {
		    if (contour->num_points == 0)
			continue;

		    cairo_save (cr); {
			cairo_scale (cr, zoom, zoom);
			cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));
			switch (contour->direction) {
			case -1:
			    cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
			    break;
			case 0:
			    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
			    break;
			case 1:
			    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
			    break;
			}
			{
			    const point_t *p = &contour->points[0];
			    cairo_arc (cr, p->x, p->y, 4/zoom, 0, 2 * M_PI);
			    cairo_save (cr);
			    cairo_identity_matrix (cr);
			    cairo_stroke (cr);
			    cairo_restore (cr);
			}
			for (n = 0; n < contour->num_points; n++) {
			    const point_t *p = &contour->points[n];
			    cairo_arc (cr, p->x, p->y, 2/zoom, 0, 2 * M_PI);
			    cairo_fill (cr);
			}
			for (n = 0; n < contour->num_points; n++) {
			    const point_t *p = &contour->points[n];
			    cairo_line_to (cr, p->x, p->y);
			}
		    } cairo_restore (cr);

		    switch (contour->direction) {
		    case -1:
			cairo_set_source_rgb (cr, 0.3, 0.3, 0.9);
			break;
		    case 0:
			cairo_set_source_rgb (cr, 0.3, 0.9, 0.3);
			break;
		    case 1:
			cairo_set_source_rgb (cr, 0.9, 0.3, 0.3);
			break;
		    }
		    cairo_stroke (cr);
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


static gdouble
edge_length (const point_t *p1, const point_t *p2)
{
    return hypot (p2->x - p1->x, p2->y - p1->y);
}

static gdouble
contour_compute_total_length (const contour_t *contour)
{
    int n;
    gdouble len = 0.;
    for (n = 1; n < contour->num_points; n++)
	len += edge_length (&contour->points[n-1], &contour->points[n]);
    return len;
}

static void
trap_view_draw_labels (TrapView *self, cairo_t *cr)
{
    contour_t *contour;
    int y = 12;

    for (contour = self->contours; contour; contour = contour->next) {
	double total_length = contour_compute_total_length (contour) / 256.;
	PangoLayout *layout;
	gint width, height;
	GString *string;
	gchar *str;

	if (contour->num_points == 0)
	    continue;

	string = g_string_new (NULL);
	g_string_append_printf (string,
				"Number of points:\t%d\n"
				"Total length of contour: \t%.2f",
				contour->num_points,
				total_length);

	str = g_string_free (string, FALSE);
	layout = gtk_widget_create_pango_layout (&self->widget, str);
	g_free (str);

	pango_layout_get_pixel_size (layout, &width, &height);

	switch (contour->direction) {
	case -1:
	    cairo_set_source_rgb (cr, 0.9, 0.3, 0.3);
	    break;
	case 0:
	    cairo_set_source_rgb (cr, 0.3, 0.9, 0.3);
	    break;
	case 1:
	    cairo_set_source_rgb (cr, 0.3, 0.3, 0.9);
	    break;
	}

	cairo_move_to (cr, 10, y);
	pango_cairo_show_layout (cr, layout);
	g_object_unref (layout);

	y += height + 4;
    }
}

static gboolean
trap_view_expose (GtkWidget *w, GdkEventExpose *ev)
{
    TrapView *self = (TrapView *) w;
    cairo_t *cr;

    cr = gdk_cairo_create (w->window);
    gdk_cairo_region (cr, ev->region);
    cairo_clip (cr);

    trap_view_draw (self, cr);
    trap_view_draw_labels (self, cr);

    cairo_destroy (cr);
    return FALSE;
}

static gboolean
trap_view_key_press (GtkWidget *w, GdkEventKey *ev)
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
trap_view_button_press (GtkWidget *w, GdkEventButton *ev)
{
    TrapView *self = (TrapView *) w;

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
trap_view_button_release (GtkWidget *w, GdkEventButton *ev)
{
    TrapView *self = (TrapView *) w;

    self->in_mag_drag = FALSE;

    return FALSE;
}

static void
trap_view_update_mouse (TrapView *self, GdkEventMotion *ev)
{
    self->px = ev->x;
    self->py = ev->y;

    gtk_widget_queue_draw (&self->widget);
}

static void
trap_view_update_magnifier (TrapView *self, gint *xy)
{
    self->mag_x = xy[0];
    self->mag_y = xy[1];

    gtk_widget_queue_draw (&self->widget);
}

static gboolean
trap_view_motion (GtkWidget *w, GdkEventMotion *ev)
{
    TrapView *self = (TrapView *) w;

    if (self->in_mag_drag) {
	int xy[2];

	xy[0] = self->mag_x + ev->x - self->mag_drag_x;
	xy[1] = self->mag_y + ev->y - self->mag_drag_y;

	trap_view_update_magnifier (self, xy);

	self->mag_drag_x = ev->x;
	self->mag_drag_y = ev->y;
    } else if (ev->x < self->mag_x ||
	       ev->y < self->mag_y ||
	       ev->x > self->mag_x + self->mag_size ||
	       ev->y > self->mag_y + self->mag_size)
    {
	trap_view_update_mouse (self, ev);
    }

    return FALSE;
}

static void
trap_view_realize (GtkWidget *widget)
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
trap_view_size_allocate (GtkWidget *w, GdkRectangle *r)
{
    TrapView *self = (TrapView *) w;

    GTK_WIDGET_CLASS (trap_view_parent_class)->size_allocate (w, r);

    self->mag_x = w->allocation.width - self->mag_size - 10;
    self->mag_y = w->allocation.height - self->mag_size - 10;
}

static void
trap_view_finalize (GObject *obj)
{
    G_OBJECT_CLASS (trap_view_parent_class)->finalize (obj);
}

static void
trap_view_class_init (TrapViewClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

    object_class->finalize = trap_view_finalize;

    widget_class->realize = trap_view_realize;
    widget_class->size_allocate = trap_view_size_allocate;
    widget_class->expose_event = trap_view_expose;
    widget_class->key_press_event = trap_view_key_press;
    widget_class->button_press_event = trap_view_button_press;
    widget_class->button_release_event = trap_view_button_release;
    widget_class->motion_notify_event = trap_view_motion;
}

static void
trap_view_init (TrapView *self)
{
    self->mag_zoom = 64;
    self->mag_size = 200;

    self->extents.p1.x = G_MAXDOUBLE;
    self->extents.p1.y = G_MAXDOUBLE;
    self->extents.p2.x = -G_MAXDOUBLE;
    self->extents.p2.y = -G_MAXDOUBLE;

    GTK_WIDGET_SET_FLAGS (self, GTK_CAN_FOCUS);
}

static contour_t *
_contour_add_point (TrapView *tv, contour_t *contour, point_t *p)
{
    if (contour == NULL)
	return NULL;

    if (p->y < tv->extents.p1.y)
	tv->extents.p1.y = p->y;
    if (p->y > tv->extents.p2.y)
	tv->extents.p2.y = p->y;

    if (p->x < tv->extents.p1.x)
	tv->extents.p1.x = p->x;
    if (p->x > tv->extents.p2.x)
	tv->extents.p2.x = p->x;

    if (contour->num_points == contour->size) {
	int newsize = 2 * contour->size;
	void *newcontour;

	newcontour = g_realloc (contour,
			      sizeof (contour_t) + newsize * sizeof (point_t));
	if (newcontour == NULL)
	    return contour;

	contour = newcontour;
	contour->size = newsize;

	if (contour->next != NULL)
	    contour->next->prev = newcontour;
	if (contour->prev != NULL)
	    contour->prev->next = newcontour;
	else
	    tv->contours = newcontour;
    }

    contour->points[contour->num_points++] = *p;

    return contour;
}

static contour_t *
contour_new (TrapView *tv, int direction)
{
    contour_t *t;

    t = g_malloc (sizeof (contour_t) + 128 * sizeof (point_t));
    t->direction = direction;
    t->prev = NULL;
    t->next = tv->contours;
    if (tv->contours)
	tv->contours->prev = t;
    tv->contours = t;

    t->size = 128;
    t->num_points = 0;

    return t;
}

int
main (int argc, char **argv)
{
    TrapView *tv;
    contour_t *contour = NULL;
    GtkWidget *window;
    FILE *file;
    char *line = NULL;
    size_t len = 0;

    gtk_init (&argc, &argv);

    tv = g_object_new (trap_view_get_type (), NULL);

    file = fopen (argv[1], "r");
    if (file != NULL) {
	while (getline (&line, &len, file) != -1) {
	    point_t p;
	    int direction;

	    if (sscanf (line, "contour: direction=%d", &direction)) {
		if (contour)
		    g_print ("read %d contour\n", contour->num_points);

		contour = contour_new (tv, direction);
	    } else if (sscanf (line, "  [%*d] = (%lf, %lf)", &p.x, &p.y) == 2) {
		contour = _contour_add_point (tv, contour, &p);
	    }
	}

	if (contour)
	    g_print ("read %d contour\n", contour->num_points);

	g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
		 tv->extents.p1.x, tv->extents.p1.y,
		 tv->extents.p2.x, tv->extents.p2.y);
	fclose (file);
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (gtk_main_quit), NULL);
    gtk_widget_set_size_request (window, 800, 800);
    gtk_container_add (GTK_CONTAINER (window), &tv->widget);
    gtk_widget_show_all (window);

    gtk_main ();
    return 0;
}
