#ifndef _WB_LAYERS_H
#define _WB_LAYERS_H
#include <wlr/types/wlr_layer_shell_v1.h>

struct wb_server;

struct wb_layer_surface {
	struct wb_output *output;
	struct wb_server *server;

#if WLR_CHECK_VERSION(0, 16, 0)
	struct wlr_scene_layer_surface_v1 *scene;
#else
	struct {
		struct wlr_layer_surface_v1 *layer_surface;
		struct wlr_scene_node *node;
	} *scene;
#endif

	bool mapped;

	struct wl_listener destroy;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener surface_commit;
	struct wl_listener new_popup;
};

struct wb_layer_popup {
	struct wlr_xdg_popup *wlr_popup;
	struct wlr_scene_node *scene;

	struct wl_listener destroy;
	struct wl_listener new_popup;
};

struct wb_layer_subsurface {
	struct wlr_scene_node *scene;

	struct wl_listener destroy;
};

enum wb_scene_descriptor_type {
	WB_SCENE_DESC_NODE,
	WB_SCENE_DESC_LAYER_SHELL,
	WB_SCENE_DESC_LAYER_SHELL_POPUP,
};

struct wb_scene_descriptor {
	enum wb_scene_descriptor_type type;
	void *data;
	struct wl_listener destroy;
};

void init_layer_shell(struct wb_server *server);
void assign_scene_descriptor(struct wlr_scene_node *node,
	enum wb_scene_descriptor_type type, void *data);

#endif
