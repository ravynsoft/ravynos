#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <assert.h>
#include <string.h>
#include <libinput.h>
#include <stdlib.h>
#include <wlr/backend/session.h>
#include <wlr/interfaces/wlr_tablet_pad.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include "backend/libinput.h"
#include "util/signal.h"

// FIXME: Decide on how to alloc/count here
static void add_pad_group_from_libinput(struct wlr_tablet_pad *pad,
		struct libinput_device *device, unsigned int index) {
	struct libinput_tablet_pad_mode_group *li_group =
		libinput_device_tablet_pad_get_mode_group(device, index);
	struct wlr_tablet_pad_group *group =
		calloc(1, sizeof(struct wlr_tablet_pad_group));
	if (!group) {
		return;
	}

	for (size_t i = 0; i < pad->ring_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_ring(li_group, i)) {
			++group->ring_count;
		}
	}
	group->rings = calloc(sizeof(unsigned int), group->ring_count);
	size_t ring = 0;
	for (size_t i = 0; i < pad->ring_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_ring(li_group, i)) {
			group->rings[ring++] = i;
		}
	}

	for (size_t i = 0; i < pad->strip_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_strip(li_group, i)) {
			++group->strip_count;
		}
	}
	group->strips = calloc(sizeof(unsigned int), group->strip_count);
	size_t strip = 0;
	for (size_t i = 0; i < pad->strip_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_strip(li_group, i)) {
			group->strips[strip++] = i;
		}
	}

	for (size_t i = 0; i < pad->button_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_button(li_group, i)) {
			++group->button_count;
		}
	}
	group->buttons = calloc(sizeof(unsigned int), group->button_count);
	size_t button = 0;
	for (size_t i = 0; i < pad->button_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_button(li_group, i)) {
			group->buttons[button++] = i;
		}
	}

	group->mode_count = libinput_tablet_pad_mode_group_get_num_modes(li_group);
	wl_list_insert(&pad->groups, &group->link);
}

struct wlr_tablet_pad *create_libinput_tablet_pad(
		struct libinput_device *libinput_dev) {
	assert(libinput_dev);
	struct wlr_tablet_pad *wlr_tablet_pad =
		calloc(1, sizeof(struct wlr_tablet_pad));
	if (!wlr_tablet_pad) {
		wlr_log(WLR_ERROR, "Unable to allocate wlr_tablet_pad");
		return NULL;
	}
	wlr_tablet_pad_init(wlr_tablet_pad, NULL);

	wlr_tablet_pad->button_count =
		libinput_device_tablet_pad_get_num_buttons(libinput_dev);
	wlr_tablet_pad->ring_count =
		libinput_device_tablet_pad_get_num_rings(libinput_dev);
	wlr_tablet_pad->strip_count =
		libinput_device_tablet_pad_get_num_strips(libinput_dev);

	struct udev_device *udev = libinput_device_get_udev_device(libinput_dev);
	char **dst = wl_array_add(&wlr_tablet_pad->paths, sizeof(char *));
	*dst = strdup(udev_device_get_syspath(udev));

	int groups = libinput_device_tablet_pad_get_num_mode_groups(libinput_dev);
	for (int i = 0; i < groups; ++i) {
		add_pad_group_from_libinput(wlr_tablet_pad, libinput_dev, i);
	}

	return wlr_tablet_pad;
}

void handle_tablet_pad_button(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TABLET_PAD, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG,
			"Got a tablet pad event for a device with no tablet pad?");
		return;
	}
	struct libinput_event_tablet_pad *pevent =
		libinput_event_get_tablet_pad_event(event);
	struct wlr_event_tablet_pad_button wlr_event = { 0 };
	wlr_event.time_msec =
		usec_to_msec(libinput_event_tablet_pad_get_time_usec(pevent));
	wlr_event.button = libinput_event_tablet_pad_get_button_number(pevent);
	wlr_event.mode = libinput_event_tablet_pad_get_mode(pevent);
	wlr_event.group = libinput_tablet_pad_mode_group_get_index(
		libinput_event_tablet_pad_get_mode_group(pevent));
	switch (libinput_event_tablet_pad_get_button_state(pevent)) {
	case LIBINPUT_BUTTON_STATE_PRESSED:
		wlr_event.state = WLR_BUTTON_PRESSED;
		break;
	case LIBINPUT_BUTTON_STATE_RELEASED:
		wlr_event.state = WLR_BUTTON_RELEASED;
		break;
	}
	wlr_signal_emit_safe(&wlr_dev->tablet_pad->events.button, &wlr_event);
}

void handle_tablet_pad_ring(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TABLET_PAD, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG,
			"Got a tablet pad event for a device with no tablet pad?");
		return;
	}
	struct libinput_event_tablet_pad *pevent =
		libinput_event_get_tablet_pad_event(event);
	struct wlr_event_tablet_pad_ring wlr_event = { 0 };
	wlr_event.time_msec =
		usec_to_msec(libinput_event_tablet_pad_get_time_usec(pevent));
	wlr_event.ring = libinput_event_tablet_pad_get_ring_number(pevent);
	wlr_event.position = libinput_event_tablet_pad_get_ring_position(pevent);
	wlr_event.mode = libinput_event_tablet_pad_get_mode(pevent);
	switch (libinput_event_tablet_pad_get_ring_source(pevent)) {
	case LIBINPUT_TABLET_PAD_RING_SOURCE_UNKNOWN:
		wlr_event.source = WLR_TABLET_PAD_RING_SOURCE_UNKNOWN;
		break;
	case LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER:
		wlr_event.source = WLR_TABLET_PAD_RING_SOURCE_FINGER;
		break;
	}
	wlr_signal_emit_safe(&wlr_dev->tablet_pad->events.ring, &wlr_event);
}

void handle_tablet_pad_strip(struct libinput_event *event,
		struct libinput_device *libinput_dev) {
	struct wlr_input_device *wlr_dev =
		get_appropriate_device(WLR_INPUT_DEVICE_TABLET_PAD, libinput_dev);
	if (!wlr_dev) {
		wlr_log(WLR_DEBUG,
			"Got a tablet pad event for a device with no tablet pad?");
		return;
	}
	struct libinput_event_tablet_pad *pevent =
		libinput_event_get_tablet_pad_event(event);
	struct wlr_event_tablet_pad_strip wlr_event = { 0 };
	wlr_event.time_msec =
		usec_to_msec(libinput_event_tablet_pad_get_time_usec(pevent));
	wlr_event.strip = libinput_event_tablet_pad_get_strip_number(pevent);
	wlr_event.position = libinput_event_tablet_pad_get_strip_position(pevent);
	wlr_event.mode = libinput_event_tablet_pad_get_mode(pevent);
	switch (libinput_event_tablet_pad_get_strip_source(pevent)) {
	case LIBINPUT_TABLET_PAD_STRIP_SOURCE_UNKNOWN:
		wlr_event.source = WLR_TABLET_PAD_STRIP_SOURCE_UNKNOWN;
		break;
	case LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER:
		wlr_event.source = WLR_TABLET_PAD_STRIP_SOURCE_FINGER;
		break;
	}
	wlr_signal_emit_safe(&wlr_dev->tablet_pad->events.strip, &wlr_event);
}
