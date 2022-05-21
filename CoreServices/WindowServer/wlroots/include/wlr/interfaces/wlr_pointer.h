/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_POINTER_H
#define WLR_INTERFACES_WLR_POINTER_H

#include <wlr/types/wlr_pointer.h>

struct wlr_pointer_impl {
	void (*destroy)(struct wlr_pointer *pointer);
};

void wlr_pointer_init(struct wlr_pointer *pointer,
		const struct wlr_pointer_impl *impl);
void wlr_pointer_destroy(struct wlr_pointer *pointer);

#endif
