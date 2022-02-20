#ifndef RENDER_WLR_RENDERER_H
#define RENDER_WLR_RENDERER_H

#include <wlr/render/wlr_renderer.h>

/**
 * Automatically select and create a renderer suitable for the DRM FD.
 */
struct wlr_renderer *renderer_autocreate_with_drm_fd(int drm_fd);
/**
 * Bind a buffer to the renderer.
 *
 * All subsequent rendering operations will operate on the supplied buffer.
 * After rendering operations are done, the caller must unbind a buffer by
 * calling renderer_bind_buffer with a NULL buffer.
 */
bool renderer_bind_buffer(struct wlr_renderer *r, struct wlr_buffer *buffer);
/**
 * Get the supported render formats. Buffers allocated with a format from this
 * list may be attached via wlr_renderer_begin_with_buffer.
 */
const struct wlr_drm_format_set *wlr_renderer_get_render_formats(
	struct wlr_renderer *renderer);
/**
 * Get the supported buffer capabilities.
 *
 * This functions returns a bitfield of supported wlr_buffer_cap.
 */
uint32_t renderer_get_render_buffer_caps(struct wlr_renderer *renderer);

#endif
