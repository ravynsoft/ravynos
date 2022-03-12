#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/types/wlr_touch.h>

#include "interfaces/wlr_input_device.h"

void wlr_touch_init(struct wlr_touch *touch,
		const struct wlr_touch_impl *impl, const char *name) {
	wlr_input_device_init(&touch->base, WLR_INPUT_DEVICE_TOUCH, name);
	touch->base.touch = touch;

	touch->impl = impl;
	wl_signal_init(&touch->events.down);
	wl_signal_init(&touch->events.up);
	wl_signal_init(&touch->events.motion);
	wl_signal_init(&touch->events.cancel);
	wl_signal_init(&touch->events.frame);
}

void wlr_touch_finish(struct wlr_touch *touch) {
	wlr_input_device_finish(&touch->base);
}
