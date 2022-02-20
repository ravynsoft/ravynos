/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_KEYBOARD_H
#define WLR_INTERFACES_WLR_KEYBOARD_H

#include <stdint.h>
#include <wlr/types/wlr_keyboard.h>

struct wlr_keyboard_impl {
	void (*destroy)(struct wlr_keyboard *keyboard);
	void (*led_update)(struct wlr_keyboard *keyboard, uint32_t leds);
};

void wlr_keyboard_init(struct wlr_keyboard *keyboard,
		const struct wlr_keyboard_impl *impl);
void wlr_keyboard_destroy(struct wlr_keyboard *keyboard);
void wlr_keyboard_notify_key(struct wlr_keyboard *keyboard,
		struct wlr_event_keyboard_key *event);
void wlr_keyboard_notify_modifiers(struct wlr_keyboard *keyboard,
		uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
		uint32_t group);

#endif
