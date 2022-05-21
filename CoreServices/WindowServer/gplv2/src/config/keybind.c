// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/util/log.h>
#include "config/keybind.h"
#include "config/rcxml.h"

uint32_t
parse_modifier(const char *symname)
{
	if (!strcmp(symname, "S")) {
		return WLR_MODIFIER_SHIFT;
	} else if (!strcmp(symname, "C")) {
		return WLR_MODIFIER_CTRL;
	} else if (!strcmp(symname, "A")) {
		return WLR_MODIFIER_ALT;
	} else if (!strcmp(symname, "W")) {
		return WLR_MODIFIER_LOGO;
	} else {
		return 0;
	}
}

struct keybind *
keybind_create(const char *keybind)
{
	struct keybind *k = calloc(1, sizeof(struct keybind));
	xkb_keysym_t keysyms[MAX_KEYSYMS];
	gchar **symnames = g_strsplit(keybind, "-", -1);
	for (int i = 0; symnames[i]; i++) {
		char *symname = symnames[i];
		uint32_t modifier = parse_modifier(symname);
		if (modifier != 0) {
			k->modifiers |= modifier;
		} else {
			xkb_keysym_t sym = xkb_keysym_to_lower(
				xkb_keysym_from_name(symname, XKB_KEYSYM_CASE_INSENSITIVE));
			if (sym == XKB_KEY_NoSymbol) {
				wlr_log(WLR_ERROR, "unknown keybind (%s)", symname);
				free(k);
				k = NULL;
				break;
			}
			keysyms[k->keysyms_len] = sym;
			k->keysyms_len++;
			if (k->keysyms_len == MAX_KEYSYMS) {
				wlr_log(WLR_ERROR, "There are a lot of fingers involved. "
					"We stopped counting at %u.", MAX_KEYSYMS);
				wlr_log(WLR_ERROR, "Offending keybind was %s", keybind);
				break;
			}
		}
	}
	g_strfreev(symnames);
	if (!k) {
		return NULL;
	}
	wl_list_insert(&rc.keybinds, &k->link);
	k->keysyms = malloc(k->keysyms_len * sizeof(xkb_keysym_t));
	memcpy(k->keysyms, keysyms, k->keysyms_len * sizeof(xkb_keysym_t));
	wl_list_init(&k->actions);
	return k;
}
