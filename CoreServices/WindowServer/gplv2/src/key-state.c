// SPDX-License-Identifier: GPL-2.0-only
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_PRESSED_KEYS (16)

struct key_array {
	uint32_t keys[MAX_PRESSED_KEYS];
	int nr_keys;
};

static struct key_array pressed, bound;

static void
remove_key(struct key_array *array, uint32_t keycode)
{
	bool shifting = false;

	for (int i = 0; i < MAX_PRESSED_KEYS; ++i) {
		if (array->keys[i] == keycode) {
			--array->nr_keys;
			shifting = true;
		}
		if (shifting) {
			array->keys[i] = i < MAX_PRESSED_KEYS - 1
				? array->keys[i + 1] : 0;
		}
	}
}

static void
add_key(struct key_array *array, uint32_t keycode)
{
	array->keys[array->nr_keys++] = keycode;
}

void
key_state_set_pressed(uint32_t keycode, bool ispressed)
{
	if (ispressed) {
		add_key(&pressed, keycode);
	} else {
		remove_key(&pressed, keycode);
	}
}

void
key_state_store_pressed_keys_as_bound(void)
{
	memcpy(bound.keys, pressed.keys, MAX_PRESSED_KEYS * sizeof(uint32_t));
	bound.nr_keys = pressed.nr_keys;
}

bool
key_state_corresponding_press_event_was_bound(uint32_t keycode)
{
	for (int i = 0; i < bound.nr_keys; ++i) {
		if (bound.keys[i] == keycode) {
			return true;
		}
	}
	return false;
}

int
key_state_bound_key_remove(uint32_t keycode)
{
	remove_key(&bound, keycode);
	return bound.nr_keys;
}
