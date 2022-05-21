#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wlr/render/interface.h>
#include <wlr/render/wlr_texture.h>
#include "types/wlr_buffer.h"

void wlr_texture_init(struct wlr_texture *texture,
		const struct wlr_texture_impl *impl, uint32_t width, uint32_t height) {
	texture->impl = impl;
	texture->width = width;
	texture->height = height;
}

void wlr_texture_destroy(struct wlr_texture *texture) {
	if (texture && texture->impl && texture->impl->destroy) {
		texture->impl->destroy(texture);
	} else {
		free(texture);
	}
}

struct wlr_texture *wlr_texture_from_pixels(struct wlr_renderer *renderer,
		uint32_t fmt, uint32_t stride, uint32_t width, uint32_t height,
		const void *data) {
	assert(width > 0);
	assert(height > 0);
	assert(stride > 0);
	assert(data);

	struct wlr_readonly_data_buffer *buffer =
		readonly_data_buffer_create(fmt, stride, width, height, data);
	if (buffer == NULL) {
		return NULL;
	}

	struct wlr_texture *texture =
		wlr_texture_from_buffer(renderer, &buffer->base);

	// By this point, the renderer should have locked the buffer if it still
	// needs to access it in the future.
	readonly_data_buffer_drop(buffer);

	return texture;
}

struct wlr_texture *wlr_texture_from_dmabuf(struct wlr_renderer *renderer,
		struct wlr_dmabuf_attributes *attribs) {
	struct wlr_dmabuf_buffer *buffer = dmabuf_buffer_create(attribs);
	if (buffer == NULL) {
		return NULL;
	}

	struct wlr_texture *texture =
		wlr_texture_from_buffer(renderer, &buffer->base);

	// By this point, the renderer should have locked the buffer if it still
	// needs to access it in the future.
	dmabuf_buffer_drop(buffer);

	return texture;
}

struct wlr_texture *wlr_texture_from_buffer(struct wlr_renderer *renderer,
		struct wlr_buffer *buffer) {
	assert(!renderer->rendering);
	if (!renderer->impl->texture_from_buffer) {
		return NULL;
	}
	return renderer->impl->texture_from_buffer(renderer, buffer);
}

bool wlr_texture_is_opaque(struct wlr_texture *texture) {
	if (!texture->impl->is_opaque) {
		return false;
	}
	return texture->impl->is_opaque(texture);
}

bool wlr_texture_write_pixels(struct wlr_texture *texture,
		uint32_t stride, uint32_t width, uint32_t height,
		uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y,
		const void *data) {
	if (!texture->impl->write_pixels) {
		return false;
	}
	return texture->impl->write_pixels(texture, stride, width, height,
		src_x, src_y, dst_x, dst_y, data);
}
