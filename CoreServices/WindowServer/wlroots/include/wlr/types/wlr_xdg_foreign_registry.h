/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_XDG_FOREIGN_REGISTRY_H
#define WLR_TYPES_WLR_XDG_FOREIGN_REGISTRY_H

#include <wayland-server-core.h>

#define WLR_XDG_FOREIGN_HANDLE_SIZE 37

/**
 * wlr_xdg_foreign_registry is used for storing a list of exported surfaces with
 * the xdg-foreign family of protocols.
 *
 * It can be used to allow interoperability between clients using different
 * versions of the protocol (if all versions use the same registry).
 */
struct wlr_xdg_foreign_registry {
	struct wl_list exported_surfaces; // struct wlr_xdg_foreign_exported_surface

	struct wl_listener display_destroy;
	struct {
		struct wl_signal destroy;
	} events;
};

struct wlr_xdg_foreign_exported {
	struct wl_list link; // wlr_xdg_foreign_registry::exported_surfaces
	struct wlr_xdg_foreign_registry *registry;

	struct wlr_surface *surface;

	char handle[WLR_XDG_FOREIGN_HANDLE_SIZE];

	struct {
		struct wl_signal destroy;
	} events;
};

/**
 * Create an empty wlr_xdg_foreign_registry.
 *
 * It will be destroyed when the associated display is destroyed.
 */
struct wlr_xdg_foreign_registry *wlr_xdg_foreign_registry_create(
	struct wl_display *display);

/**
 * Add the given exported surface to the registry and assign it a unique handle.
 * The caller is responsible for removing the exported surface from the repository
 * if it is destroyed.
 *
 * Returns true if the initialization was successful.
 */
bool wlr_xdg_foreign_exported_init(struct wlr_xdg_foreign_exported *surface,
	struct wlr_xdg_foreign_registry *registry);

/**
 * Find an exported surface with the given handle, or NULL if such a surface
 * does not exist.
 */
struct wlr_xdg_foreign_exported *wlr_xdg_foreign_registry_find_by_handle(
	struct wlr_xdg_foreign_registry *registry, const char *handle);

/**
 * Remove the given surface from the registry it was previously added in.
 */
void wlr_xdg_foreign_exported_finish(struct wlr_xdg_foreign_exported *surface);

#endif
