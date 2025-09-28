/*
 * Copyright © 2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef EVDEV_TABLET_PAD_H
#define EVDEV_TABLET_PAD_H

#include "evdev.h"

#define LIBINPUT_BUTTONSET_AXIS_NONE 0

enum pad_status {
	PAD_NONE		= 0,
	PAD_AXES_UPDATED	= bit(0),
	PAD_BUTTONS_PRESSED	= bit(1),
	PAD_BUTTONS_RELEASED	= bit(2),
};

enum pad_axes {
	PAD_AXIS_NONE		= 0,
	PAD_AXIS_RING1		= bit(0),
	PAD_AXIS_RING2		= bit(1),
	PAD_AXIS_STRIP1		= bit(2),
	PAD_AXIS_STRIP2		= bit(3),
};

struct button_state {
	unsigned char bits[NCHARS(KEY_CNT)];
};

typedef struct {
	uint32_t value;
} key_or_button_map_t;

#define map_init(x_) ((x_).value = (uint32_t)-1)
#define map_is_unmapped(x_) ((x_).value == (uint32_t)-1)
#define map_is_button(x_) (((x_).value & 0xFF000000) == 0)
#define map_is_key(x_) (((x_).value & 0xFF000000) != 0)
#define map_set_button_map(field_, value_) ((field_).value = value_)
#define map_set_key_map(field_, value_) ((field_).value = value_ | 0xFF000000)
#define map_value(x_) ((x_).value & 0x00FFFFFF)

struct pad_dispatch {
	struct evdev_dispatch base;
	struct evdev_device *device;
	unsigned char status;
	uint32_t changed_axes;

	struct button_state button_state;
	struct button_state prev_button_state;

	key_or_button_map_t button_map[KEY_CNT];
	unsigned int nbuttons;

	bool have_abs_misc_terminator;

	struct {
		struct libinput_device_config_send_events config;
		enum libinput_config_send_events_mode current_mode;
	} sendevents;

	struct {
		struct list mode_group_list;
	} modes;
};

static inline struct pad_dispatch*
pad_dispatch(struct evdev_dispatch *dispatch)
{
	evdev_verify_dispatch_type(dispatch, DISPATCH_TABLET_PAD);

	return container_of(dispatch, struct pad_dispatch, base);
}

static inline struct libinput *
pad_libinput_context(const struct pad_dispatch *pad)
{
	return evdev_libinput_context(pad->device);
}

int
pad_init_leds(struct pad_dispatch *pad, struct evdev_device *device);
void
pad_destroy_leds(struct pad_dispatch *pad);
void
pad_button_update_mode(struct libinput_tablet_pad_mode_group *g,
		       unsigned int button_index,
		       enum libinput_button_state state);
#endif
