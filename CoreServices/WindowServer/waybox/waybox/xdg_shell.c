#include "waybox/xdg_shell.h"

struct wb_view *get_view_at(
		struct wb_server *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy) {
	/* This returns the topmost node in the scene at the given layout coords.
	 * we only care about surface nodes as we are specifically looking for a
	 * surface in the surface tree of a wb_view. */
	struct wlr_scene_node *node = wlr_scene_node_at(
		&server->scene->node, lx, ly, sx, sy);
	if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
		return NULL;
	}
	*surface = wlr_scene_surface_from_node(node)->surface;
	/* Find the node corresponding to the tinywl_view at the root of this
	 * surface tree, it is the only one for which we set the data field. */
	while (node != NULL && node->data == NULL) {
		node = node->parent;
	}
	return node->data;
}

void focus_view(struct wb_view *view, struct wlr_surface *surface) {
	/* Note: this function only deals with keyboard focus. */
	if (view == NULL || surface == NULL || !wlr_surface_is_xdg_surface(surface)) {
		return;
	}

	struct wlr_xdg_surface *xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);
	if (xdg_surface)
		wlr_log(WLR_INFO, "%s: %s", _("Keyboard focus is now on surface"),
				xdg_surface->toplevel->app_id);

	struct wb_server *server = view->server;
	struct wlr_seat *seat = server->seat->seat;
	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	if (prev_surface == surface) {
		/* Don't re-focus an already focused surface. */
		return;
	}
	if (prev_surface && wlr_surface_is_xdg_surface(prev_surface)) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		struct wlr_xdg_surface *previous = wlr_xdg_surface_from_wlr_surface(
					prev_surface);
#if WLR_CHECK_VERSION(0, 16, 0)
		wlr_xdg_toplevel_set_activated(previous->toplevel, false);
#else
		wlr_xdg_toplevel_set_activated(previous, false);
#endif
	}
	/* Move the view to the front */
	if (!server->seat->focused_layer) {
		wlr_scene_node_raise_to_top(view->scene_node);
	}
	wl_list_remove(&view->link);
	wl_list_insert(&server->views, &view->link);
	/* Activate the new surface */
#if WLR_CHECK_VERSION(0, 16, 0)
	wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);
#else
	wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
#endif
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	seat_focus_surface(server->seat, view->xdg_toplevel->base->surface);
}

struct wlr_output *get_active_output(struct wb_view *view) {
	double closest_x, closest_y;
	struct wlr_output *output = NULL;
	wlr_output_layout_closest_point(view->server->output_layout, output,
			view->current_position.x + view->current_position.width / 2,
			view->current_position.y + view->current_position.height / 2,
			&closest_x, &closest_y);
        output = wlr_output_layout_output_at(view->server->output_layout, closest_x, closest_y);
        return output;
}

static struct wlr_box get_usable_area(struct wb_view *view) {
	struct wlr_output *output = get_active_output(view);
	struct wlr_box usable_area = {0};
	wlr_output_effective_resolution(output, &usable_area.width, &usable_area.height);
	return usable_area;
}

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct wb_view *view = wl_container_of(listener, view, map);
	if (view->xdg_toplevel->base->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
		return;

	struct wb_config *config = view->server->config;
	struct wlr_box geo_box = {0};
	struct wlr_box usable_area = get_usable_area(view);
	wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);

	if (config) {
		view->current_position.height = MIN(geo_box.height,
				usable_area.height - config->margins.top - config->margins.bottom);
		view->current_position.width = MIN(geo_box.width,
				usable_area.width - config->margins.left - config->margins.right);
		view->current_position.x = config->margins.left;
		view->current_position.y = config->margins.top;
	} else {
		view->current_position.height = MIN(geo_box.height, usable_area.height);
		view->current_position.width = MIN(geo_box.width, usable_area.width);
		view->current_position.x = 0;
		view->current_position.y = 0;
	}

	/* A view no larger than a title bar shouldn't be sized or focused */
	if (view->current_position.height > TITLEBAR_HEIGHT &&
			view->current_position.height > TITLEBAR_HEIGHT *
			(usable_area.width / usable_area.height)) {
#if WLR_CHECK_VERSION(0, 16, 0)
		wlr_xdg_toplevel_set_size(view->xdg_toplevel,
				view->current_position.width, view->current_position.height);
#else
		wlr_xdg_toplevel_set_size(view->xdg_surface,
				view->current_position.width, view->current_position.height);
#endif
		focus_view(view, view->xdg_toplevel->base->surface);
	}

	wlr_scene_node_set_position(view->scene_node,
			view->current_position.x, view->current_position.y);
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct wb_view *view = wl_container_of(listener, view, unmap);
	if (view->xdg_toplevel->base->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
		return;

	/* Focus the next view, if any. */
	struct wb_view *next_view = wl_container_of(view->link.next, next_view, link);
	if (next_view && next_view->scene_node && next_view->scene_node->state.enabled) {
		wlr_log(WLR_INFO, "%s: %s", _("Focusing next view"),
				next_view->xdg_toplevel->app_id);
		focus_view(next_view, next_view->xdg_toplevel->base->surface);
	}
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
	/* Called when the surface is destroyed and should never be shown again. */
	struct wb_view *view = wl_container_of(listener, view, destroy);

	wl_list_remove(&view->map.link);
	wl_list_remove(&view->unmap.link);
	wl_list_remove(&view->destroy.link);
	wl_list_remove(&view->new_popup.link);

	if (view->xdg_toplevel->base->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		wl_list_remove(&view->request_minimize.link);
		wl_list_remove(&view->request_maximize.link);
		wl_list_remove(&view->request_move.link);
		wl_list_remove(&view->request_resize.link);
		wl_list_remove(&view->link);
	}

	free(view);
}

static void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
	struct wb_view *view = wl_container_of(listener, view, request_maximize);
	struct wlr_box usable_area = get_usable_area(view);

	bool is_maximized = view->xdg_toplevel->current.maximized;
	if (!is_maximized) {
		struct wb_config *config = view->server->config;
		view->previous_position = view->current_position;
		if (config) {
			view->current_position.x = config->margins.left;
			view->current_position.y = config->margins.top;
			usable_area.height -= config->margins.top + config->margins.bottom;
			usable_area.width -= config->margins.left + config->margins.right;
		} else {
			view->current_position.x = 0;
			view->current_position.y = 0;
		}
	} else {
		usable_area = view->previous_position;
		view->current_position.x = view->previous_position.x;
		view->current_position.y = view->previous_position.y;
	}
#if WLR_CHECK_VERSION(0, 16, 0)
	wlr_xdg_toplevel_set_size(view->xdg_toplevel, usable_area.width, usable_area.height);
	wlr_xdg_toplevel_set_maximized(view->xdg_toplevel, !is_maximized);
#else
	wlr_xdg_toplevel_set_size(view->xdg_surface, usable_area.width, usable_area.height);
	wlr_xdg_toplevel_set_maximized(view->xdg_surface, !is_maximized);
#endif
	wlr_scene_node_set_position(view->scene_node,
			view->current_position.x, view->current_position.y);
}

static void xdg_toplevel_request_minimize(struct wl_listener *listener, void *data) {
	struct wb_view *view = wl_container_of(listener, view, request_minimize);
	bool minimize_requested = view->xdg_toplevel->requested.minimized;
	if (minimize_requested) {
		view->previous_position = view->current_position;
		view->current_position.y = -view->current_position.height;
	} else {
		view->current_position = view->previous_position;
	}

	struct wb_view *parent_view = wl_container_of(view->link.prev, parent_view, link);
	struct wb_view *previous_view = wl_container_of(parent_view->link.prev, previous_view, link);
	focus_view(previous_view, previous_view->xdg_toplevel->base->surface);
	wlr_scene_node_set_position(view->scene_node,
			view->current_position.x, view->current_position.y);
}

static void begin_interactive(struct wb_view *view,
		enum wb_cursor_mode mode, uint32_t edges) {
	/* This function sets up an interactive move or resize operation, where the
	 * compositor stops propagating pointer events to clients and instead
	 * consumes them itself, to move or resize windows. */
	struct wb_server *server = view->server;
	struct wlr_surface *focused_surface =
		server->seat->seat->pointer_state.focused_surface;
	if (view->xdg_toplevel->base->surface != wlr_surface_get_root_surface(focused_surface)) {
		/* Deny move/resize requests from unfocused clients. */
		return;
	}
	server->grabbed_view = view;
	server->cursor->cursor_mode = mode;

	if (mode == WB_CURSOR_MOVE) {
		server->grab_x = server->cursor->cursor->x - view->current_position.x;
		server->grab_y = server->cursor->cursor->y - view->current_position.y;
	} else if (mode == WB_CURSOR_RESIZE) {
		struct wlr_box geo_box;
		wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);

		double border_x = (view->current_position.x + geo_box.x) +
			((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
		double border_y = (view->current_position.y + geo_box.y) +
			((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
		server->grab_x = server->cursor->cursor->x - border_x;
		server->grab_y = server->cursor->cursor->y - border_y;

		server->grab_geo_box = geo_box;
		server->grab_geo_box.x += view->current_position.x;
		server->grab_geo_box.y += view->current_position.y;

		server->resize_edges = edges;
	}
}

static void xdg_toplevel_request_move(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a client would like to begin an interactive
	 * move, typically because the user clicked on their client-side
	 * decorations. */
	struct wb_view *view = wl_container_of(listener, view, request_move);
	begin_interactive(view, WB_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a client would like to begin an interactive
	 * resize, typically because the user clicked on their client-side
	 * decorations. */
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct wb_view *view = wl_container_of(listener, view, request_resize);
	begin_interactive(view, WB_CURSOR_RESIZE, event->edges);
}

static void handle_new_popup(struct wl_listener *listener, void *data) {
	struct wlr_xdg_popup *popup = data;
	struct wb_view *view = wl_container_of(listener, view, new_popup);

	struct wlr_output *wlr_output = wlr_output_layout_output_at(
			view->server->output_layout,
			view->current_position.x + popup->geometry.x,
			view->current_position.y + popup->geometry.y);
	if (!wlr_output) return;
	struct wb_output *output = wlr_output->data;

	struct wlr_box output_toplevel_box = {
		.x = output->geometry.x - view->current_position.x,
		.y = output->geometry.y - view->current_position.y,
		.width = output->geometry.width,
		.height = output->geometry.height,
	};
	wlr_xdg_popup_unconstrain_from_box(popup, &output_toplevel_box);
}

static void handle_new_xdg_surface(struct wl_listener *listener, void *data) {
	/* This event is raised when wlr_xdg_shell receives a new xdg surface from a
	 * client, either a toplevel (application window) or popup. */
	struct wb_server *server =
		wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface *xdg_surface = data;

	/* We must add xdg popups to the scene graph so they get rendered. The
	 * wlroots scene graph provides a helper for this, but to use it we must
	 * provide the proper parent scene node of the xdg popup. To enable this,
	 * we always set the user data field of xdg_surfaces to the corresponding
	 * scene node. */
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
		if (wlr_surface_is_xdg_surface(xdg_surface->popup->parent)) {
			struct wlr_xdg_surface *parent = wlr_xdg_surface_from_wlr_surface(
				xdg_surface->popup->parent);
				struct wlr_scene_node *parent_node = parent->data;
			xdg_surface->data = wlr_scene_xdg_surface_create(
				parent_node, xdg_surface);
		}
		/* The scene graph doesn't currently unconstrain popups, so keep going */
		/* return; */
	}
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_NONE)
		return;

	/* Allocate a wb_view for this surface */
	struct wb_view *view =
		calloc(1, sizeof(struct wb_view));
	view->server = server;
	view->xdg_toplevel = xdg_surface->toplevel;
#if !WLR_CHECK_VERSION(0, 16, 0)
	view->xdg_surface = xdg_surface;
#endif

	/* Listen to the various events it can emit */
	view->map.notify = xdg_toplevel_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = xdg_toplevel_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);
	view->new_popup.notify = handle_new_popup;
	wl_signal_add(&xdg_surface->events.new_popup, &view->new_popup);

	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		view->scene_node = wlr_scene_xdg_surface_create(
			&view->server->scene->node, view->xdg_toplevel->base);
		view->scene_node->data = view;
		xdg_surface->data = view->scene_node;

		struct wlr_xdg_toplevel *toplevel = view->xdg_toplevel;
		view->request_maximize.notify = xdg_toplevel_request_maximize;
		wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);
		view->request_minimize.notify = xdg_toplevel_request_minimize;
		wl_signal_add(&toplevel->events.request_minimize, &view->request_minimize);
		view->request_move.notify = xdg_toplevel_request_move;
		wl_signal_add(&toplevel->events.request_move, &view->request_move);
		view->request_resize.notify = xdg_toplevel_request_resize;
		wl_signal_add(&toplevel->events.request_resize, &view->request_resize);

		wl_list_insert(&view->server->views, &view->link);
	}
}

void init_xdg_shell(struct wb_server *server) {
	server->xdg_shell = wlr_xdg_shell_create(server->wl_display);
	server->new_xdg_surface.notify = handle_new_xdg_surface;
	wl_signal_add(&server->xdg_shell->events.new_surface, &server->new_xdg_surface);
}
