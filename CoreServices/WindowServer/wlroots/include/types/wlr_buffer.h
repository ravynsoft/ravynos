#ifndef TYPES_WLR_BUFFER
#define TYPES_WLR_BUFFER

#include <wlr/types/wlr_buffer.h>

struct wlr_shm_client_buffer {
	struct wlr_buffer base;

	uint32_t format;
	size_t stride;

	// The following fields are NULL if the client has destroyed the wl_buffer
	struct wl_resource *resource;
	struct wl_shm_buffer *shm_buffer;

	// This is used to keep the backing storage alive after the client has
	// destroyed the wl_buffer
	struct wl_shm_pool *saved_shm_pool;
	void *saved_data;

	struct wl_listener resource_destroy;
	struct wl_listener release;
};

/**
 * A read-only buffer that holds a data pointer.
 *
 * This is suitable for passing raw pixel data to a function that accepts a
 * wlr_buffer.
 */
struct wlr_readonly_data_buffer {
	struct wlr_buffer base;

	const void *data;
	uint32_t format;
	size_t stride;

	void *saved_data;
};

/**
 * Wraps a read-only data pointer into a wlr_buffer. The data pointer may be
 * accessed until readonly_data_buffer_drop() is called.
 */
struct wlr_readonly_data_buffer *readonly_data_buffer_create(uint32_t format,
		size_t stride, uint32_t width, uint32_t height, const void *data);
/**
 * Drops ownership of the buffer (see wlr_buffer_drop() for more details) and
 * perform a copy of the data pointer if a consumer still has the buffer locked.
 */
bool readonly_data_buffer_drop(struct wlr_readonly_data_buffer *buffer);

struct wlr_dmabuf_buffer {
	struct wlr_buffer base;
	struct wlr_dmabuf_attributes dmabuf;
	bool saved;
};

/**
 * Wraps a DMA-BUF into a wlr_buffer. The DMA-BUF may be accessed until
 * dmabuf_buffer_drop() is called.
 */
struct wlr_dmabuf_buffer *dmabuf_buffer_create(
	struct wlr_dmabuf_attributes *dmabuf);
/**
 * Drops ownership of the buffer (see wlr_buffer_drop() for more details) and
 * takes a reference to the DMA-BUF (by dup'ing its file descriptors) if a
 * consumer still has the buffer locked.
 */
bool dmabuf_buffer_drop(struct wlr_dmabuf_buffer *buffer);

#endif
