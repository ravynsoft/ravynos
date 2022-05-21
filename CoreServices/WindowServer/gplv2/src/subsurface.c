// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2020 the sway authors
 * This file is only needed in support of tracking damage
 */

#include "labwc.h"

static void
subsurface_handle_destroy(struct wl_listener *listener, void *data)
{
	struct view_subsurface *subsurface = wl_container_of(listener,
		subsurface, destroy);
	struct view_child *view_child = (struct view_child *)subsurface;
	if (!view_child) {
		return;
	}
	wl_list_remove(&subsurface->destroy.link);
	view_child_finish(&subsurface->view_child);
	free(subsurface);
}

void
view_subsurface_create(struct view *view, struct wlr_subsurface *wlr_subsurface)
{
	struct view_subsurface *subsurface =
		calloc(1, sizeof(struct view_subsurface));
	if (!subsurface) {
		return;
	}
	view_child_init(&subsurface->view_child, view, wlr_subsurface->surface);
	subsurface->subsurface = wlr_subsurface;

	subsurface->destroy.notify = subsurface_handle_destroy;
	wl_signal_add(&wlr_subsurface->events.destroy, &subsurface->destroy);
}
