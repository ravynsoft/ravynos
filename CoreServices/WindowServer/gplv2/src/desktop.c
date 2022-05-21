// SPDX-License-Identifier: GPL-2.0-only
#include "config.h"
#include <assert.h>
#include "labwc.h"
#include "layers.h"
#include "ssd.h"

static void
move_to_front(struct view *view)
{
	wl_list_remove(&view->link);
	wl_list_insert(&view->server->views, &view->link);
}

#if HAVE_XWAYLAND
static struct wlr_xwayland_surface *
top_parent_of(struct view *view)
{
	struct wlr_xwayland_surface *s = view->xwayland_surface;
	while (s->parent) {
		s = s->parent;
	}
	return s;
}

static void
move_xwayland_sub_views_to_front(struct view *parent)
{
	if (!parent || parent->type != LAB_XWAYLAND_VIEW) {
		return;
	}
	struct view *view, *next;
	wl_list_for_each_reverse_safe(view, next, &parent->server->views, link)
	{
		/* need to stop here, otherwise loops keeps going forever */
		if (view == parent) {
			break;
		}
		if (view->type != LAB_XWAYLAND_VIEW) {
			continue;
		}
		if (!view->mapped && !view->minimized) {
			continue;
		}
		if (top_parent_of(view) != parent->xwayland_surface) {
			continue;
		}
		move_to_front(view);
		/* TODO: we should probably focus on these too here */
	}
}
#endif

void
desktop_move_to_front(struct view *view)
{
	if (!view) {
		return;
	}
	move_to_front(view);
#if HAVE_XWAYLAND
	move_xwayland_sub_views_to_front(view);
#endif
}

static void
wl_list_insert_tail(struct wl_list *list, struct wl_list *elm)
{
	elm->prev = list->prev;
	elm->next = list;
	list->prev = elm;
	elm->prev->next = elm;
}

void
desktop_move_to_back(struct view *view)
{
	if (!view) {
		return;
	}
	wl_list_remove(&view->link);
	wl_list_insert_tail(&view->server->views, &view->link);
}

static void
deactivate_all_views(struct server *server)
{
	struct view *view;
	wl_list_for_each (view, &server->views, link) {
		if (!view->mapped) {
			continue;
		}
		view_set_activated(view, false);
	}
}

void
desktop_focus_and_activate_view(struct seat *seat, struct view *view)
{
	if (!view) {
		seat_focus_surface(seat, NULL);
		return;
	}

	/*
	 * Guard against views with no mapped surfaces when handling
	 * 'request_activate' and 'request_minimize'.
	 * See notes by isfocusable()
	 */
	if (!view->surface) {
		return;
	}

	if (input_inhibit_blocks_surface(seat, view->surface->resource)) {
		return;
	}

	if (view->minimized) {
		/*
		 * Unminimizing will map the view which triggers a call to this
		 * function again.
		 */
		view_minimize(view, false);
		return;
	}

	if (!view->mapped) {
		return;
	}

	struct wlr_surface *prev_surface;
	prev_surface = seat->seat->keyboard_state.focused_surface;

	/* Do not re-focus an already focused surface. */
	if (prev_surface == view->surface) {
		return;
	}

	deactivate_all_views(view->server);
	view_set_activated(view, true);
	seat_focus_surface(seat, view->surface);
}

/*
 * Some xwayland apps produce unmapped surfaces on startup and also leave
 * some unmapped surfaces kicking around on 'close' (for example leafpad's
 * "about" dialogue). Whilst this is not normally a problem, we have to be
 * careful when cycling between views. The only views we should focus are
 * those that are already mapped and those that have been minimized.
 */
bool
isfocusable(struct view *view)
{
	/* filter out those xwayland surfaces that have never been mapped */
	if (!view->surface) {
		return false;
	}
	return (view->mapped || view->minimized);
}

static bool
has_focusable_view(struct wl_list *wl_list)
{
	struct view *view;
	wl_list_for_each (view, wl_list, link) {
		if (isfocusable(view)) {
			return true;
		}
	}
	return false;
}

static struct view *
first_view(struct server *server)
{
	struct view *view;
	view = wl_container_of(server->views.next, view, link);
	return view;
}

struct view *
desktop_cycle_view(struct server *server, struct view *current,
		enum lab_cycle_dir dir)
{
	if (!has_focusable_view(&server->views)) {
		return NULL;
	}

	struct view *view = current ? current : first_view(server);
	if (dir == LAB_CYCLE_DIR_FORWARD) {
		/* Replacement for wl_list_for_each_from() */
		do {
			view = wl_container_of(view->link.next, view, link);
		} while (&view->link == &server->views || !isfocusable(view));
	} else if (dir == LAB_CYCLE_DIR_BACKWARD) {
		do {
			view = wl_container_of(view->link.prev, view, link);
		} while (&view->link == &server->views || !isfocusable(view));
	}
	damage_all_outputs(server);
	return view;
}

static bool
has_mapped_view(struct wl_list *wl_list)
{
	struct view *view;
	wl_list_for_each (view, wl_list, link) {
		if (view->mapped) {
			return true;
		}
	}
	return false;
}

static struct view *
topmost_mapped_view(struct server *server)
{
	if (!has_mapped_view(&server->views)) {
		return NULL;
	}

	/* start from tail of server->views */
	struct view *view = wl_container_of(server->views.prev, view, link);
	do {
		view = wl_container_of(view->link.next, view, link);
	} while (&view->link == &server->views || !view->mapped);
	return view;
}

struct view *
desktop_focused_view(struct server *server)
{
	struct seat *seat = &server->seat;
	struct wlr_surface *focused_surface;
	focused_surface = seat->seat->keyboard_state.focused_surface;
	if (!focused_surface) {
		return NULL;
	}
	struct view *view;
	wl_list_for_each (view, &server->views, link) {
		if (view->surface == focused_surface) {
			return view;
		}
	}

	return NULL;
}

void
desktop_focus_topmost_mapped_view(struct server *server)
{
	struct view *view = topmost_mapped_view(server);
	desktop_focus_and_activate_view(&server->seat, view);
	desktop_move_to_front(view);
}

static bool
_view_at(struct view *view, double lx, double ly, struct wlr_surface **surface,
	 double *sx, double *sy)
{
	/*
	 * XDG toplevels may have nested surfaces, such as popup windows for
	 * context menus or tooltips. This function tests if any of those are
	 * underneath the coordinates lx and ly (in output Layout Coordinates).
	 * If so, it sets the surface pointer to that wlr_surface and the sx and
	 * sy coordinates to the coordinates relative to that surface's top-left
	 * corner.
	 */
	double view_sx = lx - view->x;
	double view_sy = ly - view->y;
	double _sx, _sy;
	struct wlr_surface *_surface = NULL;

	switch (view->type) {
	case LAB_XDG_SHELL_VIEW:
		_surface = wlr_xdg_surface_surface_at(
			view->xdg_surface, view_sx, view_sy, &_sx, &_sy);
		break;
#if HAVE_XWAYLAND
	case LAB_XWAYLAND_VIEW:
		_surface = wlr_surface_surface_at(view->surface, view_sx,
						  view_sy, &_sx, &_sy);
		break;
#endif
	}

	if (_surface) {
		*sx = _sx;
		*sy = _sy;
		*surface = _surface;
		return true;
	}
	return false;
}

static struct
wlr_surface *layer_surface_at(struct wl_list *layer, double ox, double oy,
		double *sx, double *sy)
{
	struct lab_layer_surface *surface;
	wl_list_for_each_reverse(surface, layer, link) {
		double _sx = ox - surface->geo.x;
		double _sy = oy - surface->geo.y;
		struct wlr_surface *wlr_surface;
		wlr_surface = wlr_layer_surface_v1_surface_at(
			surface->layer_surface, _sx, _sy, sx, sy);
		if (wlr_surface) {
			return wlr_surface;
		}
	}
	return NULL;
}

static bool
surface_is_xdg_popup(struct wlr_surface *surface)
{
	if (wlr_surface_is_xdg_surface(surface)) {
		struct wlr_xdg_surface *xdg_surface =
			wlr_xdg_surface_from_wlr_surface(surface);
		return xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP;
	}
	return false;
}

static struct wlr_surface *
layer_surface_popup_at(struct output *output, struct wl_list *layer,
		double ox, double oy, double *sx, double *sy)
{
	struct lab_layer_surface *lab_layer;
	wl_list_for_each_reverse(lab_layer, layer, link) {
		double _sx = ox - lab_layer->geo.x;
		double _sy = oy - lab_layer->geo.y;
		struct wlr_surface *sub = wlr_layer_surface_v1_surface_at(
			lab_layer->layer_surface, _sx, _sy, sx, sy);
		if (sub && surface_is_xdg_popup(sub)) {
			return sub;
		}
	}
	return NULL;
}

struct view *
desktop_surface_and_view_at(struct server *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy,
		enum ssd_part_type *view_area)
{
	struct wlr_output *wlr_output = wlr_output_layout_output_at(
			server->output_layout, lx, ly);
	struct output *output = output_from_wlr_output(server, wlr_output);

	if (!output) {
		return NULL;
	}

	double ox = lx, oy = ly;
	wlr_output_layout_output_coords(output->server->output_layout,
		wlr_output, &ox, &oy);

	/* Overlay-layer */
	*surface = layer_surface_at(
			&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY],
			ox, oy, sx, sy);
	if (*surface) {
		return NULL;
	}

	/* Check all layer popups */
	*surface = layer_surface_popup_at(output,
			&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP],
			ox, oy, sx, sy);
	if (*surface) {
		return NULL;
	}
	*surface = layer_surface_popup_at(output,
			&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM],
			ox, oy, sx, sy);
	if (*surface) {
		return NULL;
	}
	*surface = layer_surface_popup_at(output,
			&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND],
			ox, oy, sx, sy);
	if (*surface) {
		return NULL;
	}

	/* Top-layer */
	*surface = layer_surface_at(
			&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP],
			ox, oy, sx, sy);
	if (*surface) {
		return NULL;
	}

	struct view *view;
	wl_list_for_each (view, &server->views, link) {
		if (!view->mapped) {
			continue;
		}

		/* This ignores server-side deco */
		if (_view_at(view, lx, ly, surface, sx, sy)) {
			*view_area = LAB_SSD_CLIENT;
			return view;
		}
		if (!view->ssd.enabled) {
			continue;
		}

		/* Now, let's deal with the server-side deco */
		*view_area = ssd_at(view, lx, ly);
		if (*view_area != LAB_SSD_NONE) {
			return view;
		}
	}

	*surface = layer_surface_at(
			&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM],
			ox, oy, sx, sy);
	if (*surface) {
		return NULL;
	}
	*surface = layer_surface_at(
			&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND],
			ox, oy, sx, sy);
	if (*surface) {
		return NULL;
	}
	return NULL;
}

struct view *
desktop_view_at_cursor(struct server *server)
{
	double sx, sy;
	struct wlr_surface *surface;
	enum ssd_part_type view_area = LAB_SSD_NONE;

	return desktop_surface_and_view_at(server,
			server->seat.cursor->x, server->seat.cursor->y,
			&surface, &sx, &sy, &view_area);
}
