/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_KEYBOARD_SHORTCUTS_INHIBIT_V1_H
#define WLR_TYPES_WLR_KEYBOARD_SHORTCUTS_INHIBIT_V1_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

/* This interface permits clients to inhibit keyboard shortcut processing by
 * the compositor.
 *
 * This allows clients to pass them on to e.g. remote desktops or virtual
 * machine guests.
 *
 * Inhibitors are created for surfaces and seats. They should only be in effect
 * while this surface has focus.
 */

struct wlr_keyboard_shortcuts_inhibit_manager_v1 {
	// wlr_keyboard_shortcuts_inhibitor_v1::link
	struct wl_list inhibitors;
	struct wl_global *global;

	struct wl_listener display_destroy;

	struct {
		struct wl_signal new_inhibitor;	// wlr_keyboard_shortcuts_inhibitor_v1
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_keyboard_shortcuts_inhibitor_v1 {
	struct wlr_surface *surface;
	struct wlr_seat *seat;
	bool active;
	struct wl_resource *resource;

	struct wl_listener surface_destroy;
	struct wl_listener seat_destroy;

	// wlr_keyboard_shortcuts_inhibit_manager_v1::inhibitors
	struct wl_list link;

	struct {
		struct wl_signal destroy;
	} events;

	void *data;
};

/*
 * A compositor creating a manager will handle the new_inhibitor event and call
 * wlr_keyboard_shortcuts_inhibitor_v1_activate() if it decides to honour the
 * inhibitor. This will send the active event to the client, confirming
 * activation of the inhibitor. From then on the compositor should respect the
 * inhibitor until it calls wlr_keyboard_shortcuts_inhibitor_v1_deactivate() to
 * suspend the inhibitor with an inactive event to the client or receives the
 * destroy signal from wlroots, telling it that the inhibitor has been
 * destroyed.
 *
 * Not sending the active event to the client is the only way under the
 * protocol to let the client know that the compositor will not be honouring an
 * inhibitor. It's the client's job to somehow deal with not receiving the
 * event, i.e. not assume that shortcuts are inhibited and maybe destroy the
 * pending and request a new inhibitor after a timeout.
 */

struct wlr_keyboard_shortcuts_inhibit_manager_v1 *
wlr_keyboard_shortcuts_inhibit_v1_create(struct wl_display *display);

void wlr_keyboard_shortcuts_inhibitor_v1_activate(
	struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor);

void wlr_keyboard_shortcuts_inhibitor_v1_deactivate(
	struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor);

#endif
