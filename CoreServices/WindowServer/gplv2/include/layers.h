/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_LAYERS_H
#define __LABWC_LAYERS_H
#include <wayland-server.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_layer_shell_v1.h>

struct server;

enum layer_parent {
	LAYER_PARENT_LAYER,
	LAYER_PARENT_POPUP,
};

struct lab_layer_surface {
	struct wlr_layer_surface_v1 *layer_surface;
	struct wl_list link;

	struct wl_listener destroy;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener surface_commit;
	struct wl_listener output_destroy;
	struct wl_listener new_popup;
	struct wl_listener new_subsurface;

	struct wlr_box geo;
	bool mapped;
	/* TODO: add extent and layer */
	struct server *server;
};

struct lab_layer_popup {
	struct wlr_xdg_popup *wlr_popup;
	enum layer_parent parent_type;
	union {
		struct lab_layer_surface *parent_layer;
		struct lab_layer_popup *parent_popup;
	};
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
	struct wl_listener commit;
	struct wl_listener new_popup;
};

struct lab_layer_subsurface {
	struct wlr_subsurface *wlr_subsurface;
	struct lab_layer_surface *layer_surface;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
	struct wl_listener commit;
};

void layers_init(struct server *server);

#endif /* __LABWC_LAYERS_H */
