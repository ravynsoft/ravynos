#include <wlr/types/wlr_keyboard.h>

void keyboard_key_update(struct wlr_keyboard *keyboard,
		struct wlr_event_keyboard_key *event);

bool keyboard_modifier_update(struct wlr_keyboard *keyboard);

void keyboard_led_update(struct wlr_keyboard *keyboard);
