#ifndef RENDER_ALLOCATOR_GBM_H
#define RENDER_ALLOCATOR_GBM_H

#include <gbm.h>
#include <wlr/render/dmabuf.h>
#include <wlr/types/wlr_buffer.h>
#include "render/allocator/allocator.h"

struct wlr_gbm_buffer {
	struct wlr_buffer base;

	struct wl_list link; // wlr_gbm_allocator.buffers

	struct gbm_bo *gbm_bo; // NULL if the gbm_device has been destroyed
	struct wlr_dmabuf_attributes dmabuf;
};

struct wlr_gbm_allocator {
	struct wlr_allocator base;

	int fd;
	struct gbm_device *gbm_device;

	struct wl_list buffers; // wlr_gbm_buffer.link
};

/**
 * Creates a new GBM allocator from a DRM FD.
 *
 * Takes ownership over the FD.
 */
struct wlr_allocator *wlr_gbm_allocator_create(int drm_fd);

#endif
