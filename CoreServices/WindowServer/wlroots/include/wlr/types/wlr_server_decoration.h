/*
 * This protocol is obsolete and will be removed in a future version. The
 * recommended replacement is xdg-decoration.
 */

/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_SERVER_DECORATION_H
#define WLR_TYPES_WLR_SERVER_DECORATION_H

#include <wayland-server-core.h>

/**
 * Possible values to use in request_mode and the event mode. Same as
 * org_kde_kwin_server_decoration_manager_mode.
 */
enum wlr_server_decoration_manager_mode {
	/**
	 * Undecorated: The surface is not decorated at all, neither server nor
	 * client-side. An example is a popup surface which should not be
	 * decorated.
	 */
	WLR_SERVER_DECORATION_MANAGER_MODE_NONE = 0,
	/**
	 * Client-side decoration: The decoration is part of the surface and the
	 * client.
	 */
	WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT = 1,
	/**
	 * Server-side decoration: The server embeds the surface into a decoration
	 * frame.
	 */
	WLR_SERVER_DECORATION_MANAGER_MODE_SERVER = 2,
};

/**
 * A decoration negotiation interface which implements the KDE protocol.
 */
struct wlr_server_decoration_manager {
	struct wl_global *global;
	struct wl_list resources; // wl_resource_get_link
	struct wl_list decorations; // wlr_server_decoration::link

	uint32_t default_mode; // enum wlr_server_decoration_manager_mode

	struct wl_listener display_destroy;

	struct {
		struct wl_signal new_decoration;
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_server_decoration {
	struct wl_resource *resource;
	struct wlr_surface *surface;
	struct wl_list link;

	uint32_t mode; // enum wlr_server_decoration_manager_mode

	struct {
		struct wl_signal destroy;
		struct wl_signal mode;
	} events;

	struct wl_listener surface_destroy_listener;

	void *data;
};

struct wlr_server_decoration_manager *wlr_server_decoration_manager_create(
	struct wl_display *display);
void wlr_server_decoration_manager_set_default_mode(
	struct wlr_server_decoration_manager *manager, uint32_t default_mode);

#endif
