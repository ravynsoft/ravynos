/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_LIBINPUT_H
#define __LABWC_LIBINPUT_H

#include <libinput.h>
#include <string.h>
#include <wayland-server-core.h>

enum device_type {
	DEFAULT_DEVICE,
	TOUCH_DEVICE,
	NON_TOUCH_DEVICE,
};

struct libinput_category {
	enum device_type type;
	char *name;
	struct wl_list link;
	float pointer_speed;
	int natural_scroll;
	int left_handed;
	enum libinput_config_tap_state tap;
	enum libinput_config_tap_button_map tap_button_map;
	enum libinput_config_accel_profile accel_profile;
	enum libinput_config_middle_emulation_state middle_emu;
	enum libinput_config_dwt_state dwt;
};

enum device_type get_device_type(const char *s);
struct libinput_category *libinput_category_create(void);

#endif /* __LABWC_LIBINPUT_H */
