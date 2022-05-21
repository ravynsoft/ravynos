// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <linux/input-event-codes.h>
#include <strings.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include "config/mousebind.h"
#include "config/rcxml.h"

uint32_t
mousebind_button_from_str(const char *str, uint32_t *modifiers)
{
	assert(str);

	if (modifiers) {
		*modifiers = 0;
		while (strlen(str) >= 2 && str[1] == '-') {
			char modname[2] = {str[0], 0};
			uint32_t parsed_modifier = parse_modifier(modname);
			if (!parsed_modifier) {
				goto invalid;
			}
			*modifiers |= parsed_modifier;
			str += 2;
		}
	}

	if (!strcasecmp(str, "Left")) {
		return BTN_LEFT;
	} else if (!strcasecmp(str, "Right")) {
		return BTN_RIGHT;
	} else if (!strcasecmp(str, "Middle")) {
		return BTN_MIDDLE;
	}
invalid:
	wlr_log(WLR_ERROR, "unknown button (%s)", str);
	return UINT32_MAX;
}

enum mouse_event
mousebind_event_from_str(const char *str)
{
	assert(str);
	if (!strcasecmp(str, "doubleclick")) {
		return MOUSE_ACTION_DOUBLECLICK;
	} else if (!strcasecmp(str, "click")) {
		return MOUSE_ACTION_CLICK;
	} else if (!strcasecmp(str, "press")) {
		return MOUSE_ACTION_PRESS;
	} else if (!strcasecmp(str, "release")) {
		return MOUSE_ACTION_RELEASE;
	} else if (!strcasecmp(str, "drag")) {
		return MOUSE_ACTION_DRAG;
	}
	wlr_log(WLR_ERROR, "unknown mouse action (%s)", str);
	return MOUSE_ACTION_NONE;
}

static enum ssd_part_type
context_from_str(const char *str)
{
	if (!strcasecmp(str, "Close")) {
		return LAB_SSD_BUTTON_CLOSE;
	} else if (!strcasecmp(str, "Maximize")) {
		return LAB_SSD_BUTTON_MAXIMIZE;
	} else if (!strcasecmp(str, "Iconify")) {
		return LAB_SSD_BUTTON_ICONIFY;
	} else if (!strcasecmp(str, "WindowMenu")) {
		return LAB_SSD_BUTTON_WINDOW_MENU;
	} else if (!strcasecmp(str, "Titlebar")) {
		return LAB_SSD_PART_TITLEBAR;
	} else if (!strcasecmp(str, "Title")) {
		return LAB_SSD_PART_TITLE;
	} else if (!strcasecmp(str, "TLCorner")) {
		return LAB_SSD_PART_CORNER_TOP_LEFT;
	} else if (!strcasecmp(str, "TRCorner")) {
		return LAB_SSD_PART_CORNER_TOP_RIGHT;
	} else if (!strcasecmp(str, "BRCorner")) {
		return LAB_SSD_PART_CORNER_BOTTOM_RIGHT;
	} else if (!strcasecmp(str, "BLCorner")) {
		return LAB_SSD_PART_CORNER_BOTTOM_LEFT;
	} else if (!strcasecmp(str, "Top")) {
		return LAB_SSD_PART_TOP;
	} else if (!strcasecmp(str, "Right")) {
		return LAB_SSD_PART_RIGHT;
	} else if (!strcasecmp(str, "Bottom")) {
		return LAB_SSD_PART_BOTTOM;
	} else if (!strcasecmp(str, "Left")) {
		return LAB_SSD_PART_LEFT;
	} else if (!strcasecmp(str, "Frame")) {
		return LAB_SSD_FRAME;
	} else if (!strcasecmp(str, "Client")) {
		return LAB_SSD_CLIENT;
	} else if (!strcasecmp(str, "Desktop")) {
		return LAB_SSD_ROOT;
	} else if (!strcasecmp(str, "Root")) {
		return LAB_SSD_ROOT;
	}
	wlr_log(WLR_ERROR, "unknown mouse context (%s)", str);
	return LAB_SSD_NONE;
}

struct mousebind *
mousebind_create(const char *context)
{
	if (!context) {
		wlr_log(WLR_ERROR, "mousebind context not specified");
		return NULL;
	}
	struct mousebind *m = calloc(1, sizeof(struct mousebind));
	m->context = context_from_str(context);
	if (m->context != LAB_SSD_NONE) {
		wl_list_insert(&rc.mousebinds, &m->link);
	}
	wl_list_init(&m->actions);
	return m;
}
