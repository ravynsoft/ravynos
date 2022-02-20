/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_RENDER_DMABUF_H
#define WLR_RENDER_DMABUF_H

#include <stdbool.h>
#include <stdint.h>

#define WLR_DMABUF_MAX_PLANES 4

/**
 * A Linux DMA-BUF pixel buffer.
 *
 * If the buffer was allocated with explicit modifiers enabled, the `modifier`
 * field must not be INVALID.
 *
 * If the buffer was allocated with explicit modifiers disabled (either because
 * the driver doesn't support it, or because the user didn't specify a valid
 * modifier list), the `modifier` field can have two values: INVALID means that
 * an implicit vendor-defined modifier is in use, LINEAR means that the buffer
 * is linear. The `modifier` field must not have any other value.
 *
 * When importing a DMA-BUF, users must not ignore the modifier unless it's
 * INVALID or LINEAR. In particular, users must not import a DMA-BUF to a
 * legacy API which doesn't support specifying an explicit modifier unless the
 * modifier is set to INVALID or LINEAR.
 */
struct wlr_dmabuf_attributes {
	int32_t width, height;
	uint32_t format; // FourCC code, see DRM_FORMAT_* in <drm_fourcc.h>
	uint64_t modifier; // see DRM_FORMAT_MOD_* in <drm_fourcc.h>

	int n_planes;
	uint32_t offset[WLR_DMABUF_MAX_PLANES];
	uint32_t stride[WLR_DMABUF_MAX_PLANES];
	int fd[WLR_DMABUF_MAX_PLANES];
};

/**
 * Closes all file descriptors in the DMA-BUF attributes.
 */
void wlr_dmabuf_attributes_finish(struct wlr_dmabuf_attributes *attribs);
/**
 * Clones the DMA-BUF attributes.
 */
bool wlr_dmabuf_attributes_copy(struct wlr_dmabuf_attributes *dst,
	const struct wlr_dmabuf_attributes *src);

#endif
