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
typedef struct _line {
    point_t p1, p2;
} line_t;
typedef struct _trapezoid {
    gdouble top, bottom;
    line_t left, right;
} trapezoid_t;
typedef struct _traps {
    struct _traps *next, *prev;
    box_t extents;
    int num_traps;
    int size;
    trapezoid_t traps[0];
} traps_t;

typedef struct _edge {
    line_t line;
    gdouble top, bottom;
    point_t p1, p2;
    int dir;
} edge_t;
typedef struct _edges {
    struct _edges *next, *prev;
    box_t extents;
    int num_edges;
    int size;
    edge_t edges[0];
} edges_t;

typedef struct _TrapView {
    GtkWidget widget;

    struct _TrapView *group_head;
    struct _TrapView *group_next;
    struct _TrapView *group_prev;

    cairo_surface_t *pixmap;
    int pixmap_width, pixmap_height;

    traps_t *traps_list;
    traps_t *current_traps;

    edges_t *edges_list;
    edges_t *current_edges;

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

static cairo_surface_t *
pixmap_create (TrapView *self, cairo_surface_t *target)
{
    cairo_surface_t *surface = cairo_surface_create_similar (target, CAIRO_CONTENT_COLOR,
							     self->widget.allocation.width,
							     self->widget.allocation.height);
    cairo_t *cr;
    traps_t *traps;
    edges_t *edges;
    gdouble sf_x, sf_y, sf;
    gdouble mid, dim;
    gdouble x0,  y0;
    double dash[2] = {8, 8};
    double dots[2] = {0., 1.};
    int n;
    box_t extents;
    point_t p;

    traps = self->current_traps;
    if (traps == NULL)
	return surface;
    edges = self->current_edges;

    extents = traps->extents;
    if (edges != NULL) {
	if (edges->extents.p1.x < extents.p1.x)
	    extents.p1.x = edges->extents.p1.x;
	if (edges->extents.p1.y < extents.p1.y)
	    extents.p1.y = edges->extents.p1.y;
	if (edges->extents.p2.x > extents.p2.x)
	    extents.p2.x = edges->extents.p2.x;
	if (edges->extents.p2.y > extents.p2.y)
	    extents.p2.y = edges->extents.p2.y;
    }

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

    cr = cairo_create (surface);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_save (cr);
    cairo_scale (cr, sf, sf);
    cairo_translate (cr, -x0, -y0);
    cairo_set_source_rgba (cr, 0, 1, 0, .2);
    for (n = 0; n < traps->num_traps; n++) {
	const trapezoid_t *t = &traps->traps[n];

	_compute_intersection_point (&t->left, t->top, &p);
	cairo_move_to (cr, p.x, p.y);
	_compute_intersection_point (&t->right, t->top, &p);
	cairo_line_to (cr, p.x, p.y);
	_compute_intersection_point (&t->right, t->bottom, &p);
	cairo_line_to (cr, p.x, p.y);
	_compute_intersection_point (&t->left, t->bottom, &p);
	cairo_line_to (cr, p.x, p.y);
	cairo_close_path (cr);
	cairo_fill (cr);
    }
    cairo_restore (cr);

    if (edges == NULL) {
	cairo_save (cr);

	/* top, bottom */
	cairo_save (cr); {
	    cairo_matrix_t m;
	    cairo_matrix_init_scale (&m, sf, sf);
	    cairo_matrix_translate (&m, -x0, -y0);

	    cairo_set_line_width (cr, 1.);
	    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	    for (n = 0; n < traps->num_traps; n++) {
		const trapezoid_t *t = &traps->traps[n];

		_compute_intersection_point (&t->left, t->top, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_move_to (cr, floor (p.x), floor (p.y) + .5);
		cairo_set_dash (cr, dash, 2, fmod (floor (p.x), dash[0] + dash[1]));

		_compute_intersection_point (&t->right, t->top, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_line_to (cr, ceil (p.x), floor (p.y) + .5);
		cairo_stroke (cr);

		_compute_intersection_point (&t->left, t->bottom, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_move_to (cr, floor (p.x), floor (p.y) + .5);
		cairo_set_dash (cr, dash, 2, fmod (floor (p.x), dash[0] + dash[1]));

		_compute_intersection_point (&t->right, t->bottom, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_line_to (cr, ceil (p.x), floor (p.y) + .5);
		cairo_stroke (cr);
	    }
	} cairo_restore (cr);

	/* left extents */
	cairo_save (cr); {
	    cairo_save (cr); {
		cairo_scale (cr, sf, sf);
		cairo_translate (cr, -x0, -y0);

		for (n = 0; n < traps->num_traps; n++) {
		    const trapezoid_t *t = &traps->traps[n];
		    cairo_move_to (cr, t->left.p1.x, t->left.p1.y);
		    cairo_line_to (cr, t->left.p2.x, t->left.p2.y);
		}
	    } cairo_restore (cr);
	    cairo_set_source_rgb (cr, 1, 0, 0);
	    cairo_set_line_width (cr, 1.);
	    cairo_set_dash (cr, dash, 2, 0.);
	    cairo_stroke (cr);
	} cairo_restore (cr);

	/* left line */
	cairo_save (cr); {
	    cairo_save (cr); {
		cairo_scale (cr, sf, sf);
		cairo_translate (cr, -x0, -y0);

		for (n = 0; n < traps->num_traps; n++) {
		    const trapezoid_t *t = &traps->traps[n];
		    _compute_intersection_point (&t->left, t->top, &p);
		    cairo_move_to (cr, p.x, p.y);
		    _compute_intersection_point (&t->left, t->bottom, &p);
		    cairo_line_to (cr, p.x, p.y);
		}
	    } cairo_restore (cr);
	    cairo_set_source_rgb (cr, 1, 0, 0);
	    cairo_stroke (cr);
	} cairo_restore (cr);

	/* right extents */
	cairo_save (cr); {
	    cairo_save (cr); {
		cairo_scale (cr, sf, sf);
		cairo_translate (cr, -x0, -y0);

		for (n = 0; n < traps->num_traps; n++) {
		    const trapezoid_t *t = &traps->traps[n];
		    cairo_move_to (cr, t->right.p1.x, t->right.p1.y);
		    cairo_line_to (cr, t->right.p2.x, t->right.p2.y);
		}
	    } cairo_restore (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1);
	    cairo_set_line_width (cr, 1.);
	    cairo_set_dash (cr, dash, 2, 0.);
	    cairo_stroke (cr);
	} cairo_restore (cr);

	/* right line */
	cairo_save (cr); {
	    cairo_save (cr); {
		cairo_scale (cr, sf, sf);
		cairo_translate (cr, -x0, -y0);

		for (n = 0; n < traps->num_traps; n++) {
		    const trapezoid_t *t = &traps->traps[n];
		    _compute_intersection_point (&t->right, t->top, &p);
		    cairo_move_to (cr, p.x, p.y);
		    _compute_intersection_point (&t->right, t->bottom, &p);
		    cairo_line_to (cr, p.x, p.y);
		} cairo_restore (cr);
		cairo_set_source_rgb (cr, 0, 0, 1);
		cairo_stroke (cr);
	    } cairo_restore (cr);
	}

	/* end-points */
	cairo_save (cr); {
	    cairo_save (cr); {
		cairo_scale (cr, sf, sf);
		cairo_translate (cr, -x0, -y0);

		for (n = 0; n < traps->num_traps; n++) {
		    const trapezoid_t *t = &traps->traps[n];
		    _compute_intersection_point (&t->left, t->top, &p);
		    cairo_move_to (cr, p.x, p.y);
		    cairo_close_path (cr);
		    _compute_intersection_point (&t->left, t->bottom, &p);
		    cairo_move_to (cr, p.x, p.y);
		    cairo_close_path (cr);
		    _compute_intersection_point (&t->right, t->top, &p);
		    cairo_move_to (cr, p.x, p.y);
		    cairo_close_path (cr);
		    _compute_intersection_point (&t->right, t->bottom, &p);
		    cairo_move_to (cr, p.x, p.y);
		    cairo_close_path (cr);
		}
	    } cairo_restore (cr);
	    cairo_set_source_rgb (cr, 0, 0, 0);
	    cairo_set_dash (cr, dots, 2, 0.);
	    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	    cairo_set_line_width (cr, 4.);
	    cairo_stroke (cr);
	} cairo_restore (cr);

	cairo_restore (cr);
    } else {
	cairo_save (cr);

	for (n = 0; n < edges->num_edges; n++) {
	    const edge_t *e = &edges->edges[n];

	    cairo_save (cr); {
		cairo_scale (cr, sf, sf);
		cairo_translate (cr, -x0, -y0);
		cairo_move_to (cr, e->p1.x, e->p1.y);
		cairo_line_to (cr, e->p2.x, e->p2.y);
	    } cairo_restore (cr);

	    if (e->dir < 0) {
		cairo_set_source_rgb (cr, 0, 0, 1);
		cairo_set_dash (cr, dash, 2, dash[0]);
	    } else {
		cairo_set_source_rgb (cr, 1, 0, 0);
		cairo_set_dash (cr, dash, 2, 0.);
	    }

	    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	    cairo_set_line_width (cr, 1.);
	    cairo_stroke (cr);

	    cairo_save (cr); {
		cairo_scale (cr, sf, sf);
		cairo_translate (cr, -x0, -y0);
		cairo_move_to (cr, e->p1.x, e->p1.y);
		cairo_close_path (cr);
		cairo_move_to (cr, e->p2.x, e->p2.y);
		cairo_close_path (cr);
	    } cairo_restore (cr);
	    cairo_set_source_rgb (cr, 0, 0, 0);
	    cairo_set_dash (cr, dots, 2, 0.);
	    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	    cairo_set_line_width (cr, 4.);
	    cairo_stroke (cr);
	}

	cairo_restore (cr);
    }

    cairo_destroy (cr);
    return surface;
}

static void
trap_view_draw (TrapView *self, cairo_t *cr)
{
    traps_t *traps;
    edges_t *edges;
    gdouble sf_x, sf_y, sf;
    gdouble mid, dim;
    gdouble x0, y0;
    double dash[2] = {8, 8};
    int n;
    box_t extents;
    point_t p;

    if (self->pixmap_width != self->widget.allocation.width ||
	self->pixmap_height != self->widget.allocation.height)
    {
	cairo_surface_destroy (self->pixmap);
	self->pixmap = pixmap_create (self, cairo_get_target (cr));
	self->pixmap_width = self->widget.allocation.width;
	self->pixmap_height = self->widget.allocation.height;
    }

    cairo_save (cr);
    cairo_set_source_surface (cr, self->pixmap, 0, 0);
    cairo_paint (cr);
    cairo_restore (cr);

    traps = self->current_traps;
    if (traps == NULL)
	return;

    extents = traps->extents;
    edges = self->current_edges;
    if (edges != NULL) {
	if (edges->extents.p1.x < extents.p1.x)
	    extents.p1.x = edges->extents.p1.x;
	if (edges->extents.p1.y < extents.p1.y)
	    extents.p1.y = edges->extents.p1.y;
	if (edges->extents.p2.x > extents.p2.x)
	    extents.p2.x = edges->extents.p2.x;
	if (edges->extents.p2.y > extents.p2.y)
	    extents.p2.y = edges->extents.p2.y;
    }

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

    cairo_save (cr);

    /* draw a zoom view of the area around the mouse */
    {
	double zoom = self->mag_zoom;
	int size = self->mag_size;

	cairo_save (cr);

	/* bottom right */
	cairo_rectangle (cr, self->mag_x, self->mag_y, size, size);
	cairo_stroke_preserve (cr);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_fill_preserve (cr);
	cairo_clip (cr);

	/* compute roi in extents */
	cairo_translate (cr, self->mag_x + size/2, self->mag_y + size/2);

	cairo_save (cr);
	cairo_scale (cr, zoom, zoom);
	cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));
	for (n = 0; n < traps->num_traps; n++) {
	    const trapezoid_t *t = &traps->traps[n];

	    _compute_intersection_point (&t->left, t->top, &p);
	    cairo_move_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->right, t->top, &p);
	    cairo_line_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->right, t->bottom, &p);
	    cairo_line_to (cr, p.x, p.y);
	    _compute_intersection_point (&t->left, t->bottom, &p);
	    cairo_line_to (cr, p.x, p.y);
	    cairo_close_path (cr);
	    cairo_set_source_rgba (cr, 0, 1, 0, .2);
	    cairo_fill (cr);
	}
	cairo_restore (cr);

	cairo_save (cr); {
	    cairo_matrix_t m;
	    cairo_matrix_init_scale (&m, zoom, zoom);
	    cairo_matrix_translate (&m, -(self->px / sf + x0), -(self->py /sf + y0));

	    cairo_set_source_rgb (cr, 0, 0, 0);
	    cairo_set_line_width (cr, 1.);
	    cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	    for (n = 0; n < traps->num_traps; n++) {
		const trapezoid_t *t = &traps->traps[n];

		_compute_intersection_point (&t->left, t->top, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_move_to (cr, floor (p.x), floor (p.y) + .5);
		cairo_set_dash (cr, dash, 2, fmod (floor (p.x), dash[0] + dash[1]));

		_compute_intersection_point (&t->right, t->top, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_line_to (cr, ceil (p.x), floor (p.y) + .5);
		cairo_stroke (cr);

		_compute_intersection_point (&t->left, t->bottom, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_move_to (cr, floor (p.x), floor (p.y) + .5);
		cairo_set_dash (cr, dash, 2, fmod (floor (p.x), dash[0] + dash[1]));

		_compute_intersection_point (&t->right, t->bottom, &p);
		cairo_matrix_transform_point (&m, &p.x, &p.y);
		cairo_line_to (cr, ceil (p.x), floor (p.y) + .5);
		cairo_stroke (cr);
	    }
	} cairo_restore (cr);

	cairo_save (cr); { /* left extents */
	    cairo_save (cr); {
		cairo_scale (cr, zoom, zoom);
		cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));
		for (n = 0; n < traps->num_traps; n++) {
		    const trapezoid_t *t = &traps->traps[n];
		    cairo_move_to (cr, t->left.p1.x, t->left.p1.y);
		    cairo_line_to (cr, t->left.p2.x, t->left.p2.y);
		}
	    } cairo_restore (cr);
	    cairo_set_source_rgb (cr, 1, 0, 0);
	    cairo_set_line_width (cr, .5);
	    cairo_set_dash (cr, dash, 2, 0.);
	    cairo_stroke (cr);
	} cairo_restore (cr);
	cairo_save (cr); { /* right extents */
	    cairo_save (cr); {
		cairo_scale (cr, zoom, zoom);
		cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));

		for (n = 0; n < traps->num_traps; n++) {
		    const trapezoid_t *t = &traps->traps[n];
		    cairo_move_to (cr, t->right.p1.x, t->right.p1.y);
		    cairo_line_to (cr, t->right.p2.x, t->right.p2.y);
		}
	    } cairo_restore (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1);
	    cairo_set_line_width (cr, .5);
	    cairo_set_dash (cr, dash, 2, 0.);
	    cairo_stroke (cr);
	} cairo_restore (cr);

	cairo_save (cr); { /* left lines */
	    cairo_save (cr);
	    cairo_scale (cr, zoom, zoom);
	    cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));
	    for (n = 0; n < traps->num_traps; n++) {
		const trapezoid_t *t = &traps->traps[n];
		_compute_intersection_point (&t->left, t->top, &p);
		cairo_move_to (cr, p.x, p.y);
		_compute_intersection_point (&t->left, t->bottom, &p);
		cairo_line_to (cr, p.x, p.y);
	    }
	    cairo_restore (cr);
	    cairo_set_source_rgb (cr, 1, 0, 0);
	    cairo_stroke (cr);
	} cairo_restore (cr);
	cairo_save (cr); { /* right lines */
	    cairo_save (cr);
	    cairo_scale (cr, zoom, zoom);
	    cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));
	    for (n = 0; n < traps->num_traps; n++) {
		const trapezoid_t *t = &traps->traps[n];
		_compute_intersection_point (&t->right, t->top, &p);
		cairo_move_to (cr, p.x, p.y);
		_compute_intersection_point (&t->right, t->bottom, &p);
		cairo_line_to (cr, p.x, p.y);
	    }
	    cairo_restore (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1);
	    cairo_stroke (cr);
	} cairo_restore (cr);

	/* end-points */
	cairo_save (cr); {
	    double dots[2] = {0., 1.};

	    cairo_save (cr);
	    cairo_scale (cr, zoom, zoom);
	    cairo_translate (cr, -(self->px / sf + x0), -(self->py /sf + y0));
	    for (n = 0; n < traps->num_traps; n++) {
		const trapezoid_t *t = &traps->traps[n];
		_compute_intersection_point (&t->left, t->top, &p);
		cairo_move_to (cr, p.x, p.y);
		cairo_close_path (cr);
		_compute_intersection_point (&t->left, t->bottom, &p);
		cairo_move_to (cr, p.x, p.y);
		cairo_close_path (cr);
		_compute_intersection_point (&t->right, t->top, &p);
		cairo_move_to (cr, p.x, p.y);
		cairo_close_path (cr);
		_compute_intersection_point (&t->right, t->bottom, &p);
		cairo_move_to (cr, p.x, p.y);
		cairo_close_path (cr);
	    }
	    cairo_restore (cr);
	    cairo_set_source_rgb (cr, 0, 0, 0);
	    cairo_set_dash (cr, dots, 2, 0.);
	    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	    cairo_set_line_width (cr, 4.);
	    cairo_stroke (cr);
	} cairo_restore (cr);

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
    }

    cairo_restore (cr);
}

static gdouble
trapezoid_area (const trapezoid_t *t)
{
    gdouble inner_left, inner_right;
    gdouble outer_left, outer_right;
    gdouble height;
    gdouble area;

    /* split into 3 sections: a rectangle with a pair of triangular bookends */
    inner_left = _compute_intersection_x_for_y (&t->left, t->top);
    outer_left = _compute_intersection_x_for_y (&t->left, t->bottom);
    if (outer_left > inner_left) {
	gdouble t = outer_left;
	outer_left = inner_left;
	inner_left = t;
    }

    inner_right = _compute_intersection_x_for_y (&t->right, t->top);
    outer_right = _compute_intersection_x_for_y (&t->right, t->bottom);
    if (outer_right > inner_right) {
	gdouble t = outer_right;
	outer_right = inner_right;
	inner_right = t;
    }

    if (outer_left > outer_right) { /* reverse */
	gdouble t;

	t = outer_left;
	outer_left = inner_right;
	inner_right = t;

	t = inner_left;
	inner_left = outer_right;
	outer_right = t;
    }

    height = t->bottom - t->top;
    area  = (inner_left - outer_left) * height / 2;
    area += (outer_right - inner_right) * height / 2;
    area += (inner_right - inner_left) * height;

    return area;
}

static gdouble
traps_compute_total_area (const traps_t *traps)
{
    int n;
    gdouble area = 0.;
    for (n = 0; n < traps->num_traps; n++)
	area += trapezoid_area (&traps->traps[n]);
    return area;
}

static void
trap_view_draw_labels (TrapView *self, cairo_t *cr)
{
    PangoLayout *layout;
    gint width, height;
    gdouble total_area;
    gchar *str;
    traps_t *traps;

    traps = self->current_traps;
    if (traps == NULL)
	return;

    /* convert total area from fixed-point (assuming 24.8) */
    total_area = traps_compute_total_area (traps) / (256. * 256.);
    str = g_strdup_printf ("Number of trapezoids:\t%d\n"
			   "Total area of trapezoids:\t%.2f",
			   traps->num_traps,
			   total_area);
    layout = gtk_widget_create_pango_layout (&self->widget, str);
    g_free (str);

    pango_layout_get_pixel_size (layout, &width, &height);

    cairo_move_to (cr, 10, 10 + height);
    pango_cairo_show_layout (cr, layout);
    g_object_unref (layout);
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

static void
trap_view_advance (TrapView *self)
{
    if (self->current_traps && self->current_traps->prev)
	self->current_traps = self->current_traps->prev;
    if (self->current_edges && self->current_edges->prev)
	self->current_edges = self->current_edges->prev;
    self->pixmap_width = self->pixmap_height = 0;
    gtk_widget_queue_draw (&self->widget);
}

static void
trap_view_back (TrapView *self)
{
    if (self->current_traps && self->current_traps->next)
	self->current_traps = self->current_traps->next;
    if (self->current_edges && self->current_edges->next)
	self->current_edges = self->current_edges->next;
    self->pixmap_width = self->pixmap_height = 0;
    gtk_widget_queue_draw (&self->widget);
}

static void
trap_view_group_foreach (TrapView *group, GFunc func, gpointer data)
{
    while (group) {
	func (group, data);
	group = group->group_next;
    }
}

static gboolean
trap_view_key_press (GtkWidget *w, GdkEventKey *ev)
{
    TrapView *self = (TrapView *) w;

    switch (ev->keyval) {
    case GDK_BackSpace:
	trap_view_group_foreach (self->group_head,
				 (GFunc) trap_view_back,
				 NULL);
	break;

    case GDK_space:
	trap_view_group_foreach (self->group_head,
				 (GFunc) trap_view_advance,
				 NULL);
	break;

    case GDK_Return:
	trap_view_group_foreach (self->group_head,
				 (GFunc) trap_view_advance,
				 NULL);
	break;

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
	if (ev->type == GDK_BUTTON_PRESS) {
	    if (self->current_traps == NULL)
		return FALSE;

	    if (ev->button == 1) {
		trap_view_group_foreach (self->group_head,
					 (GFunc) trap_view_advance,
					 NULL);
	    } else if (ev->button == 3) {
		trap_view_group_foreach (self->group_head,
					 (GFunc) trap_view_back,
					 NULL);
	    }
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

	trap_view_group_foreach (self->group_head,
				 (GFunc) trap_view_update_magnifier,
				 xy);

	self->mag_drag_x = ev->x;
	self->mag_drag_y = ev->y;
    } else if (ev->x < self->mag_x ||
	       ev->y < self->mag_y ||
	       ev->x > self->mag_x + self->mag_size ||
	       ev->y > self->mag_y + self->mag_size)
    {
	trap_view_group_foreach (self->group_head,
				 (GFunc) trap_view_update_mouse,
				 ev);
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
    self->mag_zoom = 10;
    self->mag_size = 200;

    GTK_WIDGET_SET_FLAGS (self, GTK_CAN_FOCUS);
}

static traps_t *
_traps_add_trapezoid (TrapView *tv, traps_t *traps, const trapezoid_t *trap)
{
    if (trap->top < traps->extents.p1.y)
	traps->extents.p1.y = trap->top;
    if (trap->bottom > traps->extents.p2.y)
	traps->extents.p2.y = trap->bottom;

    if (trap->left.p1.x < traps->extents.p1.x)
	traps->extents.p1.x = trap->left.p1.x;
    if (trap->left.p2.x < traps->extents.p1.x)
	traps->extents.p1.x = trap->left.p2.x;

    if (trap->right.p1.x > traps->extents.p2.x)
	traps->extents.p2.x = trap->right.p1.x;
    if (trap->right.p2.x > traps->extents.p2.x)
	traps->extents.p2.x = trap->right.p2.x;

    if (traps->num_traps == traps->size) {
	int newsize = 2 * traps->size;
	void *newtraps;

	newtraps = g_realloc (traps,
			      sizeof (traps_t) + newsize * sizeof (trapezoid_t));
	if (newtraps == NULL)
	    return traps;

	if (tv->current_traps == traps)
	    tv->current_traps = newtraps;

	traps = newtraps;
	traps->size = newsize;

	if (traps->next != NULL)
	    traps->next->prev = newtraps;
	if (traps->prev != NULL)
	    traps->prev->next = newtraps;
	else
	    tv->traps_list = newtraps;
    }

    traps->traps[traps->num_traps++] = *trap;

    return traps;
}

static traps_t *
traps_new (TrapView *tv)
{
    traps_t *t;

    t = g_malloc (sizeof (traps_t) + 16 * sizeof (trapezoid_t));
    t->prev = NULL;
    t->next = tv->traps_list;
    if (tv->traps_list)
	tv->traps_list->prev = t;
    tv->traps_list = t;

    if (tv->current_traps == NULL)
	tv->current_traps = t;

    t->size = 16;
    t->num_traps = 0;
    t->extents.p1.x = G_MAXDOUBLE;
    t->extents.p1.y = G_MAXDOUBLE;
    t->extents.p2.x = -G_MAXDOUBLE;
    t->extents.p2.y = -G_MAXDOUBLE;

    return t;
}

static edges_t *
_edges_add_edge (TrapView *tv, edges_t *edges, edge_t *e)
{
    if (e->top < edges->extents.p1.y)
	edges->extents.p1.y = e->top;
    if (e->bottom > edges->extents.p2.y)
	edges->extents.p2.y = e->bottom;

    _compute_intersection_point (&e->line, e->top, &e->p1);
    _compute_intersection_point (&e->line, e->bottom, &e->p2);

    if (e->p1.x < edges->extents.p1.x)
	edges->extents.p1.x = e->p1.x;
    if (e->p2.x < edges->extents.p1.x)
	edges->extents.p1.x = e->p2.x;

    if (e->p1.x > edges->extents.p2.x)
	edges->extents.p2.x = e->p1.x;
    if (e->p2.x > edges->extents.p2.x)
	edges->extents.p2.x = e->p2.x;

    if (edges->num_edges == edges->size) {
	int newsize = 2 * edges->size;
	void *newedges;

	newedges = g_realloc (edges,
			      sizeof (edges_t) + newsize * sizeof (edge_t));
	if (newedges == NULL)
	    return edges;

	if (tv->current_edges == edges)
	    tv->current_edges = newedges;

	edges = newedges;
	edges->size = newsize;

	if (edges->next != NULL)
	    edges->next->prev = newedges;
	if (edges->prev != NULL)
	    edges->prev->next = newedges;
	else
	    tv->edges_list = newedges;
    }

    edges->edges[edges->num_edges++] = *e;

    return edges;
}

static edges_t *
edges_new (TrapView *tv)
{
    edges_t *t;

    t = g_malloc (sizeof (edges_t) + 16 * sizeof (edge_t));
    t->prev = NULL;
    t->next = tv->edges_list;
    if (tv->edges_list)
	tv->edges_list->prev = t;
    tv->edges_list = t;

    if (tv->current_edges == NULL)
	tv->current_edges = t;

    t->size = 16;
    t->num_edges = 0;
    t->extents.p1.x = G_MAXDOUBLE;
    t->extents.p1.y = G_MAXDOUBLE;
    t->extents.p2.x = -G_MAXDOUBLE;
    t->extents.p2.y = -G_MAXDOUBLE;

    return t;
}

int
main (int argc, char **argv)
{
    TrapView *tv, *group_head = NULL, *group_prev = NULL;
    GtkWidget *window, *hbox;
    FILE *file;

    gtk_init (&argc, &argv);

    hbox = gtk_hbox_new (TRUE, 0);

    file = fopen (argv[1], "r");
    if (file != NULL) {
	char *line = NULL;
	size_t len = 0;
	traps_t *traps;

	tv = g_object_new (trap_view_get_type (), NULL);
	gtk_box_pack_start (GTK_BOX (hbox), &tv->widget, TRUE, TRUE, 0);
	gtk_widget_show (&tv->widget);

	tv->group_prev = group_prev;
	tv->group_next = NULL;
	if (group_prev)
	    group_prev->group_next = tv;
	group_prev = tv;
	if (group_head == NULL)
	    group_head = tv;
	tv->group_head = group_head;

	traps = traps_new (tv);
	while (getline (&line, &len, file) != -1) {
	    trapezoid_t t;

	    if (sscanf (line,
		       "%lf %lf L:(%lf, %lf), (%lf, %lf) R:(%lf, %lf), (%lf, %lf)",
		       &t.top, &t.bottom,
		       &t.left.p1.x, &t.left.p1.y,
		       &t.left.p2.x, &t.left.p2.y,
		       &t.right.p1.x, &t.right.p1.y,
		       &t.right.p2.x, &t.right.p2.y) == 10) {
		traps = _traps_add_trapezoid (tv, traps, &t);
	    } else {
		if (traps->num_traps) {
		    g_print ("read %d trapezoids\n", traps->num_traps);
		    g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
			    traps->extents.p1.x, traps->extents.p1.y,
			    traps->extents.p2.x, traps->extents.p2.y);
		    traps = traps_new (tv);
		}
	    }
	}
	free (line);
	fclose (file);

	if (traps->num_traps) {
	    g_print ("read %d trapezoids\n", traps->num_traps);
	    g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
		     traps->extents.p1.x, traps->extents.p1.y,
		     traps->extents.p2.x, traps->extents.p2.y);
	}
    }

    file = fopen (argv[2], "r");
    if (file != NULL) {
	char *line = NULL;
	size_t len = 0;
	traps_t *traps;

	tv = g_object_new (trap_view_get_type (), NULL);
	gtk_box_pack_start (GTK_BOX (hbox), &tv->widget, TRUE, TRUE, 0);
	gtk_widget_show (&tv->widget);

	tv->group_prev = group_prev;
	tv->group_next = NULL;
	if (group_prev)
	    group_prev->group_next = tv;
	group_prev = tv;
	if (group_head == NULL)
	    group_head = tv;
	tv->group_head = group_head;

	traps = traps_new (tv);
	while (getline (&line, &len, file) != -1) {
	    trapezoid_t t;

	    if (sscanf (line,
		       "%lf %lf L:(%lf, %lf), (%lf, %lf) R:(%lf, %lf), (%lf, %lf)",
		       &t.top, &t.bottom,
		       &t.left.p1.x, &t.left.p1.y,
		       &t.left.p2.x, &t.left.p2.y,
		       &t.right.p1.x, &t.right.p1.y,
		       &t.right.p2.x, &t.right.p2.y) == 10) {
		traps = _traps_add_trapezoid (tv, traps, &t);
	    } else {
		if (traps->num_traps) {
		    g_print ("read %d trapezoids\n", traps->num_traps);
		    g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
			    traps->extents.p1.x, traps->extents.p1.y,
			    traps->extents.p2.x, traps->extents.p2.y);
		    traps = traps_new (tv);
		}
	    }
	}
	free (line);
	fclose (file);

	if (traps->num_traps) {
	    g_print ("read %d trapezoids\n", traps->num_traps);
	    g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
		     traps->extents.p1.x, traps->extents.p1.y,
		     traps->extents.p2.x, traps->extents.p2.y);
	}
    }

#if 1
    if (argc >=4) {
	file = fopen (argv[3], "r");
	if (file != NULL) {
	    char *line = NULL;
	    size_t len = 0;
	    edges_t *edges;

	    edges = edges_new (tv);
	    while (getline (&line, &len, file) != -1) {
		edge_t e;

		if (sscanf (line,
			   "(%lf, %lf), (%lf, %lf) %lf %lf %d",
			   &e.line.p1.x, &e.line.p1.y,
			   &e.line.p2.x, &e.line.p2.y,
			   &e.top, &e.bottom,
			   &e.dir) == 7) {
		    edges = _edges_add_edge (tv, edges, &e);
		} else {
		    if (edges->num_edges) {
			g_print ("read %d edges\n", edges->num_edges);
			g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
				edges->extents.p1.x, edges->extents.p1.y,
				edges->extents.p2.x, edges->extents.p2.y);
			edges = edges_new (tv);
		    }
		}
	    }
	    free (line);
	    fclose (file);

	    if (edges->num_edges) {
		g_print ("read %d edges\n", edges->num_edges);
		g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
			 edges->extents.p1.x, edges->extents.p1.y,
			 edges->extents.p2.x, edges->extents.p2.y);
	    }
	}
    }
#else
    if (argc >= 4) {
	file = fopen (argv[3], "r");
	if (file != NULL) {
	    char *line = NULL;
	    size_t len = 0;
	    traps_t *traps;

	    tv = g_object_new (trap_view_get_type (), NULL);
	    gtk_box_pack_start (GTK_BOX (hbox), &tv->widget, TRUE, TRUE, 0);
	    gtk_widget_show (&tv->widget);

	    tv->group_prev = group_prev;
	    tv->group_next = NULL;
	    if (group_prev)
		group_prev->group_next = tv;
	    group_prev = tv;
	    if (group_head == NULL)
		group_head = tv;
	    tv->group_head = group_head;

	    traps = traps_new (tv);
	    while (getline (&line, &len, file) != -1) {
		trapezoid_t t;

		if (sscanf (line,
			    "%lf %lf L:(%lf, %lf), (%lf, %lf) R:(%lf, %lf), (%lf, %lf)",
			    &t.top, &t.bottom,
			    &t.left.p1.x, &t.left.p1.y,
			    &t.left.p2.x, &t.left.p2.y,
			    &t.right.p1.x, &t.right.p1.y,
			    &t.right.p2.x, &t.right.p2.y) == 10) {
		    traps = _traps_add_trapezoid (tv, traps, &t);
		} else {
		    if (traps->num_traps) {
			g_print ("read %d trapezoids\n", traps->num_traps);
			g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
				 traps->extents.p1.x, traps->extents.p1.y,
				 traps->extents.p2.x, traps->extents.p2.y);
			traps = traps_new (tv);
		    }
		}
	    }
	    free (line);
	    fclose (file);

	    if (traps->num_traps) {
		g_print ("read %d trapezoids\n", traps->num_traps);
		g_print ("extents=(%lg, %lg), (%lg, %lg)\n",
			 traps->extents.p1.x, traps->extents.p1.y,
			 traps->extents.p2.x, traps->extents.p2.y);
	    }
	}
    }
#endif

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (gtk_main_quit), NULL);
    gtk_widget_set_size_request (window, 512, 512);
    gtk_container_add (GTK_CONTAINER (window), hbox);
    gtk_widget_show (hbox);
    gtk_widget_show (window);

    gtk_main ();
    return 0;
}
