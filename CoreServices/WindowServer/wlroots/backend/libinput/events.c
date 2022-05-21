#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <libinput.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <wlr/backend/session.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/util/log.h>
#include "backend/libinput.h"
#include "util/array.h"
#include "util/signal.h"

static struct wlr_libinput_input_device *get_libinput_device_from_device(
		struct wlr_input_device *wlr_dev) {
	assert(wlr_input_device_is_libinput(wlr_dev));
	return (struct wlr_libinput_input_device *)wlr_dev;
}

struct wlr_input_device *get_appropriate_device(
		enum wlr_input_device_type desired_type,
		struct libinput_device *libinput_dev) {
	struct wl_list *wlr_devices = libinput_device_get_user_data(libinput_dev);
	if (!wlr_devices) {
		return NULL;
	}
	struct wlr_libinput_input_device *dev;
	wl_list_for_each(dev, wlr_devices, link) {
		if (dev->wlr_input_device.type == desired_type) {
			return &dev->wlr_input_device;
		}
	}
	return NULL;
}

static void input_device_destroy(struct wlr_input_device *wlr_dev) {
	struct wlr_libinput_input_device *dev =
		get_libinput_device_from_device(wlr_dev);
	libinput_device_unref(dev->handle);
	wl_list_remove(&dev->link);
	free(dev);
}

static const struct wlr_input_device_impl input_device_impl = {
	.destroy = input_device_destroy,
};

static struct wlr_input_device *allocate_device(
		struct wlr_libinput_backend *backend,
		struct libinput_device *libinput_dev, struct wl_list *wlr_devices,
		enum wlr_input_device_type type) {
	int vendor = libinput_device_get_id_vendor(libinput_dev);
	int product = libinput_device_get_id_product(libinput_dev);
	const char *name = libinput_device_get_name(libinput_dev);
	struct wlr_libinput_input_device *dev =
		calloc(1, sizeof(struct wlr_libinput_input_device));
	if (dev == NULL) {
		return NULL;
	}
	struct wlr_input_device *wlr_dev = &dev->wlr_input_device;
	libinput_device_get_size(libinput_dev,
			&wlr_dev->width_mm, &wlr_dev->height_mm);
	const char *output_name = libinput_device_get_output_name(libinput_dev);
	if (output_name != NULL) {
		wlr_dev->output_name = strdup(output_name);
	}
	wl_list_insert(wlr_devices, &dev->link);
	dev->handle = libinput_dev;
	libinput_device_ref(libinput_dev);
	wlr_input_device_init(wlr_dev, type, &input_device_impl,
			name, vendor, product);
	return wlr_dev;
}

bool wlr_input_device_is_libinput(struct wlr_input_device *wlr_dev) {
	return wlr_dev->impl == &input_device_impl;
}

static void handle_device_added(struct wlr_libinput_backend *backend,
		struct libinput_device *libinput_dev) {
	/*
	 * Note: the wlr API exposes only devices with a single capability, because
	 * that meshes better with how Wayland does things and is a bit simpler.
	 * However, libinput devices often have multiple capabilities - in such
	 * cases we have to create several devices.
	 */
	int vendor = libinput_device_get_id_vendor(libinput_dev);
	int product = libinput_device_get_id_product(libinput_dev);
	const char *name = libinput_device_get_name(libinput_dev);
	struct wl_list *wlr_devices = calloc(1, sizeof(struct wl_list));
	if (!wlr_devices) {
		wlr_log(WLR_ERROR, "Allocation failed");
		return;
	}
	wl_list_init(wlr_devices);
	wlr_log(WLR_DEBUG, "Added %s [%d:%d]", name, vendor, product);

	if (libinput_device_has_capability(
			libinput_dev, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
		struct wlr_input_device *wlr_dev = allocate_device(backend,
				libinput_dev, wlr_devices, WLR_INPUT_DEVICE_KEYBOARD);
		if (!wlr_dev) {
			goto fail;
		}
		wlr_dev->keyboard = create_libinput_keyboard(libinput_dev);
		if (!wlr_dev->keyboard) {
			free(wlr_dev);
			goto fail;
		}
		wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_dev);
	}
	if (libinput_device_has_capability(
			libinput_dev, LIBINPUT_DEVICE_CAP_POINTER)) {
		struct wlr_input_device *wlr_dev = allocate_device(backend,
				libinput_dev, wlr_devices, WLR_INPUT_DEVICE_POINTER);
		if (!wlr_dev) {
			goto fail;
		}
		wlr_dev->pointer = create_libinput_pointer(libinput_dev);
		if (!wlr_dev->pointer) {
			free(wlr_dev);
			goto fail;
		}
		wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_dev);
	}
	if (libinput_device_has_capability(
			libinput_dev, LIBINPUT_DEVICE_CAP_TOUCH)) {
		struct wlr_input_device *wlr_dev = allocate_device(backend,
				libinput_dev, wlr_devices, WLR_INPUT_DEVICE_TOUCH);
		if (!wlr_dev) {
			goto fail;
		}
		wlr_dev->touch = create_libinput_touch(libinput_dev);
		if (!wlr_dev->touch) {
			free(wlr_dev);
			goto fail;
		}
		wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_dev);
	}
	if (libinput_device_has_capability(libinput_dev,
			LIBINPUT_DEVICE_CAP_TABLET_TOOL)) {
		struct wlr_input_device *wlr_dev = allocate_device(backend,
				libinput_dev, wlr_devices, WLR_INPUT_DEVICE_TABLET_TOOL);
		if (!wlr_dev) {
			goto fail;
		}
		wlr_dev->tablet = create_libinput_tablet(libinput_dev);
		if (!wlr_dev->tablet) {
			free(wlr_dev);
			goto fail;
		}
		wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_dev);
	}
	if (libinput_device_has_capability(
			libinput_dev, LIBINPUT_DEVICE_CAP_TABLET_PAD)) {
		struct wlr_input_device *wlr_dev = allocate_device(backend,
				libinput_dev, wlr_devices, WLR_INPUT_DEVICE_TABLET_PAD);
		if (!wlr_dev) {
			goto fail;
		}
		wlr_dev->tablet_pad = create_libinput_tablet_pad(libinput_dev);
		if (!wlr_dev->tablet_pad) {
			free(wlr_dev);
			goto fail;
		}
		wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_dev);
	}
	if (libinput_device_has_capability(
			libinput_dev, LIBINPUT_DEVICE_CAP_GESTURE)) {
		// TODO
	}
	if (libinput_device_has_capability(
			libinput_dev, LIBINPUT_DEVICE_CAP_SWITCH)) {
		struct wlr_input_device *wlr_dev = allocate_device(backend,
			libinput_dev, wlr_devices, WLR_INPUT_DEVICE_SWITCH);
		if (!wlr_dev) {
			goto fail;
		}
		wlr_dev->switch_device = create_libinput_switch(libinput_dev);
		if (!wlr_dev->switch_device) {
			free(wlr_dev);
			goto fail;
		}
		wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_dev);
	}

	if (!wl_list_empty(wlr_devices)) {
		struct wl_list **dst = wl_array_add(&backend->wlr_device_lists, sizeof(wlr_devices));
		if (!dst) {
			goto fail;
		}
		*dst = wlr_devices;

		libinput_device_set_user_data(libinput_dev, wlr_devices);
	} else {
		free(wlr_devices);
	}
	return;

fail:
	wlr_log(WLR_ERROR, "Could not allocate new device");
	struct wlr_libinput_input_device *dev, *tmp_dev;
	wl_list_for_each_safe(dev, tmp_dev, wlr_devices, link) {
		free(dev);
	}
	free(wlr_devices);
}

static void handle_device_removed(struct wlr_libinput_backend *backend,
		struct libinput_device *libinput_dev) {
	struct wl_list *wlr_devices = libinput_device_get_user_data(libinput_dev);
	int vendor = libinput_device_get_id_vendor(libinput_dev);
	int product = libinput_device_get_id_product(libinput_dev);
	const char *name = libinput_device_get_name(libinput_dev);
	wlr_log(WLR_DEBUG, "Removing %s [%d:%d]", name, vendor, product);
	if (!wlr_devices) {
		return;
	}
	struct wlr_libinput_input_device *dev, *tmp_dev;
	wl_list_for_each_safe(dev, tmp_dev, wlr_devices, link) {
		wlr_input_device_destroy(&dev->wlr_input_device);
	}
	size_t i = 0;
	struct wl_list **ptr;
	wl_array_for_each(ptr, &backend->wlr_device_lists) {
		struct wl_list *iter = *ptr;
		if (iter == wlr_devices) {
			array_remove_at(&backend->wlr_device_lists,
				i * sizeof(struct wl_list *), sizeof(struct wl_list *));
			break;
		}
		i++;
	}
	free(wlr_devices);
}

void handle_libinput_event(struct wlr_libinput_backend *backend,
		struct libinput_event *event) {
	struct libinput_device *libinput_dev = libinput_event_get_device(event);
	enum libinput_event_type event_type = libinput_event_get_type(event);
	switch (event_type) {
	case LIBINPUT_EVENT_DEVICE_ADDED:
		handle_device_added(backend, libinput_dev);
		break;
	case LIBINPUT_EVENT_DEVICE_REMOVED:
		handle_device_removed(backend, libinput_dev);
		break;
	case LIBINPUT_EVENT_KEYBOARD_KEY:
		handle_keyboard_key(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_POINTER_MOTION:
		handle_pointer_motion(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
		handle_pointer_motion_abs(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_POINTER_BUTTON:
		handle_pointer_button(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_POINTER_AXIS:
		handle_pointer_axis(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TOUCH_DOWN:
		handle_touch_down(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TOUCH_UP:
		handle_touch_up(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TOUCH_MOTION:
		handle_touch_motion(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TOUCH_CANCEL:
		handle_touch_cancel(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TOUCH_FRAME:
		handle_touch_frame(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
		handle_tablet_tool_axis(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
		handle_tablet_tool_proximity(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_TIP:
		handle_tablet_tool_tip(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
		handle_tablet_tool_button(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
		handle_tablet_pad_button(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TABLET_PAD_RING:
		handle_tablet_pad_ring(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_TABLET_PAD_STRIP:
		handle_tablet_pad_strip(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_SWITCH_TOGGLE:
		handle_switch_toggle(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
		handle_pointer_swipe_begin(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
		handle_pointer_swipe_update(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_END:
		handle_pointer_swipe_end(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
		handle_pointer_pinch_begin(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
		handle_pointer_pinch_update(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_END:
		handle_pointer_pinch_end(event, libinput_dev);
		break;
#if LIBINPUT_HAS_HOLD_GESTURES
	case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
		handle_pointer_hold_begin(event, libinput_dev);
		break;
	case LIBINPUT_EVENT_GESTURE_HOLD_END:
		handle_pointer_hold_end(event, libinput_dev);
		break;
#endif
	default:
		wlr_log(WLR_DEBUG, "Unknown libinput event %d", event_type);
		break;
	}
}
