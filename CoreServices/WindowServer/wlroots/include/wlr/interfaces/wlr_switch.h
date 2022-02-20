/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_SWITCH_H
#define WLR_INTERFACES_WLR_SWITCH_H

#include <wlr/types/wlr_switch.h>

struct wlr_switch_impl {
	void (*destroy)(struct wlr_switch *switch_device);
};

void wlr_switch_init(struct wlr_switch *switch_device,
		struct wlr_switch_impl *impl);
void wlr_switch_destroy(struct wlr_switch *switch_device);

#endif
