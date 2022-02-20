/*
 * This is a stable interface of wlroots. Future changes will be limited to:
 *
 * - New functions
 * - New struct members
 * - New enum members
 *
 * Note that wlroots does not make an ABI compatibility promise - in the future,
 * the layout and size of structs used by wlroots may change, requiring code
 * depending on this header to be recompiled (but not edited).
 *
 * Breaking changes are announced in the release notes and follow a 1-year
 * deprecation schedule.
 */

#ifndef WLR_TYPES_WLR_REGION_H
#define WLR_TYPES_WLR_REGION_H

#include <pixman.h>

struct wl_resource;

/**
 * Obtain a Pixman region from a wl_region resource.
 *
 * To allow clients to create wl_region objects, call wlr_compositor_create().
 */
pixman_region32_t *wlr_region_from_resource(struct wl_resource *resource);

#endif
