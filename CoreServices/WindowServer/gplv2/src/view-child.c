// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2020 the sway authors
 * This file is only needed in support of tracking damage
 */

#include "labwc.h"

static void
view_child_handle_commit(struct wl_listener *listener, void *data)
{
	struct view_child *child = wl_container_of(listener, child, commit);
	damage_all_outputs(child->parent->server);
}

static void
view_child_handle_new_subsurface(struct wl_listener *listener, void *data)
{
	struct view_child *child;
	child = wl_container_of(listener, child, new_subsurface);
	struct wlr_subsurface *wlr_subsurface = data;
	view_subsurface_create(child->parent, wlr_subsurface);
}

void
view_child_finish(struct view_child *child)
{
	wl_list_remove(&child->commit.link);
	wl_list_remove(&child->new_subsurface.link);
}

void
view_child_init(struct view_child *child, struct view *view,
		struct wlr_surface *wlr_surface)
{
	child->parent = view;
	child->surface = wlr_surface;
	child->commit.notify = view_child_handle_commit;
	wl_signal_add(&wlr_surface->events.commit, &child->commit);
	child->new_subsurface.notify = view_child_handle_new_subsurface;
	wl_signal_add(&wlr_surface->events.new_subsurface, &child->new_subsurface);
}
