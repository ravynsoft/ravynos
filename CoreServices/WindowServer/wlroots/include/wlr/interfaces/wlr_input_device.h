/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_INPUT_DEVICE_H
#define WLR_INTERFACES_WLR_INPUT_DEVICE_H

#include <wlr/types/wlr_input_device.h>

struct wlr_input_device_impl {
	void (*destroy)(struct wlr_input_device *wlr_device);
};

void wlr_input_device_init(
		struct wlr_input_device *wlr_device,
		enum wlr_input_device_type type,
		const struct wlr_input_device_impl *impl,
		const char *name, int vendor, int product);
void wlr_input_device_destroy(struct wlr_input_device *dev);

#endif
