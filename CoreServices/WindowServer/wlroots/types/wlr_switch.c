#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_switch.h>
#include <wlr/types/wlr_switch.h>

void wlr_switch_init(struct wlr_switch *switch_device,
		struct wlr_switch_impl *impl) {
	switch_device->impl = impl;
	wl_signal_init(&switch_device->events.toggle);
}

void wlr_switch_destroy(struct wlr_switch *switch_device) {
	if (!switch_device) {
		return;
	}
	if (switch_device->impl && switch_device->impl->destroy) {
		switch_device->impl->destroy(switch_device);
	} else {
		free(switch_device);
	}
}
