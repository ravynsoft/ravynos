/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_UTIL_ADDON_H
#define WLR_UTIL_ADDON_H

#include <wayland-server-core.h>

struct wlr_addon_set {
	// private state
	struct wl_list addons;
};

struct wlr_addon;

struct wlr_addon_interface {
	const char *name;
	void (*destroy)(struct wlr_addon *addon);
};

struct wlr_addon {
	const struct wlr_addon_interface *impl;
	// private state
	const void *owner;
	struct wl_list link;
};

void wlr_addon_set_init(struct wlr_addon_set *set);
void wlr_addon_set_finish(struct wlr_addon_set *set);

void wlr_addon_init(struct wlr_addon *addon, struct wlr_addon_set *set,
		const void *owner, const struct wlr_addon_interface *impl);
void wlr_addon_finish(struct wlr_addon *addon);

struct wlr_addon *wlr_addon_find(struct wlr_addon_set *set, const void *owner,
		const struct wlr_addon_interface *impl);

#endif
