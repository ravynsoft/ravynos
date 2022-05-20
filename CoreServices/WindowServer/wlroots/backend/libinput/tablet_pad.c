#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <string.h>
#include <libinput.h>
#include <stdlib.h>
#include <wlr/interfaces/wlr_tablet_pad.h>
#include <wlr/util/log.h>
#include "backend/libinput.h"
#include "util/signal.h"

const struct wlr_tablet_pad_impl libinput_tablet_pad_impl = {
	.name = "libinput-tablet-pad",
};

static void group_destroy(struct wlr_tablet_pad_group *group) {
	free(group->buttons);
	free(group->strips);
	free(group->rings);
	free(group);
}

static void add_pad_group_from_libinput(struct wlr_tablet_pad *pad,
		struct libinput_device *device, unsigned int index) {
	struct libinput_tablet_pad_mode_group *li_group =
		libinput_device_tablet_pad_get_mode_group(device, index);
	struct wlr_tablet_pad_group *group =
		calloc(1, sizeof(struct wlr_tablet_pad_group));
	if (!group) {
		wlr_log_errno(WLR_ERROR, "failed to allocate wlr_tablet_pad_group");
		return;
	}

	for (size_t i = 0; i < pad->ring_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_ring(li_group, i)) {
			++group->ring_count;
		}
	}
	group->rings = calloc(sizeof(unsigned int), group->ring_count);
	if (group->rings == NULL) {
		goto group_fail;
	}

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
	if (group->strips == NULL) {
		goto group_fail;
	}
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
	if (group->buttons == NULL) {
		goto group_fail;
	}
	size_t button = 0;
	for (size_t i = 0; i < pad->button_count; ++i) {
		if (libinput_tablet_pad_mode_group_has_button(li_group, i)) {
			group->buttons[button++] = i;
		}
	}

	group->mode_count = libinput_tablet_pad_mode_group_get_num_modes(li_group);

	libinput_tablet_pad_mode_group_ref(li_group);

	wl_list_insert(&pad->groups, &group->link);
	return;

group_fail:
	wlr_log(WLR_ERROR, "failed to configure wlr_tablet_pad_group");
	group_destroy(group);
}

void init_device_tablet_pad(struct wlr_libinput_input_device *dev) {
	struct libinput_device *handle = dev->handle;
	const char *name = libinput_device_get_name(handle);
	struct wlr_tablet_pad *wlr_tablet_pad = &dev->tablet_pad;
	wlr_tablet_pad_init(wlr_tablet_pad, &libinput_tablet_pad_impl, name);
	wlr_tablet_pad->base.vendor = libinput_device_get_id_vendor(handle);
	wlr_tablet_pad->base.product = libinput_device_get_id_product(handle);

	wlr_tablet_pad->button_count =
		libinput_device_tablet_pad_get_num_buttons(handle);
	wlr_tablet_pad->ring_count =
		libinput_device_tablet_pad_get_num_rings(handle);
	wlr_tablet_pad->strip_count =
		libinput_device_tablet_pad_get_num_strips(handle);

	struct udev_device *udev = libinput_device_get_udev_device(handle);
	char **dst = wl_array_add(&wlr_tablet_pad->paths, sizeof(char *));
	*dst = strdup(udev_device_get_syspath(udev));

	int groups = libinput_device_tablet_pad_get_num_mode_groups(handle);
	for (int i = 0; i < groups; ++i) {
		add_pad_group_from_libinput(wlr_tablet_pad, handle, i);
	}
}

void finish_device_tablet_pad(struct wlr_libinput_input_device *dev) {
	struct wlr_tablet_pad_group *group, *tmp;
	wl_list_for_each_safe(group, tmp, &dev->tablet_pad.groups, link) {
		group_destroy(group);
	}

	wlr_tablet_pad_finish(&dev->tablet_pad);

	int groups = libinput_device_tablet_pad_get_num_mode_groups(dev->handle);
	for (int i = 0; i < groups; ++i) {
		struct libinput_tablet_pad_mode_group *li_group =
			libinput_device_tablet_pad_get_mode_group(dev->handle, i);
		libinput_tablet_pad_mode_group_unref(li_group);
	}
}

struct wlr_libinput_input_device *device_from_tablet_pad(
		struct wlr_tablet_pad *wlr_tablet_pad) {
	assert(wlr_tablet_pad->impl == &libinput_tablet_pad_impl);

	struct wlr_libinput_input_device *dev =
		wl_container_of(wlr_tablet_pad, dev, tablet_pad);
	return dev;
}

void handle_tablet_pad_button(struct libinput_event *event,
		struct wlr_tablet_pad *tablet_pad) {
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
	wlr_signal_emit_safe(&tablet_pad->events.button, &wlr_event);
}

void handle_tablet_pad_ring(struct libinput_event *event,
		struct wlr_tablet_pad *tablet_pad) {
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
	wlr_signal_emit_safe(&tablet_pad->events.ring, &wlr_event);
}

void handle_tablet_pad_strip(struct libinput_event *event,
		struct wlr_tablet_pad *tablet_pad) {
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
	wlr_signal_emit_safe(&tablet_pad->events.strip, &wlr_event);
}
