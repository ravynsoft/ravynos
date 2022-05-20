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
	const char *name;
};

void wlr_tablet_pad_init(struct wlr_tablet_pad *pad,
	const struct wlr_tablet_pad_impl *impl, const char *name);

/**
 * Cleans up the resources owned by a wlr_tablet_pad.
 * This function will not clean the memory allocated by wlr_tablet_pad_group,
 * it's the responsibility of the caller to clean it.
 */
void wlr_tablet_pad_finish(struct wlr_tablet_pad *pad);

#endif
