/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_KEYBOARD_H
#define WLR_TYPES_WLR_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_input_device.h>
#include <xkbcommon/xkbcommon.h>

#define WLR_LED_COUNT 3

enum wlr_keyboard_led {
	WLR_LED_NUM_LOCK = 1 << 0,
	WLR_LED_CAPS_LOCK = 1 << 1,
	WLR_LED_SCROLL_LOCK = 1 << 2,
};

#define WLR_MODIFIER_COUNT 8

enum wlr_keyboard_modifier {
	WLR_MODIFIER_SHIFT = 1 << 0,
	WLR_MODIFIER_CAPS = 1 << 1,
	WLR_MODIFIER_CTRL = 1 << 2,
	WLR_MODIFIER_ALT = 1 << 3,
	WLR_MODIFIER_MOD2 = 1 << 4,
	WLR_MODIFIER_MOD3 = 1 << 5,
	WLR_MODIFIER_LOGO = 1 << 6,
	WLR_MODIFIER_MOD5 = 1 << 7,
};

#define WLR_KEYBOARD_KEYS_CAP 32

struct wlr_keyboard_impl;

struct wlr_keyboard_modifiers {
	xkb_mod_mask_t depressed;
	xkb_mod_mask_t latched;
	xkb_mod_mask_t locked;
	xkb_mod_mask_t group;
};

struct wlr_keyboard {
	struct wlr_input_device base;

	const struct wlr_keyboard_impl *impl;
	struct wlr_keyboard_group *group;

	char *keymap_string;
	size_t keymap_size;
	int keymap_fd;
	struct xkb_keymap *keymap;
	struct xkb_state *xkb_state;
	xkb_led_index_t led_indexes[WLR_LED_COUNT];
	xkb_mod_index_t mod_indexes[WLR_MODIFIER_COUNT];

	uint32_t keycodes[WLR_KEYBOARD_KEYS_CAP];
	size_t num_keycodes;
	struct wlr_keyboard_modifiers modifiers;

	struct {
		int32_t rate;
		int32_t delay;
	} repeat_info;

	struct {
		/**
		 * The `key` event signals with a `wlr_event_keyboard_key` event that a
		 * key has been pressed or released on the keyboard. This event is
		 * emitted before the xkb state of the keyboard has been updated
		 * (including modifiers).
		 */
		struct wl_signal key;

		/**
		 * The `modifiers` event signals that the modifier state of the
		 * `wlr_keyboard` has been updated. At this time, you can read the
		 * modifier state of the `wlr_keyboard` and handle the updated state by
		 * sending it to clients.
		 */
		struct wl_signal modifiers;
		struct wl_signal keymap;
		struct wl_signal repeat_info;
	} events;

	void *data;
};

struct wlr_event_keyboard_key {
	uint32_t time_msec;
	uint32_t keycode;
	bool update_state; // if backend doesn't update modifiers on its own
	enum wl_keyboard_key_state state;
};

bool wlr_keyboard_set_keymap(struct wlr_keyboard *kb,
	struct xkb_keymap *keymap);

bool wlr_keyboard_keymaps_match(struct xkb_keymap *km1, struct xkb_keymap *km2);

/**
 * Sets the keyboard repeat info. `rate` is in key repeats/second and delay is
 * in milliseconds.
 */
void wlr_keyboard_set_repeat_info(struct wlr_keyboard *kb, int32_t rate,
	int32_t delay);
void wlr_keyboard_led_update(struct wlr_keyboard *keyboard, uint32_t leds);
uint32_t wlr_keyboard_get_modifiers(struct wlr_keyboard *keyboard);

#endif
