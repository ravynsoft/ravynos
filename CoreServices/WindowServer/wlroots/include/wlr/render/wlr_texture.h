/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_RENDER_WLR_TEXTURE_H
#define WLR_RENDER_WLR_TEXTURE_H

#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/render/dmabuf.h>

struct wlr_buffer;
struct wlr_renderer;
struct wlr_texture_impl;

struct wlr_texture {
	const struct wlr_texture_impl *impl;
	uint32_t width, height;
};

/**
 * Create a new texture from raw pixel data. `stride` is in bytes. The returned
 * texture is mutable.
 *
 * Should not be called in a rendering block like renderer_begin()/end() or
 * between attaching a renderer to an output and committing it.
 */
struct wlr_texture *wlr_texture_from_pixels(struct wlr_renderer *renderer,
	uint32_t fmt, uint32_t stride, uint32_t width, uint32_t height,
	const void *data);

/**
 * Create a new texture from a DMA-BUF. The returned texture is immutable.
 *
 * Should not be called in a rendering block like renderer_begin()/end() or
 * between attaching a renderer to an output and committing it.
 */
struct wlr_texture *wlr_texture_from_dmabuf(struct wlr_renderer *renderer,
	struct wlr_dmabuf_attributes *attribs);

/**
 * Returns true if this texture is using a fully opaque format.
 */
bool wlr_texture_is_opaque(struct wlr_texture *texture);

/**
  * Update a texture with raw pixels. The texture must be mutable, and the input
  * data must have the same pixel format that the texture was created with.
  *
  * Should not be called in a rendering block like renderer_begin()/end() or
  * between attaching a renderer to an output and committing it.
  */
bool wlr_texture_write_pixels(struct wlr_texture *texture,
	uint32_t stride, uint32_t width, uint32_t height,
	uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y,
	const void *data);

/**
 * Destroys this wlr_texture.
 */
void wlr_texture_destroy(struct wlr_texture *texture);

/**
 * Create a new texture from a buffer.
 *
 * Should not be called in a rendering block like renderer_begin()/end() or
 * between attaching a renderer to an output and committing it.
 */
struct wlr_texture *wlr_texture_from_buffer(struct wlr_renderer *renderer,
	struct wlr_buffer *buffer);

#endif
