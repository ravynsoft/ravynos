/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_INPUT_INHIBITOR_H
#define WLR_TYPES_INPUT_INHIBITOR_H
#include <wayland-server-core.h>

struct wlr_input_inhibit_manager {
	struct wl_global *global;
	struct wl_client *active_client;
	struct wl_resource *active_inhibitor;

	struct wl_listener display_destroy;

	struct {
		struct wl_signal activate;   // struct wlr_input_inhibit_manager *
		struct wl_signal deactivate; // struct wlr_input_inhibit_manager *
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_input_inhibit_manager *wlr_input_inhibit_manager_create(
		struct wl_display *display);

#endif
