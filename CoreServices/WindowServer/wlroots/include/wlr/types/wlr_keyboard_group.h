/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_KEYBOARD_GROUP_H
#define WLR_TYPES_WLR_KEYBOARD_GROUP_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_input_device.h>

struct wlr_keyboard_group {
	struct wlr_keyboard keyboard;
	struct wlr_input_device *input_device;
	struct wl_list devices; // keyboard_group_device::link
	struct wl_list keys; // keyboard_group_key::link

	struct {
		/*
		 * Sent when a keyboard has entered the group with keys currently
		 * pressed that are not pressed by any other keyboard in the group. The
		 * data for this signal will be a wl_array containing the key codes.
		 * This should be used to update the compositor's internal state.
		 * Bindings should not be triggered based off of these key codes and
		 * they should also not notify any surfaces of the key press.
		 */
		struct wl_signal enter;

		/*
		 * Sent when a keyboard has left the group with keys currently pressed
		 * that are not pressed by any other keyboard in the group. The data for
		 * this signal will be a wl_array containing the key codes. This should
		 * be used to update the compositor's internal state. Bindings should
		 * not be triggered based off of these key codes. Additionally, surfaces
		 * should only be notified if they received a corresponding key press
		 * for the key code.
		 */
		struct wl_signal leave;
	} events;

	void *data;
};

struct wlr_keyboard_group *wlr_keyboard_group_create(void);

struct wlr_keyboard_group *wlr_keyboard_group_from_wlr_keyboard(
		struct wlr_keyboard *keyboard);

bool wlr_keyboard_group_add_keyboard(struct wlr_keyboard_group *group,
		struct wlr_keyboard *keyboard);

void wlr_keyboard_group_remove_keyboard(struct wlr_keyboard_group *group,
		struct wlr_keyboard *keyboard);

void wlr_keyboard_group_destroy(struct wlr_keyboard_group *group);

#endif
