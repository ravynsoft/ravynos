#ifndef _WB_OUTPUT_H
#define _WB_OUTPUT_H

#include <stdlib.h>
#include <time.h>

#include "waybox/server.h"

struct wb_output {
	struct wlr_output *wlr_output;
	struct wb_server *server;

	struct {
		struct wlr_scene_node *shell_background;
		struct wlr_scene_node *shell_bottom;
		struct wlr_scene_node *shell_fullscreen;
		struct wlr_scene_node *shell_overlay;
		struct wlr_scene_node *shell_top;
	} layers;

	struct wlr_scene_rect *background;
	struct wlr_box geometry;

	struct wl_listener destroy;
	struct wl_listener frame;

	struct wl_list link;
};

struct wb_view {
	struct wl_list link;
	struct wb_server *server;
	struct wlr_xdg_toplevel *xdg_toplevel;
#if !WLR_CHECK_VERSION(0, 16, 0)
	struct wlr_xdg_surface *xdg_surface;
#endif
	struct wlr_scene_node *scene_node;

	struct wlr_xdg_toplevel_decoration_v1 *decoration;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
	struct wl_listener new_popup;
	struct wl_listener request_maximize;
	struct wl_listener request_minimize;
	struct wl_listener request_move;
	struct wl_listener request_resize;

	struct wlr_box current_position;
	struct wlr_box previous_position;
};

void output_frame_notify(struct wl_listener* listener, void *data);
void output_destroy_notify(struct wl_listener* listener, void *data);
void new_output_notify(struct wl_listener* listener, void *data);

#endif /* output.h */
