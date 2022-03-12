#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include "interfaces/wlr_input_device.h"
#include "util/signal.h"

void wlr_input_device_init(struct wlr_input_device *dev,
		enum wlr_input_device_type type, const char *name) {
	dev->type = type;
	dev->name = strdup(name);
	dev->vendor = 0;
	dev->product = 0;

	wl_signal_init(&dev->events.destroy);
}

void wlr_input_device_finish(struct wlr_input_device *wlr_device) {
	if (!wlr_device) {
		return;
	}

	wlr_signal_emit_safe(&wlr_device->events.destroy, wlr_device);

	wl_list_remove(&wlr_device->events.destroy.listener_list);

	free(wlr_device->name);
	free(wlr_device->output_name);
}
