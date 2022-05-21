#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/types/wlr_touch.h>

void wlr_touch_init(struct wlr_touch *touch,
		const struct wlr_touch_impl *impl) {
	touch->impl = impl;
	wl_signal_init(&touch->events.down);
	wl_signal_init(&touch->events.up);
	wl_signal_init(&touch->events.motion);
	wl_signal_init(&touch->events.cancel);
	wl_signal_init(&touch->events.frame);
}

void wlr_touch_destroy(struct wlr_touch *touch) {
	if (touch && touch->impl && touch->impl->destroy) {
		touch->impl->destroy(touch);
	} else {
		free(touch);
	}
}
