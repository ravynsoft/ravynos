/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_TABLET_PAD_H
#define WLR_INTERFACES_WLR_TABLET_PAD_H

#include <wlr/types/wlr_tablet_pad.h>

struct wlr_tablet_pad_impl {
	void (*destroy)(struct wlr_tablet_pad *pad);
};

void wlr_tablet_pad_init(struct wlr_tablet_pad *pad,
		struct wlr_tablet_pad_impl *impl);
void wlr_tablet_pad_destroy(struct wlr_tablet_pad *pad);

#endif
