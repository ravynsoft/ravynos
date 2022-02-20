#include <assert.h>
#include <stdlib.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_tablet_pad.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/interfaces/wlr_switch.h>
#include <wlr/util/log.h>
#include "backend/headless.h"
#include "util/signal.h"

static void input_device_destroy(struct wlr_input_device *wlr_dev) {
	struct wlr_headless_input_device *dev =
		wl_container_of(wlr_dev, dev, wlr_input_device);
	wl_list_remove(&dev->link);
	free(dev);
}

static const struct wlr_input_device_impl input_device_impl = {
	.destroy = input_device_destroy,
};

bool wlr_input_device_is_headless(struct wlr_input_device *wlr_dev) {
	return wlr_dev->impl == &input_device_impl;
}

struct wlr_input_device *wlr_headless_add_input_device(
		struct wlr_backend *wlr_backend, enum wlr_input_device_type type) {
	struct wlr_headless_backend *backend =
		headless_backend_from_backend(wlr_backend);

	struct wlr_headless_input_device *device =
		calloc(1, sizeof(struct wlr_headless_input_device));
	if (device == NULL) {
		return NULL;
	}
	device->backend = backend;

	int vendor = 0;
	int product = 0;
	const char *name = "headless";
	struct wlr_input_device *wlr_device = &device->wlr_input_device;
	wlr_input_device_init(wlr_device, type, &input_device_impl, name, vendor,
		product);

	switch (type) {
	case WLR_INPUT_DEVICE_KEYBOARD:
		wlr_device->keyboard = calloc(1, sizeof(struct wlr_keyboard));
		if (wlr_device->keyboard == NULL) {
			wlr_log(WLR_ERROR, "Unable to allocate wlr_keyboard");
			goto error;
		}
		wlr_keyboard_init(wlr_device->keyboard, NULL);
		break;
	case WLR_INPUT_DEVICE_POINTER:
		wlr_device->pointer = calloc(1, sizeof(struct wlr_pointer));
		if (wlr_device->pointer == NULL) {
			wlr_log(WLR_ERROR, "Unable to allocate wlr_pointer");
			goto error;
		}
		wlr_pointer_init(wlr_device->pointer, NULL);
		break;
	case WLR_INPUT_DEVICE_TOUCH:
		wlr_device->touch = calloc(1, sizeof(struct wlr_touch));
		if (wlr_device->touch == NULL) {
			wlr_log(WLR_ERROR, "Unable to allocate wlr_touch");
			goto error;
		}
		wlr_touch_init(wlr_device->touch, NULL);
		break;
	case WLR_INPUT_DEVICE_TABLET_TOOL:
		wlr_device->tablet = calloc(1, sizeof(struct wlr_tablet));
		if (wlr_device->tablet == NULL) {
			wlr_log(WLR_ERROR, "Unable to allocate wlr_tablet");
			goto error;
		}
		wlr_tablet_init(wlr_device->tablet, NULL);
		break;
	case WLR_INPUT_DEVICE_TABLET_PAD:
		wlr_device->tablet_pad = calloc(1, sizeof(struct wlr_tablet_pad));
		if (wlr_device->tablet_pad == NULL) {
			wlr_log(WLR_ERROR, "Unable to allocate wlr_tablet_pad");
			goto error;
		}
		wlr_tablet_pad_init(wlr_device->tablet_pad, NULL);
		break;
	case WLR_INPUT_DEVICE_SWITCH:
		wlr_device->switch_device = calloc(1, sizeof(struct wlr_switch));
		if (wlr_device->switch_device == NULL) {
			wlr_log(WLR_ERROR, "Unable to allocate wlr_switch");
			goto error;
		}
		wlr_switch_init(wlr_device->switch_device, NULL);
	}

	wl_list_insert(&backend->input_devices, &device->link);

	if (backend->started) {
		wlr_signal_emit_safe(&backend->backend.events.new_input, wlr_device);
	}

	return wlr_device;
error:
	free(device);
	return NULL;
}
