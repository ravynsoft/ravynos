#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_switch.h>
#include <wlr/interfaces/wlr_tablet_pad.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include "util/signal.h"

void wlr_input_device_init(struct wlr_input_device *dev,
		enum wlr_input_device_type type,
		const struct wlr_input_device_impl *impl,
		const char *name, int vendor, int product) {
	dev->type = type;
	dev->impl = impl;
	dev->name = strdup(name);
	dev->vendor = vendor;
	dev->product = product;

	wl_signal_init(&dev->events.destroy);
}

void wlr_input_device_destroy(struct wlr_input_device *dev) {
	if (!dev) {
		return;
	}

	wlr_signal_emit_safe(&dev->events.destroy, dev);

	if (dev->_device) {
		switch (dev->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			wlr_keyboard_destroy(dev->keyboard);
			break;
		case WLR_INPUT_DEVICE_POINTER:
			wlr_pointer_destroy(dev->pointer);
			break;
		case WLR_INPUT_DEVICE_SWITCH:
			wlr_switch_destroy(dev->switch_device);
			break;
		case WLR_INPUT_DEVICE_TOUCH:
			wlr_touch_destroy(dev->touch);
			break;
		case WLR_INPUT_DEVICE_TABLET_TOOL:
			wlr_tablet_destroy(dev->tablet);
			break;
		case WLR_INPUT_DEVICE_TABLET_PAD:
			wlr_tablet_pad_destroy(dev->tablet_pad);
			break;
		default:
			wlr_log(WLR_DEBUG, "Warning: leaking memory %p %p %d",
					dev->_device, dev, dev->type);
			break;
		}
	}
	free(dev->name);
	free(dev->output_name);
	if (dev->impl && dev->impl->destroy) {
		dev->impl->destroy(dev);
	} else {
		free(dev);
	}
}
