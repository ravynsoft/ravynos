#include "decoration.h"

static void destroy_xdg_toplevel_decoration(struct wl_listener *listener, void *data) {
	struct wb_decoration *decoration = wl_container_of(listener, decoration, toplevel_decoration_destroy);
	wl_list_remove(&decoration->toplevel_decoration_destroy.link);
	free(decoration);
}

static void free_xdg_decoration_mode(struct wl_listener *listener, void *data) {
	struct wb_decoration *decoration = wl_container_of(listener, decoration, mode_destroy);
	wl_list_remove(&decoration->mode_destroy.link);
	wl_list_remove(&decoration->request_mode.link);
}

static void handle_xdg_decoration_mode(struct wl_listener *listener, void *data) {
	struct wlr_xdg_toplevel_decoration_v1 *toplevel_decoration = data;
	struct wb_decoration *decoration = wl_container_of(listener, decoration, request_mode);
	struct wb_view *view = wl_container_of(decoration->server->views.next, view, link);
	wlr_xdg_toplevel_decoration_v1_set_mode(toplevel_decoration, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
	view->decoration = toplevel_decoration;
}

static void handle_new_xdg_toplevel_decoration(struct wl_listener *listener, void *data) {
	struct wb_decoration *decoration = (struct wb_decoration *) calloc(1, sizeof(struct wb_decoration));
	struct wb_server *server = wl_container_of(listener, server, new_xdg_decoration);
	decoration->server = server;
	struct wlr_xdg_toplevel_decoration_v1 *toplevel_decoration = data;
	decoration->toplevel_decoration_destroy.notify = destroy_xdg_toplevel_decoration;
	wl_signal_add(&toplevel_decoration->manager->events.destroy, &decoration->toplevel_decoration_destroy);
	decoration->request_mode.notify = handle_xdg_decoration_mode;
	wl_signal_add(&toplevel_decoration->events.request_mode, &decoration->request_mode);
	decoration->mode_destroy.notify = free_xdg_decoration_mode;
	wl_signal_add(&toplevel_decoration->events.destroy, &decoration->mode_destroy);
	/* For some reason, a lot of clients don't emit the request_mode signal. */
	handle_xdg_decoration_mode(&decoration->request_mode, toplevel_decoration);
}

void init_xdg_decoration(struct wb_server *server) {
	struct wlr_xdg_decoration_manager_v1 *decoration = wlr_xdg_decoration_manager_v1_create(server->wl_display);
	server->new_xdg_decoration.notify = handle_new_xdg_toplevel_decoration;
	wl_signal_add(&decoration->events.new_toplevel_decoration, &server->new_xdg_decoration);
}
