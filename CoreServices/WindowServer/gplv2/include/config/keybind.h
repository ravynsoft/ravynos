/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_KEYBIND_H
#define __LABWC_KEYBIND_H

#include <wlr/types/wlr_keyboard.h>
#include <xkbcommon/xkbcommon.h>

#define MAX_KEYSYMS 32

struct keybind {
	uint32_t modifiers;
	xkb_keysym_t *keysyms;
	size_t keysyms_len;
	struct wl_list actions;
	struct wl_list link;
};

/**
 * keybind_create - parse keybind and add to linked list
 * @keybind: key combination
 */
struct keybind *keybind_create(const char *keybind);

/**
 * parse_modifier - parse a string containing a single modifier name (e.g. "S")
 * into the represented modifier value. returns 0 for invalid modifier names.
 * @symname: modifier name
 */
uint32_t parse_modifier(const char *symname);

#endif /* __LABWC_KEYBIND_H */
