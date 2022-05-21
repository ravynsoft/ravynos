// SPDX-License-Identifier: GPL-2.0-only
#include "labwc.h"

static void
handle_toplevel_handle_request_minimize(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view,
		toplevel_handle_request_minimize);
	struct wlr_foreign_toplevel_handle_v1_minimized_event *event = data;
	if (view) {
		view_minimize(view, event->minimized);
	}
}

static void
handle_toplevel_handle_request_maximize(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view,
		toplevel_handle_request_maximize);
	struct wlr_foreign_toplevel_handle_v1_maximized_event *event = data;
	if (view) {
		view_maximize(view, event->maximized);
	}
}

static void
handle_toplevel_handle_request_fullscreen(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view,
		toplevel_handle_request_fullscreen);
	struct wlr_foreign_toplevel_handle_v1_fullscreen_event *event = data;
	if (view) {
		view_set_fullscreen(view, event->fullscreen, NULL);
	}
}

static void
handle_toplevel_handle_request_activate(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view,
		toplevel_handle_request_activate);
	// struct wlr_foreign_toplevel_handle_v1_activated_event *event = data;
	/* In a multi-seat world we would select seat based on event->seat here. */
	if (view) {
		desktop_focus_and_activate_view(&view->server->seat, view);
		desktop_move_to_front(view);
	}
}

static void
handle_toplevel_handle_request_close(struct wl_listener *listener, void *data)
{
	struct view *view = wl_container_of(listener, view,
		toplevel_handle_request_close);
	if (view) {
		view_close(view);
	}
}

void
foreign_toplevel_handle_create(struct view *view)
{
	view->toplevel_handle = wlr_foreign_toplevel_handle_v1_create(
		view->server->foreign_toplevel_manager);
	if (!view->toplevel_handle) {
		wlr_log(WLR_ERROR, "cannot create foreign toplevel handle for (%s)",
			view_get_string_prop(view, "title"));
		return;
	}

	struct wlr_output *wlr_output = view_wlr_output(view);
	if (!wlr_output) {
		wlr_log(WLR_ERROR, "no wlr_output for (%s)",
			view_get_string_prop(view, "title"));
		return;
	}
	wlr_foreign_toplevel_handle_v1_output_enter(view->toplevel_handle,
		wlr_output);

	view->toplevel_handle_request_maximize.notify =
		handle_toplevel_handle_request_maximize;
	wl_signal_add(&view->toplevel_handle->events.request_maximize,
		&view->toplevel_handle_request_maximize);

	view->toplevel_handle_request_minimize.notify =
		handle_toplevel_handle_request_minimize;
	wl_signal_add(&view->toplevel_handle->events.request_minimize,
		&view->toplevel_handle_request_minimize);

	view->toplevel_handle_request_fullscreen.notify =
		handle_toplevel_handle_request_fullscreen;
	wl_signal_add(&view->toplevel_handle->events.request_fullscreen,
		&view->toplevel_handle_request_fullscreen);

	view->toplevel_handle_request_activate.notify =
		handle_toplevel_handle_request_activate;
	wl_signal_add(&view->toplevel_handle->events.request_activate,
		&view->toplevel_handle_request_activate);

	view->toplevel_handle_request_close.notify =
		handle_toplevel_handle_request_close;
	wl_signal_add(&view->toplevel_handle->events.request_close,
		&view->toplevel_handle_request_close);

	/* TODO: hook up remaining signals (destroy) */
}
