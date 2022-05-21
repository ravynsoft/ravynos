// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2020 the sway authors
 *
 * This file is only needed in support of
 *	- tracking damage
 *	- unconstraining XDG popups
 */

#include "labwc.h"

static void
xdg_popup_destroy(struct view_child *view_child)
{
	if (!view_child) {
		return;
	}
	struct xdg_popup *popup = (struct xdg_popup *)view_child;
	wl_list_remove(&popup->destroy.link);
	wl_list_remove(&popup->map.link);
	wl_list_remove(&popup->unmap.link);
	wl_list_remove(&popup->new_popup.link);
	view_child_finish(&popup->view_child);
	free(popup);
}

static void
handle_xdg_popup_map(struct wl_listener *listener, void *data)
{
	struct xdg_popup *popup = wl_container_of(listener, popup, map);
	damage_all_outputs(popup->view_child.parent->server);
}

static void
handle_xdg_popup_unmap(struct wl_listener *listener, void *data)
{
	struct xdg_popup *popup = wl_container_of(listener, popup, unmap);
	damage_all_outputs(popup->view_child.parent->server);
}

static void
handle_xdg_popup_destroy(struct wl_listener *listener, void *data)
{
	struct xdg_popup *popup = wl_container_of(listener, popup, destroy);
	struct view_child *view_child = (struct view_child *)popup;
	xdg_popup_destroy(view_child);
}

static void
popup_handle_new_xdg_popup(struct wl_listener *listener, void *data)
{
	struct xdg_popup *popup = wl_container_of(listener, popup, new_popup);
	struct wlr_xdg_popup *wlr_popup = data;
	xdg_popup_create(popup->view_child.parent, wlr_popup);
}

static void
popup_unconstrain(struct xdg_popup *popup)
{
	struct view *view = popup->view_child.parent;
	struct server *server = view->server;
	struct wlr_box *popup_box = &popup->wlr_popup->geometry;
	struct wlr_output_layout *output_layout = server->output_layout;
	struct wlr_output *wlr_output = wlr_output_layout_output_at(
		output_layout, view->x + popup_box->x, view->y + popup_box->y);
	struct wlr_box *output_box = wlr_output_layout_get_box(
		output_layout, wlr_output);

	struct wlr_box output_toplevel_box = {
		.x = output_box->x - view->x,
		.y = output_box->y - view->y,
		.width = output_box->width,
		.height = output_box->height,
	};
	wlr_xdg_popup_unconstrain_from_box(
		popup->wlr_popup, &output_toplevel_box);
}

void
xdg_popup_create(struct view *view, struct wlr_xdg_popup *wlr_popup)
{
	struct xdg_popup *popup = calloc(1, sizeof(struct xdg_popup));
	if (!popup) {
		return;
	}

	popup->wlr_popup = wlr_popup;
	view_child_init(&popup->view_child, view, wlr_popup->base->surface);

	popup->destroy.notify = handle_xdg_popup_destroy;
	wl_signal_add(&wlr_popup->base->events.destroy, &popup->destroy);
	popup->map.notify = handle_xdg_popup_map;
	wl_signal_add(&wlr_popup->base->events.map, &popup->map);
	popup->unmap.notify = handle_xdg_popup_unmap;
	wl_signal_add(&wlr_popup->base->events.unmap, &popup->unmap);
	popup->new_popup.notify = popup_handle_new_xdg_popup;
	wl_signal_add(&wlr_popup->base->events.new_popup, &popup->new_popup);

	popup_unconstrain(popup);
}
