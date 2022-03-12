/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_TOUCH_H
#define WLR_INTERFACES_WLR_TOUCH_H

#include <wlr/types/wlr_touch.h>

struct wlr_touch_impl {
	const char *name;
};

void wlr_touch_init(struct wlr_touch *touch,
	const struct wlr_touch_impl *impl, const char *name);
void wlr_touch_finish(struct wlr_touch *touch);

#endif
