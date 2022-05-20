#ifndef _WB_SERVER_H
#define _WB_SERVER_H

#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))
#define TITLEBAR_HEIGHT 8 /* TODO: Get this from the theme */
#include <wlr/version.h>
#define WLR_CHECK_VERSION(major, minor, micro) (WLR_VERSION_NUM >= ((major << 16) | (minor << 8) | (micro)))

#include <wlr/backend.h>
#include <wlr/render/allocator.h>

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_screencopy_v1.h>
#if WLR_CHECK_VERSION(0, 16, 0)
#include <wlr/types/wlr_subcompositor.h>
#endif
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#ifdef USE_NLS
#	include <libintl.h>
#	include <locale.h>
#	define _ gettext
#else
#	define _(s) (s)
#endif

#include "config.h"
#include "waybox/cursor.h"
#include "decoration.h"
#include "layer_shell.h"
#include "waybox/output.h"
#include "waybox/seat.h"

struct wb_server {
	struct wl_display *wl_display;

	struct wlr_allocator *allocator;
	struct wlr_backend *backend;
	struct wlr_compositor *compositor;
	struct wlr_output_layout *output_layout;
	struct wlr_xdg_output_manager_v1 *output_manager;
	struct wlr_renderer *renderer;
	struct wlr_scene *scene;
#if WLR_CHECK_VERSION(0, 16, 0)
	struct wlr_subcompositor *subcompositor;
#endif

	struct wb_config *config;
	char *config_file;

	struct wb_cursor *cursor;
	struct wb_seat *seat;

	struct wb_view *grabbed_view;
	struct wlr_box grab_geo_box;
	double grab_x, grab_y;
	uint32_t resize_edges;
	struct wl_list views;

	struct wlr_layer_shell_v1 *layer_shell;
	struct wlr_xdg_shell *xdg_shell;

	struct wl_listener new_layer_surface;
	struct wl_listener new_xdg_surface;
	struct wl_listener new_xdg_decoration;

	struct wl_listener new_input;
	struct wl_listener new_output;
	struct wl_list outputs; /* wb_output::link */
};

bool wb_create_backend(struct wb_server *server);
bool wb_start_server(struct wb_server *server);
bool wb_terminate(struct wb_server *server);

#endif /* server.h */
