// SPDX-License-Identifier: GPL-2.0-only
#include <string.h>
#include <strings.h>

#include "config/libinput.h"
#include "config/rcxml.h"
static void
libinput_category_init(struct libinput_category *l)
{
	l->name = NULL;
	l->pointer_speed = -2;
	l->natural_scroll = -1;
	l->left_handed = -1;
	l->tap = LIBINPUT_CONFIG_TAP_ENABLED;
	l->tap_button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;
	l->accel_profile = -1;
	l->middle_emu = -1;
	l->dwt = -1;
}

enum device_type
get_device_type(const char *s)
{
	if (!s) {
		return DEFAULT_DEVICE;
	}
	if (!strcasecmp(s, "touch")) {
		return TOUCH_DEVICE;
	}
	if (!strcasecmp(s, "non-touch")) {
		return NON_TOUCH_DEVICE;
	}
	return DEFAULT_DEVICE;
}

struct libinput_category *
libinput_category_create(void)
{
	struct libinput_category *l = calloc(1, sizeof(struct libinput_category));
	if (!l) {
		return NULL;
	}
	libinput_category_init(l);
	wl_list_insert(&rc.libinput_categories, &l->link);
	return l;
}
