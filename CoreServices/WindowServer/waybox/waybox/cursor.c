#include "waybox/cursor.h"
#include "waybox/xdg_shell.h"

static void process_cursor_move(struct wb_server *server) {
	/* Move the grabbed view to the new position. */
	struct wb_view *view = server->grabbed_view;
	if (view->scene_node->parent->type == WLR_SCENE_NODE_ROOT) {
		view->current_position.x = server->cursor->cursor->x - server->grab_x;
		view->current_position.y = server->cursor->cursor->y - server->grab_y;
		wlr_scene_node_set_position(view->scene_node,
				view->current_position.x, view->current_position.y);
	}
}

static void process_cursor_resize(struct wb_server *server) {
	struct wb_view *view = server->grabbed_view;
	double border_x = server->cursor->cursor->x - server->grab_x;
	double border_y = server->cursor->cursor->y - server->grab_y;
	int new_left = server->grab_geo_box.x;
	int new_right = server->grab_geo_box.x + server->grab_geo_box.width;
	int new_top = server->grab_geo_box.y;
	int new_bottom = server->grab_geo_box.y + server->grab_geo_box.height;

	if (server->resize_edges & WLR_EDGE_TOP) {
		new_top = border_y;
		if (new_top >= new_bottom) {
			new_top = new_bottom - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_BOTTOM) {
		new_bottom = border_y;
		if (new_bottom <= new_top) {
			new_bottom = new_top + 1;
		}
	}
	if (server->resize_edges & WLR_EDGE_LEFT) {
		new_left = border_x;
		if (new_left >= new_right) {
			new_left = new_right - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_RIGHT) {
		new_right = border_x;
		if (new_right <= new_left) {
			new_right = new_left + 1;
		}
	}

	struct wlr_box geo_box;
	wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);
	view->current_position.x = new_left - geo_box.x;
	view->current_position.y = new_top - geo_box.y;

	int new_width = new_right - new_left;
	int new_height = new_bottom - new_top;
#if WLR_CHECK_VERSION(0, 16, 0)
	wlr_xdg_toplevel_set_size(view->xdg_toplevel, new_width, new_height);
#else
	wlr_xdg_toplevel_set_size(view->xdg_surface, new_width, new_height);
#endif
}

static void process_cursor_motion(struct wb_server *server, uint32_t time) {
	/* If the mode is non-passthrough, delegate to those functions. */
	if (server->cursor->cursor_mode == WB_CURSOR_MOVE) {
		process_cursor_move(server);
		return;
	} else if (server->cursor->cursor_mode == WB_CURSOR_RESIZE) {
		process_cursor_resize(server);
		return;
	}

	/* Otherwise, find the view under the pointer and send the event along. */
	double sx, sy;
	struct wlr_seat *seat = server->seat->seat;
	struct wlr_surface *surface = NULL;
	struct wb_view *view = get_view_at(server,
			server->cursor->cursor->x, server->cursor->cursor->y, &surface, &sx, &sy);
	if (!view) {
		/* If there's no view under the cursor, set the cursor image to a
		 * default. This is what makes the cursor image appear when you move it
		 * around the screen, not over any views. */
		wlr_xcursor_manager_set_cursor_image(
				server->cursor->xcursor_manager, "left_ptr", server->cursor->cursor);
	}
	if (surface) {
		/*
		 * "Enter" the surface if necessary. This lets the client know that the
		 * cursor has entered one of its surfaces.
		 *
		 * Note that wlroots will avoid sending duplicate enter/motion events if
		 * the surface has already has pointer focus or if the client is already
		 * aware of the coordinates passed.
		 */
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	} else {
		/* Clear pointer focus so future button events and such are not sent to
		 * the last client to have the cursor over it. */
		wlr_seat_pointer_clear_focus(seat);
	}
}

static void handle_cursor_motion(struct wl_listener *listener, void *data) {
	struct wb_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
	struct wlr_event_pointer_motion *event = data;
	wlr_cursor_move(cursor->cursor, event->device, event->delta_x, event->delta_y);
	process_cursor_motion(cursor->server, event->time_msec);
}

static void handle_cursor_motion_absolute(struct wl_listener *listener, void *data) {
	struct wb_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_absolute);
	struct wlr_event_pointer_motion_absolute *event = data;
	wlr_cursor_warp_absolute(cursor->cursor, event->device, event->x, event->y);
	process_cursor_motion(cursor->server, event->time_msec);
}

static void handle_cursor_button(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a button
	 * event. */
	struct wb_cursor *cursor =
		wl_container_of(listener, cursor, cursor_button);
	struct wlr_event_pointer_button *event = data;
	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(cursor->server->seat->seat,
			event->time_msec, event->button, event->state);
	double sx, sy;
	struct wlr_surface *surface = NULL;
	struct wb_view *view = get_view_at(cursor->server,
			cursor->server->cursor->cursor->x, cursor->server->cursor->cursor->y, &surface, &sx, &sy);
	if (event->state == WLR_BUTTON_RELEASED) {
		/* If you released any buttons, we exit interactive move/resize mode. */
		cursor->cursor_mode = WB_CURSOR_PASSTHROUGH;
	} else {
		/* Focus that client if the button was _pressed_ */
		focus_view(view, surface);
	}
}

static void handle_cursor_axis(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an axis event,
	 * for example when you move the scroll wheel. */
	struct wb_cursor *cursor =
		wl_container_of(listener, cursor, cursor_axis);
	struct wlr_event_pointer_axis *event = data;
	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(cursor->server->seat->seat,
			event->time_msec, event->orientation, event->delta,
			event->delta_discrete, event->source);
}

static void handle_cursor_frame(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an frame
	 * event. Frame events are sent after regular pointer events to group
	 * multiple events together. For instance, two axis events may happen at the
	 * same time, in which case a frame event won't be sent in between. */
	struct wb_cursor *cursor =
		wl_container_of(listener, cursor, cursor_frame);
	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(cursor->server->seat->seat);
}

static void handle_cursor_request(struct wl_listener *listener, void *data) {
	struct wb_cursor *cursor = wl_container_of(
			listener, cursor, request_cursor);
	/* This event is raised by the seat when a client provides a cursor image */
	struct wlr_seat_pointer_request_set_cursor_event *event = data;
	struct wlr_seat_client *focused_client =
		cursor->server->seat->seat->pointer_state.focused_client;
	/* This can be sent by any client, so we check to make sure this one is
	 * actually has pointer focus first. */
	if (focused_client == event->seat_client) {
		/* Once we've vetted the client, we can tell the cursor to use the
		 * provided surface as the cursor image. It will set the hardware cursor
		 * on the output that it's currently on and continue to do so as the
		 * cursor moves between outputs. */
		wlr_cursor_set_surface(cursor->cursor, event->surface,
				event->hotspot_x, event->hotspot_y);
	}
}

struct wb_cursor *wb_cursor_create(struct wb_server *server) {
	struct wb_cursor *cursor = malloc(sizeof(struct wb_cursor));
	cursor->cursor = wlr_cursor_create();
	cursor->server = server;

	const char *xcursor_size = getenv("XCURSOR_SIZE");
	cursor->xcursor_manager = wlr_xcursor_manager_create(getenv("XCURSOR_THEME"),
				xcursor_size ? strtoul(xcursor_size, (char **) NULL, 10) : 24);
	wlr_xcursor_manager_load(cursor->xcursor_manager, 1);

	cursor->cursor_motion.notify = handle_cursor_motion;
	wl_signal_add(&cursor->cursor->events.motion, &cursor->cursor_motion);

	cursor->cursor_motion_absolute.notify = handle_cursor_motion_absolute;
	wl_signal_add(&cursor->cursor->events.motion_absolute, &cursor->cursor_motion_absolute);

	cursor->cursor_button.notify = handle_cursor_button;
	wl_signal_add(&cursor->cursor->events.button, &cursor->cursor_button);

	cursor->cursor_axis.notify = handle_cursor_axis;
	wl_signal_add(&cursor->cursor->events.axis, &cursor->cursor_axis);

	cursor->cursor_frame.notify = handle_cursor_frame;
	wl_signal_add(&cursor->cursor->events.frame, &cursor->cursor_frame);

	cursor->request_cursor.notify = handle_cursor_request;
	wl_signal_add(&server->seat->seat->events.request_set_cursor,
			&cursor->request_cursor);

	wlr_cursor_attach_output_layout(cursor->cursor, server->output_layout);

	return cursor;
}

void wb_cursor_destroy(struct wb_cursor *cursor) {
	if (!cursor) {
		return;
	}

	wlr_xcursor_manager_destroy(cursor->xcursor_manager);
	wlr_cursor_destroy(cursor->cursor);
	free(cursor);
}
