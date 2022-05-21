// SPDX-License-Identifier: GPL-2.0-only
#include "labwc.h"

struct xdg_deco {
	struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration;
	struct server *server;
	struct view *view;
	struct wl_listener destroy;
	struct wl_listener request_mode;
};

static void
xdg_deco_destroy(struct wl_listener *listener, void *data)
{
	struct xdg_deco *xdg_deco =
		wl_container_of(listener, xdg_deco, destroy);
	wl_list_remove(&xdg_deco->destroy.link);
	wl_list_remove(&xdg_deco->request_mode.link);
	free(xdg_deco);
}

static void
xdg_deco_request_mode(struct wl_listener *listener, void *data)
{
	struct xdg_deco *xdg_deco = wl_container_of(listener, xdg_deco, request_mode);
	enum wlr_xdg_toplevel_decoration_v1_mode client_mode =
		xdg_deco->wlr_decoration->requested_mode;

	if (client_mode == WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_NONE) {
		client_mode = rc.xdg_shell_server_side_deco
			? WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
			: WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
	}

	wlr_xdg_toplevel_decoration_v1_set_mode(xdg_deco->wlr_decoration, client_mode);
	view_set_decorations(xdg_deco->view,
		client_mode == WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

void
xdg_toplevel_decoration(struct wl_listener *listener, void *data)
{
	struct server *server =
		wl_container_of(listener, server, xdg_toplevel_decoration);
	struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration = data;
	struct xdg_deco *xdg_deco = calloc(1, sizeof(struct xdg_deco));
	if (!xdg_deco) {
		return;
	}
	xdg_deco->wlr_decoration = wlr_decoration;
	xdg_deco->server = server;
	xdg_deco->view = wlr_decoration->surface->data;
	xdg_deco->destroy.notify = xdg_deco_destroy;
	wl_signal_add(&wlr_decoration->events.destroy, &xdg_deco->destroy);
	xdg_deco->request_mode.notify = xdg_deco_request_mode;
	wl_signal_add(&wlr_decoration->events.request_mode,
		      &xdg_deco->request_mode);

	xdg_deco_request_mode(&xdg_deco->request_mode, wlr_decoration);
}
