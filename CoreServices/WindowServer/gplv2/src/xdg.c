// SPDX-License-Identifier: GPL-2.0-only
#include <assert.h>
#include "labwc.h"
#include "ssd.h"

/*
 * xdg_popup_create() and view_subsurface_create() are only called for the
 * purposes of tracking damage.
 */
static void
handle_new_xdg_popup(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, new_popup);
	struct wlr_xdg_popup *wlr_popup = data;
	xdg_popup_create(view, wlr_popup);
}

static void
new_subsurface_notify(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, new_subsurface);
	struct wlr_subsurface *wlr_subsurface = data;
	view_subsurface_create(view, wlr_subsurface);
}

static bool
has_ssd(struct view *view)
{
	if (!rc.xdg_shell_server_side_deco) {
		return false;
	}

	/*
	 * Some XDG shells refuse to disable CSD in which case their
	 * geometry.{x,y} seems to be greater than zero. We filter on that
	 * on the assumption that this will remain true.
	 */
	if (view->xdg_surface->current.geometry.x || view->xdg_surface->current.geometry.y) {
		return false;
	}
	return true;
}

static void
handle_commit(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, commit);
	assert(view->surface);
	struct wlr_box size;
	wlr_xdg_surface_get_geometry(view->xdg_surface, &size);

	view->w = size.width;
	view->h = size.height;

	/* padding changes with maximize/unmaximize */
	view->padding.top = view->padding.bottom = size.y;
	view->padding.left = view->padding.right = size.x;

	uint32_t serial = view->pending_move_resize.configure_serial;
	if (serial > 0 && serial >= view->xdg_surface->current.configure_serial) {
		if (view->pending_move_resize.update_x) {
			view->x = view->pending_move_resize.x +
				view->pending_move_resize.width - size.width;
		}
		if (view->pending_move_resize.update_y) {
			view->y = view->pending_move_resize.y +
				view->pending_move_resize.height - size.height;
		}
		if (serial == view->xdg_surface->current.configure_serial) {
			view->pending_move_resize.configure_serial = 0;
		}
	}
	ssd_update_geometry(view, false);
	damage_view_part(view);
}

static void
handle_map(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, map);
	view->impl->map(view);
}

static void
handle_unmap(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, unmap);
	view->impl->unmap(view);
}

static void
handle_destroy(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, destroy);
	if (view->toplevel_handle) {
		wlr_foreign_toplevel_handle_v1_destroy(view->toplevel_handle);
	}
	interactive_end(view);
	wl_list_remove(&view->link);
	wl_list_remove(&view->destroy.link);
	ssd_destroy(view);
	free(view);
}

static void
handle_request_move(struct wl_listener *listener, void *data)
{
	/*
	 * This event is raised when a client would like to begin an interactive
	 * move, typically because the user clicked on their client-side
	 * decorations. Note that a more sophisticated compositor should check
	 * the provied serial against a list of button press serials sent to
	 * this
	 * client, to prevent the client from requesting this whenever they
	 * want.
	 */
	struct view *view = wl_container_of(listener, view, request_move);
	interactive_begin(view, LAB_INPUT_STATE_MOVE, 0);
}

static void
handle_request_resize(struct wl_listener *listener, void *data)
{
	/*
	 * This event is raised when a client would like to begin an interactive
	 * resize, typically because the user clicked on their client-side
	 * decorations. Note that a more sophisticated compositor should check
	 * the provied serial against a list of button press serials sent to
	 * this
	 * client, to prevent the client from requesting this whenever they
	 * want.
	 */
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct view *view = wl_container_of(listener, view, request_resize);
	interactive_begin(view, LAB_INPUT_STATE_RESIZE, event->edges);
}

static void
handle_request_minimize(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, request_minimize);
	struct wlr_xdg_surface *surface = data;
	if (view) {
		view_minimize(view, surface->toplevel->requested.minimized);
	}
}

static void
handle_request_maximize(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, request_maximize);
	struct wlr_xdg_surface *surface = data;
	if (view) {
		view_maximize(view, surface->toplevel->requested.maximized);
	}

}

static void
handle_request_fullscreen(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, request_fullscreen);
	struct wlr_xdg_toplevel_set_fullscreen_event *e = data;
	view_set_fullscreen(view, e->fullscreen, e->output);
}

static void
handle_set_title(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, set_title);
	assert(view);
	view_update_title(view);
}

static void
handle_set_app_id(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view, set_app_id);
	assert(view);
	view_update_app_id(view);
}

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
static void
xdg_toplevel_view_configure(struct view *view, struct wlr_box geo)
{
	int min_width, min_height;
	view_min_size(view, &min_width, &min_height);

	view->pending_move_resize.update_x = geo.x != view->x;
	view->pending_move_resize.update_y = geo.y != view->y;
	view->pending_move_resize.x = geo.x;
	view->pending_move_resize.y = geo.y;
	view->pending_move_resize.width = MAX(geo.width, min_width);
	view->pending_move_resize.height = MAX(geo.height, min_height);

	uint32_t serial = wlr_xdg_toplevel_set_size(view->xdg_surface,
		(uint32_t)geo.width, (uint32_t)geo.height);
	if (serial > 0) {
		view->pending_move_resize.configure_serial = serial;
	} else if (view->pending_move_resize.configure_serial == 0) {
		view->x = geo.x;
		view->y = geo.y;
		ssd_update_geometry(view, false);
		damage_all_outputs(view->server);
	}
}
#undef MAX

static void
xdg_toplevel_view_move(struct view *view, double x, double y)
{
	view->x = x;
	view->y = y;
	ssd_update_geometry(view, false);
	damage_all_outputs(view->server);
}

static void
xdg_toplevel_view_close(struct view *view)
{
	wlr_xdg_toplevel_send_close(view->xdg_surface);
}

static void
xdg_toplevel_view_for_each_popup_surface(struct view *view,
		 wlr_surface_iterator_func_t iterator, void *data)
{
	wlr_xdg_surface_for_each_popup_surface(view->xdg_surface, iterator, data);
}

static void
xdg_toplevel_view_for_each_surface(struct view *view,
		wlr_surface_iterator_func_t iterator, void *data)
{
	wlr_xdg_surface_for_each_surface(view->xdg_surface, iterator, data);
}

static void
update_padding(struct view *view)
{
	struct wlr_box padding;
	wlr_xdg_surface_get_geometry(view->xdg_surface, &padding);
	view->padding.top = view->padding.bottom = padding.y;
	view->padding.left = view->padding.right = padding.x;
}

static void
xdg_toplevel_view_maximize(struct view *view, bool maximized)
{
	wlr_xdg_toplevel_set_maximized(view->xdg_surface, maximized);
}

static void
xdg_toplevel_view_set_activated(struct view *view, bool activated)
{
	struct wlr_xdg_surface *surface = view->xdg_surface;
	if (surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		wlr_xdg_toplevel_set_activated(surface, activated);
	}
}

static void
xdg_toplevel_view_set_fullscreen(struct view *view, bool fullscreen)
{
	wlr_xdg_toplevel_set_fullscreen(view->xdg_surface, fullscreen);
}

static bool
istopmost(struct view *view)
{
	return view->xdg_surface->toplevel->parent == NULL;
}

static struct view *
parent_of(struct view *view)
{
	struct view *p;
	wl_list_for_each (p, &view->server->views, link) {
		if (p->xdg_surface == view->xdg_surface->toplevel->parent) {
			return p;
		}
	}
	return NULL;
}

static void
position_xdg_toplevel_view(struct view *view)
{
	if (istopmost(view)) {
		struct wlr_box box =
			output_usable_area_from_cursor_coords(view->server);
		view->x = box.x;
		view->y = box.y;
		view->w = view->xdg_surface->current.geometry.width;
		view->h = view->xdg_surface->current.geometry.height;
		if (view->w && view->h) {
			view_center(view);
		}
	} else {
		/*
		 * If child-toplevel-views, we center-align relative to their
		 * parents
		 */
		struct view *parent = parent_of(view);
		assert(parent);
		int center_x = parent->x + parent->w / 2;
		int center_y = parent->y + parent->h / 2;
		view->x = center_x - view->xdg_surface->current.geometry.width / 2;
		view->y = center_y - view->xdg_surface->current.geometry.height / 2;
	}
	view->x += view->margin.left - view->padding.left;
	view->y += view->margin.top - view->padding.top;
}

static const char *
xdg_toplevel_view_get_string_prop(struct view *view, const char *prop)
{
	if (!strcmp(prop, "title")) {
		return view->xdg_surface->toplevel->title;
	}
	if (!strcmp(prop, "app_id")) {
		return view->xdg_surface->toplevel->app_id;
	}
	return "";
}

static void
xdg_toplevel_view_map(struct view *view)
{
	view->mapped = true;
	view->surface = view->xdg_surface->surface;
	if (!view->been_mapped) {
		struct wlr_xdg_toplevel_requested *requested =
			&view->xdg_surface->toplevel->requested;
		foreign_toplevel_handle_create(view);

		view->ssd.enabled = has_ssd(view);
		if (view->ssd.enabled) {
			view->margin = ssd_thickness(view);
			ssd_create(view);
		}

		update_padding(view);
		if (!view->fullscreen && requested->fullscreen) {
			view_set_fullscreen(view, true,
				requested->fullscreen_output);
		} else if (!view->maximized && requested->maximized) {
			view_maximize(view, true);
		}

		if (!view->maximized && !view->fullscreen) {
			position_xdg_toplevel_view(view);
		}

		struct wlr_subsurface *subsurface;
		wl_list_for_each(subsurface, &view->surface->current.subsurfaces_below,
				 current.link) {
			view_subsurface_create(view, subsurface);
		}
		wl_list_for_each(subsurface, &view->surface->current.subsurfaces_above,
				 current.link) {
			view_subsurface_create(view, subsurface);
		}
		view_discover_output(view);

		view->been_mapped = true;
	}

	view->commit.notify = handle_commit;
	wl_signal_add(&view->xdg_surface->surface->events.commit,
		&view->commit);
	view->new_subsurface.notify = new_subsurface_notify;
	wl_signal_add(&view->surface->events.new_subsurface,
		&view->new_subsurface);

	view_impl_map(view);
}

static void
xdg_toplevel_view_unmap(struct view *view)
{
	if (view->mapped) {
		view->mapped = false;
		damage_all_outputs(view->server);
		wl_list_remove(&view->commit.link);
		wl_list_remove(&view->new_subsurface.link);
		desktop_focus_topmost_mapped_view(view->server);
	}
}

static const struct view_impl xdg_toplevel_view_impl = {
	.configure = xdg_toplevel_view_configure,
	.close = xdg_toplevel_view_close,
	.for_each_popup_surface = xdg_toplevel_view_for_each_popup_surface,
	.for_each_surface = xdg_toplevel_view_for_each_surface,
	.get_string_prop = xdg_toplevel_view_get_string_prop,
	.map = xdg_toplevel_view_map,
	.move = xdg_toplevel_view_move,
	.set_activated = xdg_toplevel_view_set_activated,
	.set_fullscreen = xdg_toplevel_view_set_fullscreen,
	.unmap = xdg_toplevel_view_unmap,
	.maximize = xdg_toplevel_view_maximize,
};

void
xdg_surface_new(struct wl_listener *listener, void *data)
{
	struct server *server =
		wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface *xdg_surface = data;
	if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		return;
	}
	wlr_xdg_surface_ping(xdg_surface);

	struct view *view = calloc(1, sizeof(struct view));
	view->server = server;
	view->type = LAB_XDG_SHELL_VIEW;
	view->impl = &xdg_toplevel_view_impl;
	view->xdg_surface = xdg_surface;
	wl_list_init(&view->ssd.parts);

	xdg_surface->data = view;

	view->map.notify = handle_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = handle_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = handle_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	view->new_popup.notify = handle_new_xdg_popup;
	wl_signal_add(&xdg_surface->events.new_popup, &view->new_popup);

	struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
	view->request_move.notify = handle_request_move;
	wl_signal_add(&toplevel->events.request_move, &view->request_move);
	view->request_resize.notify = handle_request_resize;
	wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
	view->request_minimize.notify = handle_request_minimize;
	wl_signal_add(&toplevel->events.request_minimize, &view->request_minimize);
	view->request_maximize.notify = handle_request_maximize;
	wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);
	view->request_fullscreen.notify = handle_request_fullscreen;
	wl_signal_add(&toplevel->events.request_fullscreen, &view->request_fullscreen);

	view->set_title.notify = handle_set_title;
	wl_signal_add(&toplevel->events.set_title, &view->set_title);

	view->set_app_id.notify = handle_set_app_id;
	wl_signal_add(&toplevel->events.set_app_id, &view->set_app_id);

	wl_list_insert(&server->views, &view->link);
}
